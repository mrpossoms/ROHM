#pragma once

#include "coord_utils.h"

namespace rohm
{

struct topo
{
    topo(const std::string& base_path, rohm::window win);
    ~topo();

    float elevation_m(const rohm::coord& gcs_coord);

    const TIFF* tif_at_coord(const rohm::coord& gcs_coord);

private:
    uint16 samp_per_pixel;
    uint16 bits_per_sample;
    TIFF* tiles[2][4] = {};
    tdata_t line_buf;
};

}
