#pragma once
#include "rohm.h"
#include <tiffio.h>

rohm::vec<3> gcs_to_ecef_km(rohm::coord c);

/**
 * @brief      Gets the index of the map tile that contains a given coordinate
 *
 * @param[in]  c      GCS coordinate
 * @param      r_out  The r out
 * @param      c_out  The c out
 *
 * @return     True if the coordinate is valid and contained by the map.
 */
bool get_tile_idx(rohm::coord c, size_t& r_out, size_t& c_out);

/**
 * @brief      Gets the tile window coordinates.
 *
 * @param[in]  r     Tile index row
 * @param[in]  c     Tile index column
 *
 * @return     The tile window.
 */
rohm::window get_tile_window(size_t r, size_t c);


/**
 * @brief      Returns the pixel index corresponding to a given coordinate in a
 *             provided coordinate window.
 *
 * @param      tif    The tif tile
 * @param[in]  win    The coordinate window in GCS coordinates which spans the tif
 * @param[in]  c      The coordinate to map to an index
 * @param      r_out  The row index out
 * @param      c_out  The column index out
 *
 * @return     True if the coordinate is valid and contained in the window.
 */
bool coord_to_idx(TIFF* tif, rohm::window win, rohm::coord c, size_t& r_out, size_t& c_out);

void get_window_res(TIFF* tif, rohm::window win, size_t& w_out, size_t& h_out);
