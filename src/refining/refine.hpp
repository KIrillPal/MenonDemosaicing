#pragma once
#include "../support/bitmap.hpp"

namespace refine {

    // Refines red and blue colors ONLY FOR GREEN PIXELS
    void RefineRBonG(BitmapVH& rb, const BitmapVH& lpVH, const Bitmap& hpG);

    // Refines green color ONLY FOR R/B PIXELS
    void RefineGonRB(Bitmap& green, const Bitmap& hpG, const Bitmap& hpRR);

    // Refines r/b color ONLY FOR R/B PIXELS
    void RefineRBonRB(BitmapVH& rb, const Bitmap& hpRR, const Bitmap& diff);
}
