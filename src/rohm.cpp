#include "rohm.h"
#include <tiffio.h>
#include <stdint.h>

rohm::vec<3> gcs_to_ecef_km(rohm::coord c)
{
	const auto EARTH_EQUATORIAL_RAD_KM = 6378.1370;
	const auto EARTH_POLAR_RAD_KM = 6356.7520;

	auto dtr = M_PI / 180.0;

	const auto lat = dtr * c.lat();
	const auto lng = dtr * c.lng();

	const auto lat_cos = cos(lat);
	const auto lat_sin = sin(lat);
	const auto lng_cos = cos(lng);
	const auto lng_sin = sin(lng);

	return { 
		EARTH_EQUATORIAL_RAD_KM * lng_cos * lat_cos,
		EARTH_EQUATORIAL_RAD_KM * lng_sin * lat_cos,
		EARTH_POLAR_RAD_KM * lat_sin
	};
}

bool get_tile_idx(rohm::coord c, size_t& r_out, size_t& c_out)
{
	if (c.lat() < -90 || c.lat() > 90) { return false; }
	if (c.lng() < -180 || c.lng() > 180) { return false; }

	r_out = c.lat() > 0 ? 0 : 1;
	c_out = (c.lng() + 180) / 90;

	return true;	
}


rohm::window get_tile_window(size_t r, size_t c)
{
	static rohm::window tiles[2][4] = {
		{ {{90, -180}, {0, -90}}, {{90, -90}, {0, 0}}, {{90, 0}, {0, -90}}, {{90, 90}, {0, 180}} },
		{ {{0, -180}, {-90, -90}}, {{0, -90}, {-90, 0}}, {{0, 0}, {-90, -90}}, {{0, 90}, {-90, 180}}},
	};

	return tiles[r][c];
}


bool coord_to_idx(size_t width, size_t height, rohm::window win, rohm::coord c, size_t& r_out, size_t& c_out)
{
	if (!win.contains(c)) { return false; }

	auto c1 = win.corner_se - win.corner_nw;
	c -= win.corner_nw;
	c /= c1;

	r_out = height * c.lat();
	c_out = width * c.lng();

	return true;	
}

bool coord_to_idx(TIFF* tif, rohm::window win, rohm::coord c, size_t& r_out, size_t& c_out)
{

	uint32 w, h;
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);

	return coord_to_idx(w, h, win, c, r_out, c_out);
}


void get_window_res(TIFF* tif, rohm::window win, size_t& w_out, size_t& h_out)
{
	size_t nw_r, nw_c, se_r, se_c;

	coord_to_idx(tif, win, win.corner_nw, nw_r, nw_c);
	coord_to_idx(tif, win, win.corner_se, se_r, se_c);

	w_out = se_c - nw_c;
	h_out = se_r - nw_r;
}

struct est_data {
	TIFF* tiles[2][4] = {};
	rohm::coord start;
	rohm::window map_win;
	rohm::vehicle_params car;
	size_t map_r, map_c;
	rohm::estimate_cell** map;
	rohm::vec<2> idx_to_coord;
};

void estimate_cell_r(est_data& data, size_t r, size_t c)
{
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
		if (!data.map[_r][_c].visited) { continue; }
	
		auto d_coord = data.map[_r][_c].ecef_location - data.map[r][c].ecef_location;
		auto mag = d_coord.mag();
		// if (mag > dist_km) { dist_km = mag; }

		start_kwh += data.map[_r][_c].energy_kwh;
		dist_km += mag;
		samples++;
	}

	if (samples > 0)
	{
		start_kwh /= samples;
		dist_km /= samples;
	}

	auto& here = data.map[r][c];
	if (!here.visited)
	{ // compute energy costs for this cell
		here.energy_kwh = start_kwh - (dist_km / data.car.avg_kwh_km);
		here.visited = true;
	}

	// populate neighbors
	for (int ri = -1; ri <= 1; ri++)
	for (int ci = -1; ci <= 1; ci++)
	{
		int _r = r + ri, _c = c + ci;
		if (_r < 0 || _r >= data.map_r) { continue; }
		if (_c < 0 || _c >= data.map_c) { continue; }
		if (0 == ri && 0 == ci) { continue; }

		if (!data.map[_r][_c].visited)
		{
			estimate_cell_r(data, _r, _c);
		}
	}
}


