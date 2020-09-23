#include "coord_utils.h"


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

	auto c1 = win.se - win.nw;
	c -= win.nw;
	c /= c1;

	r_out = (height-1) * c.lat();
	c_out = (width-1) * c.lng();

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

	coord_to_idx(tif, win, win.nw, nw_r, nw_c);
	coord_to_idx(tif, win, win.se, se_r, se_c);

	w_out = se_c - nw_c;
	h_out = se_r - nw_r;
}
