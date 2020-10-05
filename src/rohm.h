#pragma once

#include <stdlib.h>
#include <math.h>
#include <vector>
#include <initializer_list>
#include <string.h>
#include <string>

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
		this->nw = nw;
		this->se = se;
	}

	coord nw, se;

	coord corner(size_t ci)
	{
		switch(ci)
		{
			case 0:
				return nw;
			case 1:
				return { nw.lat(), se.lng() };
			case 2:
				return se;
			case 3:
				return { se.lat(), nw.lng() };
			default:
				return {};
		}
	}

	bool contains(const coord& c)
	{
		return nw.lat() >= c.lat() && c.lat() >= se.lat() &&
		       nw.lng() <= c.lng() && c.lng() <= se.lng();
	}

	bool operator==(const window& o)
	{
		return nw == o.nw && se == o.se;
	}

	bool is_unset()
	{
		return nw == coord{ 0, 0 } && se == coord{ 0, 0 };
	}
};

struct waypoint_ctx
{
	rohm::vec<2> wind_ms;
	float temperature_c;
};

struct vehicle_params
{
	float avg_kwh_km;
	float regen_efficiency;
	float energy_kwh;
	float mass_kg;
	float c_drag;
	float c_rolling_resistance;
	float area_m2;
};


struct trip
{
	std::vector<coord> waypoints;
	std::vector<float> avg_speed_km_h;
	std::vector<waypoint_ctx> ctx;

	bool is_empty() const { return 0 == waypoints.size(); }
};

struct estimate_params
{
	window win;
	coord origin;
	vehicle_params car;
	rohm::trip trip;
};

struct estimate_cell
{
	size_t r, c;
	int visited;
	coord gcs_location;
	vec<3> ecef_location;
	float elevation_m;
	float d_elevation_m;
	float temperature_c;
	vec<2> wind_ms;
	float energy_kwh;
};

void estimate(size_t r, size_t c, estimate_cell** energy_map_out, estimate_params params);

window window_from_trip(const trip& trip);

void write_tiff(const std::string& path, const size_t r, const size_t c, estimate_cell** map, vehicle_params car);

}

/**
 * @brief      Returns the pixel index corresponding to a given coordinate in a
 *             provided coordinate window.
 *
 * @param[in]  width   The width of the coordinate window in pixels.
 * @param[in]  height  The height of the coordinate window in pixels
 * @param[in]  win     The coordinate window in GCS coordinates
 * @param[in]  c       The coordinate to map to an index
 * @param      r_out   The row index out
 * @param      c_out   The column index out
 *
 * @return     True if the coordinate is valid and contained in the window.
 */
bool coord_to_idx(size_t width, size_t height, rohm::window win, rohm::coord c, size_t& r_out, size_t& c_out);

