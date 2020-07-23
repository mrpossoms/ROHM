#pragma once

namespace rhom
{

struct coord
{
	double lat, lng;
};

struct window
{
	struct { coord nw, se; } corners;

	coord corner(size_t ci)
	{
		switch(ci)
		{
			case 0:
				return corners.nw;
			case 1:
				return { corners.nw.lat, corners.se.lng; };
			case 2:
				return corners.se;
			case 3:
				return { corners.se.lat, corners.nw.lng; };
			default:
				return {};
		}
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

void estimate(size_t r, size_t c, float energy_map_out[r][c], estimate_params params);

}
