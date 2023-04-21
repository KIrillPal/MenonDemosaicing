#pragma once
#include "../support/bitmap.hpp"
#include "filter.hpp"

// define 'size_t pf' - variable shows the first red or blue pixel position in x-th row
#ifdef RGGB
#define SIZE_T_PF(x) size_t pf = x & 1;
#else
// For GRBG
#define SIZE_T_PF(x) size_t pf = (~x) & 1;
#endif

namespace menon {
    // Two variants of interpolation (vertical and horizontal)
    Bitmap InterpolateVertical  (const Bitmap& cfa);
    Bitmap InterpolateHorizontal(const Bitmap& cfa);
    Bitmap InterpolateDirectional(const Bitmap& cfa, Direction d);
    BitmapVH InterpolateVHInParallel(const Bitmap& cfa);
} // namespace menon