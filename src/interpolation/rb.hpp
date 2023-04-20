#pragma once
#include "directional.hpp"

namespace menon {
    // Calculates red and blue colors using original image and calculated green color
    // Returns structure BitmapVH, where V is red and H is blue
    // In other words returns BitmapVH{ red, blue };
    //
    // diff - the difference of classifiers of each pixel
    BitmapVH InterpolateRB(const Bitmap& mosaic, const Bitmap& green, const Bitmap& diff);
}