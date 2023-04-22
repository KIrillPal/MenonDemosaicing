#pragma once
#include "directional.hpp"

namespace menon {
    // Calculates red and blue colors ONLY FOR GREEN PIXELS
    // from original image and calculated green color
    // Returns structure BitmapVH, where V is red and H is blue
    // In other words returns BitmapVH{ red, blue };
    BitmapVH InterpolateRBonGreen(const Bitmap& mosaic, const Bitmap& green);

    // Changes red and blue colors FOR RED AND BLUE PIXELS
    //
    // rb - pair of red and blue color as rb.V and rb.H respectively
    // diff - the difference of classifiers of each pixel
    void FillRBonRB(
            BitmapVH& rb,
            const Bitmap& diff
            );
}