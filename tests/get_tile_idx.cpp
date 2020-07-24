#include ".test.h"
#include "rohm.h"
#include "rohm.cpp"

TEST
{
	size_t r, c;

	get_tile_idx({ 40.142786, -105.101287 }, r, c);
	assert(0 == r);
	assert(0 == c);	

	get_tile_idx({ -26.105437, 138.251740 }, r, c);
	assert(1 == r);
	assert(3 == c);

	return 0;
}
