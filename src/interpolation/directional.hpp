#pragma once
#include "../support/rgb.hpp"
#include "filter.hpp"

#define PARALLEL
#define SIMD

namespace menon {
    // Two variants of interpolation (vertical and horizontal)
    struct BitmapVH {
        Bitmap V, H;
    };
    Bitmap InterpolateVertical  (const Bitmap& mosaic);
    Bitmap InterpolateHorizontal(const Bitmap& mosaic);
    Bitmap InterpolateDirectional(const Bitmap& mosaic, Direction d);
    BitmapVH InterpolateVHInParallel(const Bitmap& mosaic);
} // namespace menon