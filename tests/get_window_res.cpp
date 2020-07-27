#include ".test.h"
#include "rohm.h"
#include "rohm.cpp"

TEST
{
	size_t w, h;
	TIFF* tif = TIFFOpen("../data/A1.tif", "r");
	
	uint32 config;
	uint16 samp_per_pixel, bits_per_sample;
	TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samp_per_pixel);

	printf("bits_per_sample: %d samp_per_pixel: %d\n", bits_per_sample, samp_per_pixel);

	get_window_res(tif, {{90, -180}, {0, -90}}, w, h);

	assert(10800 == w);
	assert(10800 == h);

	get_window_res(tif, {{90, 0}, {0, 90}}, w, h);

	assert(10800 == w);
	assert(10800 == h);

	return 0;
}
