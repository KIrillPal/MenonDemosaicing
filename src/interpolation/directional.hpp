#pragma once
#include "../support/bitmap.hpp"
#include "../support/pf.hpp"
#include "filter.hpp"

namespace menon {
    // Two variants of interpolation (vertical and horizontal)
    Bitmap InterpolateVertical  (const Bitmap& cfa);
    Bitmap InterpolateHorizontal(const Bitmap& cfa);
    Bitmap InterpolateDirectional(const Bitmap& cfa, Direction d);
    BitmapVH InterpolateGreenVH(const Bitmap& cfa);
} // namespace menon
