#include "rohm.h"
#include <tiff.h>

bool get_tile_idx(rohm::coord c, size_t& r_out, size_t& c_out)
{
	if (c.lat < -90 || c.lat > 90) { return false; }
	if (c.lng < -180 || c.lng > 180) { return false; }

	r_out = c.lat > 0 ? 0 : 1;
	c_out = (c.lng + 180) / 90;

	return true;	
}


void rohm::estimate(
	size_t r, size_t c,
	float energy_map_out[r][c],
	estimate_params params)
{
	static char* TILE_NAMES[2][4] = {
		{ "A1", "B1", "C1", "D1" },
		{ "A2", "B2", "C2", "D2" },
	};

	TIFF* tiles[2][4] = {};	

	for (size_t i = 0; i < 4; i++)
	{ // load tiles
		char path[128];
		size_t r = 0; c = 0;

		get_tile_idx(params.win.corner(i), r, c);
		snprintf(path, sizeof(path), "data/%s.tif", TILE_NAMES[r][c]);

		tiles[r][c] = TIFFOpen(path, "r");
	}
}
