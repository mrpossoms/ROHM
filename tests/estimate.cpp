#include ".test.h"
#include "rohm.h"
#include "rohm.cpp"

TEST
{
	const size_t w = 9, h = 9;
	rohm::estimate_cell** map = new rohm::estimate_cell*[h];
	for (auto r = h; r--;)
	map[r] = new rohm::estimate_cell[w];

	rohm::coord start = { 40.142727, -105.101341 };

	auto corner_nw = start + rohm::coord{ 0.2, -0.2 };
	auto corner_se = start + rohm::coord{ -0.2, 0.2 };
	rohm::window est_win;//(corner_nw, corner_se);
	est_win.corner_nw = corner_nw;
	est_win.corner_se = corner_se;

	rohm::vehicle_params params = {
		4.1, 0, 24, 1536.36
	};

	rohm::estimate(h, w, map, {
		est_win, start, params
	});

	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			printf("%f   ", map[i][j].energy_kwh);
		}
		printf("\n");		
	}
	printf("\n");

	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			printf("%f   ", map[i][j].d_elevation_m);
		}
		printf("\n");		
	}
	printf("\n");

	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			auto d_coord = map[i][j].ecef_location - map[h/2][w/2].ecef_location;
			auto mag = d_coord.mag();
			printf("%f   ", mag);
		}
		printf("\n");		
	}
	printf("\n");



	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			printf("%f,%f   ", map[i][j].gcs_location.lat(), map[i][j].gcs_location.lng());
		}
		printf("\n");		
	}


	return 0;
}
