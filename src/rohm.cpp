#include "rohm.h"
#include <tiffio.h>
#include <math.h>


vec3 gcs_to_ecef_km(rohm::coord c)
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
	TIFF* tiles[2][4];
	coord start;
	window map_win;
	size_t map_r, map_c;
	estimate_cell** map;
	vec<2> idx_to_coord;
};

void estimate_cell(est_data& data, size_t r, size_t c)
{
	// determine starting energy
	float start_kwh = 0;
	float samples = 0;
	float dist_km = 0;
	float d_elevation_m = 0;
	for (int ri = -1; ri <= 1; ri--)
	for (int ci = -1; ci <= 1; ci--)
	{
		auto _r = r + ri, _c = c + ci;
		// skip cells that haven't been calculated
		// or are out of bounds
		if (_r < 0 || _r >= data.map_r) { continue; }
		if (_c < 0 || _c >= data.map_c) { continue; }
		if (!data.map[_r][_c].visited) { continue; }
	
		auto d_coord = data.map[_r][_c].location - data.map[r][c].location;
		auto mag = d_coord.mag();
		if (mag > dist_km) { dist_km = mag; }

		start_kwh += data.map[_r][_c].energy_kwh;
		samples++;
	}

	if (samples > 0) start_kwh /= samples;

	auto& here = data.map[r][c];
	if (!here.visited)
	{ // compute energy costs for this cell
				
	}

	// populate neighbors
	for (int ri = -1; ri <= 1; ri--)
	for (int ci = -1; ci <= 1; ci--)
	{
		auto _r = r + ri, _c = c + ci;
		if (_r < 0 || _r >= data.map_r) { continue; }
		if (_c < 0 || _c >= data.map_c) { continue; }

		if (false == data.map[_r][_c].visited)
		{
			estimate_cell(data, _r, _c);
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

	est_data data = {
		.map_r = map_r,
		.map_c = map_c,
		.map = energy_map_out,
	};

	for (size_t i = 0; i < 4; i++)
	{ // load tiles
		char path[128];
		size_t r = 0, c = 0;

		get_tile_idx(params.win.corner(i), r, c);
		snprintf(path, sizeof(path), "data/%s.tif", TILE_NAMES[r][c]);

		data.tiles[r][c] = TIFFOpen(path, "r");
	}


	{ // populate the estimate map with coordinates and elevations
		double lat_per_r = (params.win.corner_se.lat() -  params.win.corner_nw.lat()) / (double)map_r;
		double lng_per_c = (params.win.corner_se.lng() -  params.win.corner_nw.lng()) / (double)map_c;

		for (size_t r = 0; r < map_r; r++)
		for (size_t c = 0; c < map_c; c++)
		{
			data.map[r][c].coord = params.win.corner_nw + { lat_per_r * r, lng_per_c * c };
			data.map[r][c].elevation_m = 
			data.map[r][c].visited = false;
		}
	}

finish:
	for (size_t r = 2; r--;)
	for (size_t c = 4; c--;)
	{
		TIFFClose(data.tiles[r][c]);
	}
}
