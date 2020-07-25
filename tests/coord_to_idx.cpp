#include ".test.h"
#include "rohm.h"
#include "rohm.cpp"

TEST
{
	size_t r, c;
	rohm::coord mid = { 45, -135 };

	get_tile_idx(mid, r, c);
	assert(0 == r);
	assert(0 == c);

	auto win = get_tile_window(r, c);	

	TIFF* tif = TIFFOpen("../data/A1.tif", "r");

	assert(coord_to_idx(tif, win, mid, r, c));

	assert(5400 == r);
	assert(5400 == c);

	return 0;
}
