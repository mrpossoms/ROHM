#include "rohm.h"
#include <tiffio.h>
#include <math.h>

struct vec3
{
	double x, y, z;
};

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


bool coord_to_idx(TIFF* tif, rohm::window win, rohm::coord c, size_t& r_out, size_t& c_out)
{
	if (!win.contains(c)) { return false; }

	uint32 w, h;
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);

	auto c1 = win.corner_se - win.corner_nw;
	c -= win.corner_nw;
	c /= c1;

	r_out = h * c.lat();
	c_out = w * c.lng();

	return true;
}


void get_window_res(TIFF* tif, rohm::window win, size_t& w_out, size_t& h_out)
{
	size_t nw_r, nw_c, se_r, se_c;

	coord_to_idx(tif, win, win.corner_nw, nw_r, nw_c);
	coord_to_idx(tif, win, win.corner_se, se_r, se_c);

	w_out = se_c - nw_c;
	h_out = se_r - nw_r;
}


void rohm::estimate(
	size_t map_r, size_t map_c,
	float** energy_map_out,
	estimate_params params)
{
	static const char* TILE_NAMES[2][4] = {
		{ "A1", "B1", "C1", "D1" },
		{ "A2", "B2", "C2", "D2" },
	};

	TIFF* tiles[2][4] = {};	

	for (size_t i = 0; i < 4; i++)
	{ // load tiles
		char path[128];
		size_t r = 0, c = 0;

		get_tile_idx(params.win.corner(i), r, c);
		snprintf(path, sizeof(path), "data/%s.tif", TILE_NAMES[r][c]);

		tiles[r][c] = TIFFOpen(path, "r");
	}

finish:
	for (size_t r = 2; r--;)
	for (size_t c = 4; c--;)
	{
		TIFFClose(tiles[r][c]);
	}
}
