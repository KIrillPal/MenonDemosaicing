#pragma once
#include "bitmap.hpp"

namespace rgb {

    struct BitmapRGB {
        Bitmap R, G, B;
    };

    // Gathers all three layers into one image with structure RGBRGBRGB...
    // BE CAREFUL: All layers must have one size and bytes per pixel
    std::vector<uint16_t> PackRGB(const Bitmap& R, const Bitmap& G, const Bitmap& B);
} // namespace rgb
