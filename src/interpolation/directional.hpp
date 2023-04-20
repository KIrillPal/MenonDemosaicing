#pragma once
#include "../support/bitmap.hpp"
#include "filter.hpp"

//#define PARALLEL
//#define SIMD
#define RGGB

// define 'size_t pf' - variable shows the first red or blue pixel position in x-th row
#ifdef RGGB
#define SIZE_T_PF(x) size_t pf = x & 1;
#else
#define SIZE_T_PF(x) size_t pf = (~x) & 1;
#endif

namespace menon {
    // Two variants of interpolation (vertical and horizontal)
    Bitmap InterpolateVertical  (const Bitmap& mosaic);
    Bitmap InterpolateHorizontal(const Bitmap& mosaic);
    Bitmap InterpolateDirectional(const Bitmap& mosaic, Direction d);
    BitmapVH InterpolateVHInParallel(const Bitmap& mosaic);
} // namespace menon