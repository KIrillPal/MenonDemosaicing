#pragma once
#include "../support/bitmap.hpp"
namespace menon {
    // Merge two interpolations using a posteriori decision
    // Classifier difference is a difference between vertical and horizontal classifiers
    // for each pixel
    Bitmap Posteriori(const BitmapVH& interpolation, const Bitmap& classifier_difference);

    // Computes the difference between vertical and horizontal classifiers of given interpolations
    // for each pixel
    Bitmap GetClassifierDifference(const Bitmap& mosaic, const BitmapVH& interpolation);

} // namespace menon
