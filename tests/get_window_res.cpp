#include ".test.h"
#include "rohm.h"
#include "rohm.cpp"

TEST
{
	size_t w, h;
	TIFF* tif = TIFFOpen("../data/A1.tif", "r");
	get_window_res(tif, {{90, -180}, {0, -90}}, w, h);

	assert(10800 == w);
	assert(10800 == h);

	get_window_res(tif, {{90, 0}, {0, 90}}, w, h);

	assert(10800 == w);
	assert(10800 == h);

	return 0;
}
