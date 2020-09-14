#include ".test.h"
#include "rohm.h"
#include "rohm.cpp"

TEST
{
	const size_t w = 401, h = 401;
	// const size_t w = 9, h = 9;
	rohm::estimate_cell** map = new rohm::estimate_cell*[h];
	for (auto r = h; r--;)
	map[r] = new rohm::estimate_cell[w];

	//rohm::coord start = { 40.142727, -105.101341 }; // longmong CO
	rohm::coord start = { 40.358939, -105.530703 }; // estes park CO
	// rohm::coord start = { 27.093487, -82.429928 }; // venice FL
	auto nw = start + rohm::coord{ 1.6, -1.6 };
	auto se = start + rohm::coord{ -1.6, 1.6 };
	rohm::window est_win;//(corner_nw, corner_se);
	est_win.nw = nw;
	est_win.se = se;

	rohm::vehicle_params e_golf_2016 = {
		5.51, 0.3, 24, 1536.36
	};

	rohm::vehicle_params tesla_model3_lr = {
		6.25, 0.3, 75, 1730
	};

	auto car = e_golf_2016;

	rohm::estimate(h, w, map, {
		est_win, start, car
	});

	rohm::write_tiff("out.tif", h, w, map, car);

	// for (int i = 0; i < h; i++)
	// {
	// 	for (int j = 0; j < w; j++)
	// 	{
	// 		printf("%f   ", map[i][j].energy_kwh);
	// 	}
	// 	printf("\n");
	// }
	// printf("\n");

	// for (int i = 0; i < h; i++)
	// {
	// 	for (int j = 0; j < w; j++)
	// 	{
	// 		printf("%f   ", map[i][j].d_elevation_m);
	// 	}
	// 	printf("\n");
	// }
	// printf("\n");

	// for (int i = 0; i < h; i++)
	// {
	// 	for (int j = 0; j < w; j++)
	// 	{
	// 		auto d_coord = map[i][j].ecef_location - map[h/2][w/2].ecef_location;
	// 		auto mag = d_coord.mag();
	// 		printf("%f   ", mag);
	// 	}
	// 	printf("\n");
	// }
	// printf("\n");



	// for (int i = 0; i < h; i++)
	// {
	// 	for (int j = 0; j < w; j++)
	// 	{
	// 		printf("%f,%f   ", map[i][j].gcs_location.lat(), map[i][j].gcs_location.lng());
	// 	}
	// 	printf("\n");
	// }



	return 0;
}
