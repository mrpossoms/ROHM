#pragma once
#include "rohm.h"
#include <tiffio.h>

rohm::vec<3> gcs_to_ecef_km(rohm::coord c);

bool get_tile_idx(rohm::coord c, size_t& r_out, size_t& c_out);

rohm::window get_tile_window(size_t r, size_t c);

bool coord_to_idx(size_t width, size_t height, rohm::window win, rohm::coord c, size_t& r_out, size_t& c_out);

bool coord_to_idx(TIFF* tif, rohm::window win, rohm::coord c, size_t& r_out, size_t& c_out);

void get_window_res(TIFF* tif, rohm::window win, size_t& w_out, size_t& h_out);