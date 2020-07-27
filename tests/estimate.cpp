#include ".test.h"
#include "rohm.h"
#include "rohm.cpp"

TEST
{
	const size_t w = 3, h = 3;
	rohm::estimate_cell** map = new rohm::estimate_cell*[h];
	for (auto r = h; r--;)
	map[r] = new rohm::estimate_cell[w];

	rohm::coord start = { 40.142727, -105.101341 };

	auto corner_nw = start + rohm::coord{ 0.0001, -0.0001 };
	auto corner_se = start + rohm::coord{ -0.0001, 0.0001 };
	rohm::window est_win;//(corner_nw, corner_se);
	est_win.corner_nw = corner_nw;
	est_win.corner_se = corner_se;

	rohm::vehicle_params params = {
		4.1, 0, 24
	};

	rohm::estimate(1, 1, map, {
		est_win, start, params
	});

	return 0;
}
