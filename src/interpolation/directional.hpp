#pragma once
#include "../support/rgb.hpp"
#include "filter.hpp"

namespace menon {
    Bitmap InterpolateVertical  (const Bitmap& mosaic);
    Bitmap InterpolateHorizontal(const Bitmap& mosaic);
    Bitmap InterpolateDirectional(const Bitmap& mosaic, FIRFilter filter, Direction d);
} // namespace menon