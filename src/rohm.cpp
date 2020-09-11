#include "rohm.h"
#include "coord_utils.h"
#include "topo.h"
#include <tiffio.h>
#include <stdint.h>
#include <algorithm>


struct est_data {
	TIFF* tiles[2][4] = {};
	rohm::coord start;
	rohm::window map_win;
	rohm::vehicle_params car;
	float temperature_c;
	float avg_speed_km_h;
	size_t map_r, map_c;
	rohm::estimate_cell** map;
	rohm::vec<2> idx_to_coord;
};


inline rohm::estimate_cell* cell_at_coord(const est_data& data, rohm::coord coord)
{
	size_t r, c;
	coord_to_idx(data.map_c, data.map_r, data.map_win, coord, r, c);
	return &data.map[r][c];
}


void cell_energy_expenditure(rohm::estimate_cell& here,
	const est_data& data,
	float d_elevation_m,
	float dist_km,
	float speed_km_h)
{ // compute energy costs for this cell
	const auto g = 9.80665; // m/s^2
	const auto kmh_to_ms = 1000.0 * 3600.0;
	const auto travel_time_s = (dist_km / speed_km_h) * 3600.0;
	const auto speed_ms = speed_km_h * kmh_to_ms;
	const auto ws_to_kwh = 1 / 3.6e+6;

	here.d_elevation_m = d_elevation_m;

	float P = 101325; // air pressure (Pa) at sea level


	if (speed_km_h)
	{ // more specific calculation using drag, and rolling-resistance
		auto temp_k = here.temperature_c + 273.15; // convert celsius to Kelvin

		{ // compute air pressure
			// https://en.wikipedia.org/wiki/Barometric_formula
			const auto p_0 = 101325; // pressure (Pa) at sea level
			const auto L = 0.00976; // temperature lapse rate (K/m)
			const auto c_p = 1004.68506; // constant-pressure specific heat (J/(kg·K))
			const auto T_0 = 288.16; // sea level standard tempurature (K)
			const auto M = 0.02896968; // Molar mass of dry air (kg/mol)
			const auto R_0 = 8.314462618; // Universal gas constant (J/(mol·K))

			auto exp = (c_p * M) / R_0;
			P = p_0 * pow(1.0 - ((g * here.elevation_m) / (c_p * T_0)), exp);
		}

		// p the mass density of the air can be computed as
		// p = P / R * T
		// where P is the absolute pressure in Pa
		// R is the specific gas constant for dry air
		// T is the absolute tempurature (K)
		auto R_spec = 287.058; // J/(kg·K) for dry air
		auto p = P / R_spec * temp_k;

		// Force from drag
		// F = 0.5 * p * v^2 * c_d * A
		// where p is the mass density of the air (see below)
		// where v is the flow speed of the object relative to the air (m/s)
		// c_d is the coefficent of drag
		// A is the crossectional area of the object (m^2)
		auto F_d = 0.5 * p * -pow(speed_km_h, 2) * data.car.c_drag * data.car.area_m2;
		auto ws_d = F_d * speed_ms;
		auto kwh_d = (ws_d * travel_time_s) * ws_to_kwh;

	}
	else
	{ // simplified using 'average efficiency'
		here.energy_kwh -= dist_km / data.car.avg_kwh_km;
	}

	auto regen_efficiency = d_elevation_m > 0 ? 1 : data.car.regen_efficiency;

	auto spent_energy_ws = g * regen_efficiency * data.car.mass_kg * d_elevation_m;
	here.energy_kwh -= spent_energy_ws * ws_to_kwh; // convert to kwh
}


void estimate_cell_eval(est_data& data, size_t r, size_t c, int iteration)
{
	auto& here = data.map[r][c];

	if (here.visited) { return; }

	// determine starting energy
	float start_kwh = 0;
	float samples = 0;
	float dist_km = 0;
	float d_elevation_m = 0;

	for (int ri = -1; ri <= 1; ri++)
	for (int ci = -1; ci <= 1; ci++)
	{
		int _r = r + ri, _c = c + ci;
		// skip cells that haven't been calculated
		// or are out of bounds
		if (_r < 0 || _r >= data.map_r) { continue; }
		if (_c < 0 || _c >= data.map_c) { continue; }

		// skip unvisited, and visited in this iteration or future
		if (!data.map[_r][_c].visited) { continue; }
		if (data.map[_r][_c].visited >= iteration) { continue; }

		// compute distance as the difference of the two cell's 3d ecef
		// coordinates
		auto d_coord = data.map[_r][_c].ecef_location - here.ecef_location;
		auto mag = d_coord.mag();

		start_kwh += data.map[_r][_c].energy_kwh;
		dist_km += mag;
		d_elevation_m += here.elevation_m - data.map[_r][_c].elevation_m;

		samples++;
	}

	if (samples == 0) { return; }

	here.energy_kwh = start_kwh / samples;
	dist_km /= samples;
	d_elevation_m /= samples;

	cell_energy_expenditure(here, data, {}, d_elevation_m, dist_km);

	here.visited = iteration;
}


