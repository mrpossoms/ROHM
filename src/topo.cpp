#include "topo.h"

using namespace rohm;

topo::topo(const std::string& base_path, rohm::window win)
{
    static const char* TILE_NAMES[2][4] = {
        { "A1", "B1", "C1", "D1" },
        { "A2", "B2", "C2", "D2" },
    };

    int loaded_r = -1, loaded_c = -1;

    for (size_t i = 0; i < 4; i++)
    { // load tiles
        char path[128];
        size_t r = 0, c = 0;

        get_tile_idx(win.corner(i), r, c);
        snprintf(path, sizeof(path), "%s/%s.tif", base_path.c_str(), TILE_NAMES[r][c]);

        if (tiles[r][c] != nullptr) { continue; }

        tiles[r][c] = TIFFOpen(path, "r");
        uint32 config;
        TIFFGetField(tiles[r][c], TIFFTAG_PLANARCONFIG, &config);
        TIFFGetField(tiles[r][c], TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
        TIFFGetField(tiles[r][c], TIFFTAG_SAMPLESPERPIXEL, &samp_per_pixel);

        loaded_r = r; loaded_c = c;

        printf("loaded tile: '%s'\n", path);
        printf("bits_per_sample: %d samp_per_pixel: %d\n", bits_per_sample, samp_per_pixel);
    }

    line_buf = _TIFFmalloc(TIFFScanlineSize(tiles[loaded_r][loaded_c]));
}


topo::~topo()
{
    for (size_t r = 2; r--;)
    for (size_t c = 4; c--;)
    {
        if (nullptr == tiles[r][c]) { continue; }
        TIFFClose(tiles[r][c]);
    }

    _TIFFfree(line_buf);
}


float topo::elevation_m(const rohm::coord& gcs_coord)
{
    size_t t_r, t_c;
    uint32 t_w, t_h;
    get_tile_idx(gcs_coord, t_r, t_c);
    TIFFGetField(tiles[t_r][t_c], TIFFTAG_IMAGEWIDTH, &t_w);
    TIFFGetField(tiles[t_r][t_c], TIFFTAG_IMAGELENGTH, &t_h);

    auto win = get_tile_window(t_r, t_c);
    TIFF* tif = tiles[t_r][t_c];
    coord_to_idx(t_w, t_h, win, gcs_coord, t_r, t_c);

    TIFFReadScanline(tif, line_buf, t_r, samp_per_pixel);

    return ((uint8_t*)line_buf)[t_c] * (6400.0 / 255.0);
}


const TIFF* topo::tif_at_coord(const rohm::coord& gcs_coord)
{
    size_t t_r, t_c;
    uint32 t_w, t_h;
    get_tile_idx(gcs_coord, t_r, t_c);
    TIFFGetField(tiles[t_r][t_c], TIFFTAG_IMAGEWIDTH, &t_w);
    TIFFGetField(tiles[t_r][t_c], TIFFTAG_IMAGELENGTH, &t_h);

    auto win = get_tile_window(t_r, t_c);
    return tiles[t_r][t_c];
}
