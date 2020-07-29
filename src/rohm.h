#pragma once

#include <stdlib.h>
#include <math.h>
#include <initializer_list>
#include <string.h>

namespace rohm
{

template <size_t N>
struct vec
{
	vec() = default;
	vec(const vec<N>& o)
	{ 
		for (auto i = N; i--;)
		{
			v[i] = o.v[i];
		}
	}

	vec(std::initializer_list<double> init)
	{
		if (init.size() < N)
		{
			for (auto i = N; i--;)
			{
				v[i] = *init.begin();
			}
		}
		else
		{
			auto i = 0;
			for (auto s : init)
			{
				v[i++] = s;
			}
		}
	}

	bool operator==(const vec<N>& o)
	{
		for(int i = 0; i < N; i++)
		{
			if (o.v[i] != v[i]) { return false; } 
		} 
		return true;
	}

	vec<N>& operator=(const vec<N>& o)
	{
		for(int i = 0; i < N; i++)
		{
			v[i] = o.v[i]; 
		} 
		return *this;
	}

	vec<N> operator-(const vec<N>& o)
	{
		vec<N> out;
		for (int i = 0; i < N; i++) { out.v[i] = v[i] - o.v[i]; }
		return out;
	}

	vec<N> operator+(const vec<N>& o)
	{
		vec<N> out;
		for (int i = 0; i < N; i++) { out.v[i] = v[i] + o.v[i]; }
		return out;
	}

	vec<N> operator*(double s)
	{
		vec<N> out;
		for (int i = 0; i < N; i++) { out.v[i] = v[i] * s; }
		return out;
	}

	vec<N>& operator-=(const vec<N>& o)
	{
		for (int i = 0; i < N; i++) { v[i] -= o.v[i]; }
		return *this;
	}

	vec<N> operator/=(const vec<N>& o)
	{
		for (int i = 0; i < N; i++) {  v[i] /= o.v[i]; }
		return *this;
	}

	double mag()
	{
		double m = 0;
		for (int i = N; i--;) { m += v[i] * v[i]; }
		return sqrt(m);
	}

	double v[N] = {0};
};

struct coord : public vec<2>
{
	coord () : vec<2>() {}
	coord (vec<2> v) : vec<2>(v) {}
	coord (double lat, double lng) : vec<2>({ lat, lng }) { }

	inline double lat() const { return v[0]; }
	inline double lng() const { return v[1]; }

	inline double lat(double l) { return v[0] = l; }
	inline double lng(double l) { return v[1] = l; }
};

struct window
{
	window() = default;
	window(const coord& nw, const coord& se)
	{
		corner_nw = nw;
		corner_se = se;
	}

	coord corner_nw, corner_se;

	coord corner(size_t ci)
	{
		switch(ci)
		{
			case 0:
				return corner_nw;
			case 1:
				return { corner_nw.lat(), corner_se.lng() };
			case 2:
				return corner_se;
			case 3:
				return { corner_se.lat(), corner_nw.lng() };
			default:
				return {};
		}
	}

	bool contains(const coord& c)
	{
		return corner_nw.lat() >= c.lat() && c.lat() >= corner_se.lat() &&
		       corner_nw.lng() <= c.lng() && c.lng() <= corner_se.lng();
	}

	bool operator==(const window& o)
	{
		return corner_nw == o.corner_nw && corner_se == o.corner_se;
	}
};

struct vehicle_params
{
	float avg_kwh_km;
	float regen_efficiency;
	float energy_kwh;
	float mass_kg;
};

struct estimate_params
{
	window win;
	coord origin;
	vehicle_params car;
};

struct estimate_cell
{
	bool visited;
	coord gcs_location;
	vec<3> ecef_location;
	float elevation_m;
	float energy_kwh;
};

void estimate(size_t r, size_t c, estimate_cell** energy_map_out, estimate_params params);

}