void estimate_cell_path(est_data& data, const rohm::trip trip)
{
	// rohm::topo topo("data", data.map_win);
	auto has_speeds = trip.waypoints.size() == trip.avg_speed_km_h.size();

	for (auto i = 1; i < trip.waypoints.size(); i++)
	{
		auto cur_waypoint_cell = cell_at_coord(data, trip.waypoints[i - 1]);
		auto next_waypoint_cell = cell_at_coord(data, trip.waypoints[i]);
		auto speed_km_h = trip.avg_speed_km_h[i];

		// TODO
		auto d_elevation_m = next_waypoint_cell->elevation_m - cur_waypoint_cell->elevation_m;


		size_t wp_r, wp_c;
		coord_to_idx(data.map_c, data.map_r, data.map_win, cur_waypoint_cell->gcs_location, wp_r, wp_c);
		estimate_cell_eval(data, wp_r, wp_c, i + 1);
	}
}


rohm::window window_from_path(const std::vector<rohm::coord> path)
{
	if (path.size() == 0) return {};

	rohm::window win = {
		path[0], path[0]
	};

	// find the bounding box for the path given
	for (const auto& coord : path)
	{
		if (coord.lat() > win.nw.lat()) { win.nw.lat(coord.lat()); }
		if (coord.lng() < win.nw.lng()) { win.nw.lng(coord.lng()); }
		if (coord.lat() < win.se.lat()) { win.se.lat(coord.lat()); }
		if (coord.lng() > win.se.lng()) { win.se.lng(coord.lng()); }
	}

	return win;
}


void rohm::estimate(
	size_t map_r, size_t map_c,
	estimate_cell** map,
	estimate_params params)
{
	est_data data;
	data.map_r = map_r;
	data.map_c = map_c;
	data.map = map;
	data.car = params.car;

	// if a trip has been provided recalculate the window from waypoints
	if (!params.trip.is_empty())
	{
		params.win = window_from_path(params.trip.waypoints);
	}

	rohm::topo topo("data", params.win);

	{ // populate the estimate map with coordinates and elevations
		for (size_t r = 0; r < map_r; r++)
		for (size_t c = 0; c < map_c; c++)
		{
			double r_w = r / (double)map_r, c_w = c / (double)map_c;
			data.map[r][c].gcs_location = {
				params.win.nw.lat() * (1.0 - r_w) + params.win.se.lat() * r_w,
				params.win.nw.lng() * (1.0 - c_w) + params.win.se.lng() * c_w,
			};

			// convert GCS to ECEF 3D vector
			data.map[r][c].ecef_location = gcs_to_ecef_km(data.map[r][c].gcs_location);

			data.map[r][c].elevation_m = topo.elevation_m(data.map[r][c].gcs_location);
			data.map[r][c].visited = 0;
			data.map[r][c].energy_kwh = 0;
		}
	}

	// setup start cell, begin estimation
	size_t r, c;
	coord_to_idx(map_c, map_r, params.win, params.origin, r, c);
	data.map[r][c].energy_kwh = params.car.energy_kwh;
	data.map[r][c].visited = 1;

	if (!params.trip.is_empty())
	{
		estimate_cell_path(data, params.trip);
	}
	else
	{
		for (int itr = 1; !data.map[0][0].visited; itr++)
		for (size_t r = 0; r < map_r; r++)
		for (size_t c = 0; c < map_c; c++)
		{
			estimate_cell_eval(data, r, c, itr);
		}
	}
}

struct rgb_t
{
	uint8_t r, g, b;
};


void rohm::write_tiff(
	const std::string& path,
	const size_t r, const size_t c,
	rohm::estimate_cell** map,
	rohm::vehicle_params car)
{


	TIFF *tif= TIFFOpen(path.c_str(), "w");

	int samp_per_pixel = 3;

	char *pixel_buf = new char[c * r * samp_per_pixel];

	int w = c, h = r;
	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, w);  // set the width of the image
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, h);    // set the height of the image
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samp_per_pixel);   // set number of channels per pixel
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);    // set the size of the channels
	TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, w * samp_per_pixel));

	tsize_t line_bytes = samp_per_pixel * w;
	rgb_t* row_buf = (rgb_t*)_TIFFmalloc(line_bytes);

	for(size_t ri = 0; ri < r; ri++)
	{
		for (size_t ci = 0; ci < c; ci++)
		{
			auto charge_percentage = std::min(1.0f, std::max(map[ri][ci].energy_kwh / car.energy_kwh, 0.0f));
			// auto charge_percentage = map[ri][ci].energy_kwh / car.energy_kwh;

			auto elevation = map[ri][ci].elevation_m / 6400.0;

			row_buf[ci].r = 255 * (1.0 - charge_percentage) * elevation;
			row_buf[ci].g = 255 * (charge_percentage) * elevation;
			row_buf[ci].b = 255.0 * elevation;
		}

    	if (TIFFWriteScanline(tif, (unsigned char*)row_buf, ri, 0) < 0) { break; }
	}
	_TIFFfree((unsigned char*)row_buf);

	TIFFClose(tif);
}
