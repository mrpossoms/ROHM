#include "rohm.h"
#include "coord_utils.h"
#include <tiffio.h>
#include <stdint.h>
#include <algorithm>

struct est_data {
	TIFF* tiles[2][4] = {};
	rohm::coord start;
	rohm::window map_win;
	rohm::vehicle_params car;
	size_t map_r, map_c;
	rohm::estimate_cell** map;
	rohm::vec<2> idx_to_coord;
};

void estimate_cell_eval(est_data& data, size_t r, size_t c, int iteration)
{
	auto& here = data.map[r][c];

	if (here.visited) { return; }

	// determine starting energy
	float start_kwh = 0;
	float samples = 0;
	float dist_km = 0;
	float d_elevation_m = 0;

	for (int ri = -1; ri <= 1; ri++)
	for (int ci = -1; ci <= 1; ci++)
	{
		int _r = r + ri, _c = c + ci;
		// skip cells that haven't been calculated
		// or are out of bounds
		if (_r < 0 || _r >= data.map_r) { continue; }
		if (_c < 0 || _c >= data.map_c) { continue; }
		if (!data.map[_r][_c].visited) { continue; }
		if (data.map[_r][_c].visited >= iteration) { continue; }
	
		auto d_coord = data.map[_r][_c].ecef_location - here.ecef_location;
		auto mag = d_coord.mag();

		start_kwh += data.map[_r][_c].energy_kwh;
		dist_km += mag;
		d_elevation_m += here.elevation_m - data.map[_r][_c].elevation_m;

		samples++;
	}

	if (samples == 0) { return; }

	start_kwh /= samples;
	dist_km /= samples;
	d_elevation_m /= samples;

	{ // compute energy costs for this cell
		const auto g = 9.8; // m/s^2

		here.energy_kwh = start_kwh - (dist_km / data.car.avg_kwh_km);
		here.d_elevation_m = d_elevation_m;

		if (d_elevation_m > 0)
		{
			here.energy_kwh -= g * data.car.mass_kg * d_elevation_m / 3.6e+6;
		}


		here.visited = iteration;
	}
}


void rohm::estimate(
	size_t map_r, size_t map_c,
	estimate_cell** map,
	estimate_params params)
{

	static const char* TILE_NAMES[2][4] = {
		{ "A1", "B1", "C1", "D1" },
		{ "A2", "B2", "C2", "D2" },
	};

	est_data data;
	data.map_r = map_r;
	data.map_c = map_c;
	data.map = map;
	data.car = params.car;

	uint16 samp_per_pixel, bits_per_sample;
	for (size_t i = 0; i < 4; i++)
	{ // load tiles
		char path[128];
		size_t r = 0, c = 0;

		get_tile_idx(params.win.corner(i), r, c);
		snprintf(path, sizeof(path), "data/%s.tif", TILE_NAMES[r][c]);

		if (data.tiles[r][c] != nullptr) { continue; }

		data.tiles[r][c] = TIFFOpen(path, "r");
		uint32 config;
		TIFFGetField(data.tiles[r][c], TIFFTAG_PLANARCONFIG, &config);
		TIFFGetField(data.tiles[r][c], TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
		TIFFGetField(data.tiles[r][c], TIFFTAG_SAMPLESPERPIXEL, &samp_per_pixel);

		printf("bits_per_sample: %d samp_per_pixel: %d\n", bits_per_sample, samp_per_pixel);
	}


	{ // populate the estimate map with coordinates and elevations
		for (size_t r = 0; r < map_r; r++)
		for (size_t c = 0; c < map_c; c++)
		{
			double r_w = r / (double)map_r, c_w = c / (double)map_c;
			data.map[r][c].gcs_location = {
				params.win.corner_nw.lat() * (1.0 - r_w) + params.win.corner_se.lat() * r_w,
				params.win.corner_nw.lng() * (1.0 - c_w) + params.win.corner_se.lng() * c_w,
			};

			// convert GCS to ECEF 3D vector
			data.map[r][c].ecef_location = gcs_to_ecef_km(data.map[r][c].gcs_location);

			// query for correct tiff given the coordinate
			size_t t_r, t_c;
			uint32 t_w, t_h;
			get_tile_idx(data.map[r][c].gcs_location, t_r, t_c);
			TIFFGetField(data.tiles[t_r][t_c], TIFFTAG_IMAGEWIDTH, &t_w);
			TIFFGetField(data.tiles[t_r][t_c], TIFFTAG_IMAGELENGTH, &t_h);

			auto win = get_tile_window(t_r, t_c);
			TIFF* tif = data.tiles[t_r][t_c];
			coord_to_idx(t_w, t_h, win, data.map[r][c].gcs_location, t_r, t_c);

			tdata_t buf = _TIFFmalloc(TIFFScanlineSize(tif));

			TIFFReadScanline(tif, buf, t_r + r, samp_per_pixel);

			data.map[r][c].elevation_m = ((uint8_t*)buf)[t_c] * (6400.0 / 255.0);
			data.map[r][c].visited = 0;
			data.map[r][c].energy_kwh = 0;

			_TIFFfree(buf);
		}


	}

	// setup start cell, begin estimation
	size_t r, c;
	coord_to_idx(map_c, map_r, params.win, params.origin, r, c);
	data.map[r][c].energy_kwh = params.car.energy_kwh;
	data.map[r][c].visited = 1;	

	for (int itr = 1; itr < map_r * 2; itr++)
	for (size_t r = 0; r < map_r; r++)
	for (size_t c = 0; c < map_c; c++)
	{
		estimate_cell_eval(data, r, c, itr);
	}


finish:
	for (size_t r = 2; r--;)
	for (size_t c = 4; c--;)
	{
		if (nullptr == data.tiles[r][c]) { continue; }
		TIFFClose(data.tiles[r][c]);
	}
}

struct rgb_t
{
	uint8_t r, g, b;
};


void rohm::write_tiff(
	const std::string& path,
	const size_t r, const size_t c,
	rohm::estimate_cell** map,
	rohm::vehicle_params car)
{


	TIFF *tif= TIFFOpen(path.c_str(), "w");

	int samp_per_pixel = 3;

	char *pixel_buf = new char[c * r * samp_per_pixel];

	int w = c, h = r;
	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, w);  // set the width of the image
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, h);    // set the height of the image
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samp_per_pixel);   // set number of channels per pixel
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);    // set the size of the channels
	TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
	//   Some other essential fields to set that you do not have to understand for now.
	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, w * samp_per_pixel));

	tsize_t line_bytes = samp_per_pixel * w;
	rgb_t* row_buf = (rgb_t*)_TIFFmalloc(line_bytes);

	for(size_t ri = 0; ri < r; ri++)
	{
		for (size_t ci = 0; ci < c; ci++)
		{
			auto charge_percentage = std::max(map[ri][ci].energy_kwh / car.energy_kwh, 0.0f);
			// auto charge_percentage = map[ri][ci].energy_kwh / car.energy_kwh;

			row_buf[ci].b = 255.0 * (map[ri][ci].elevation_m / 6400.0);
			row_buf[ci].r = row_buf[ci].b;
			row_buf[ci].g = row_buf[ci].b;	

			row_buf[ci].r *= (1.0 - charge_percentage);
			row_buf[ci].g *= (charge_percentage);
		}

    	if (TIFFWriteScanline(tif, (unsigned char*)row_buf, ri, 0) < 0) { break; }
	}
	_TIFFfree((unsigned char*)row_buf);

	TIFFClose(tif);
}