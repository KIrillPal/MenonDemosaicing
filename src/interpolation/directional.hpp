#pragma once
#include "../support/rgb.hpp"
#include "filter.hpp"

#define PARALLEL
#define SIMD
#define RGGB

namespace menon {
    // Two variants of interpolation (vertical and horizontal)
    Bitmap InterpolateVertical  (const Bitmap& mosaic);
    Bitmap InterpolateHorizontal(const Bitmap& mosaic);
    Bitmap InterpolateDirectional(const Bitmap& mosaic, Direction d);
    BitmapVH InterpolateVHInParallel(const Bitmap& mosaic);
} // namespace menon