void rohm::estimate(
	size_t map_r, size_t map_c,
	estimate_cell** map,
	estimate_params params)
{
	static const char* TILE_NAMES[2][4] = {
		{ "A1", "B1", "C1", "D1" },
		{ "A2", "B2", "C2", "D2" },
	};

	est_data data;
	data.map_r = map_r;
	data.map_c = map_c;
	data.map = map;
	data.car = params.car;

	uint16 samp_per_pixel, bits_per_sample;
	for (size_t i = 0; i < 4; i++)
	{ // load tiles
		char path[128];
		size_t r = 0, c = 0;

		get_tile_idx(params.win.corner(i), r, c);
		snprintf(path, sizeof(path), "data/%s.tif", TILE_NAMES[r][c]);

		if (data.tiles[r][c] != nullptr) { continue; }

		data.tiles[r][c] = TIFFOpen(path, "r");
		uint32 config;
		TIFFGetField(data.tiles[r][c], TIFFTAG_PLANARCONFIG, &config);
		TIFFGetField(data.tiles[r][c], TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
		TIFFGetField(data.tiles[r][c], TIFFTAG_SAMPLESPERPIXEL, &samp_per_pixel);

		printf("bits_per_sample: %d samp_per_pixel: %d\n", bits_per_sample, samp_per_pixel);
	}


	{ // populate the estimate map with coordinates and elevations
		for (size_t r = 0; r < map_r; r++)
		for (size_t c = 0; c < map_c; c++)
		{
			double r_w = r / (double)map_r, c_w = c / (double)map_c;
			data.map[r][c].gcs_location = {
				params.win.corner_nw.lat() * (1.0 - r_w) + params.win.corner_se.lat() * r_w,
				params.win.corner_nw.lng() * (1.0 - c_w) + params.win.corner_se.lng() * c_w,
			};

			// convert GCS to ECEF 3D vector
			data.map[r][c].ecef_location = gcs_to_ecef_km(data.map[r][c].gcs_location);

			// query for correct tiff given the coordinate
			size_t t_r, t_c;
			uint32 t_w, t_h;
			get_tile_idx(data.map[r][c].gcs_location, t_r, t_c);
			TIFFGetField(data.tiles[t_r][t_c], TIFFTAG_IMAGEWIDTH, &t_w);
			TIFFGetField(data.tiles[t_r][t_c], TIFFTAG_IMAGELENGTH, &t_h);

			auto win = get_tile_window(t_r, t_c);
			TIFF* tif = data.tiles[t_r][t_c];
			coord_to_idx(t_w, t_h, win, data.map[r][c].gcs_location, t_r, t_c);

			tdata_t buf = _TIFFmalloc(TIFFScanlineSize(tif));

			TIFFReadScanline(tif, buf, t_r + r, samp_per_pixel);

			data.map[r][c].elevation_m = ((uint8_t*)buf)[t_c] * (6400.0 / 255.0);
			data.map[r][c].visited = false;

			_TIFFfree(buf);
		}

		size_t r, c;
		coord_to_idx(map_c, map_r, params.win, params.origin, r, c);
		data.map[r][c].energy_kwh = params.car.energy_kwh;
		data.map[r][c].visited = true;

		estimate_cell_r(data, r, c);
	}

finish:
	for (size_t r = 2; r--;)
	for (size_t c = 4; c--;)
	{
		if (nullptr == data.tiles[r][c]) { continue; }
		TIFFClose(data.tiles[r][c]);
	}
}
