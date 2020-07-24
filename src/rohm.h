#pragma once

#include <stdlib.h>

namespace rohm
{

struct coord
{
	double lat, lng;

	bool operator==(const coord& other) { return other.lat == lat && other.lng == lng; }
};

struct window
{
	coord corner_nw, corner_se;

	coord corner(size_t ci)
	{
		switch(ci)
		{
			case 0:
				return corner_nw;
			case 1:
				return { corner_nw.lat, corner_se.lng };
			case 2:
				return corner_se;
			case 3:
				return { corner_se.lat, corner_nw.lng };
			default:
				return {};
		}
	}

	bool operator==(const window& other)
	{
		return corner_nw == other.corner_nw && corner_se == other.corner_se;
	}
};

struct vehicle_params
{
	float avg_kwh_km;
	float regen_efficiency;
	float energy_kwh;
};

struct estimate_params
{
	window win;
	vehicle_params car;
};

void estimate(size_t r, size_t c, float** energy_map_out, estimate_params params);

}
