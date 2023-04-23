#pragma once
#include <future>
#include "../support/bitmap.hpp"

namespace lp {
    // Computes Low-pass filter for each pixel
    // in vertical and horizontal directions
    // cfa32 - bayer mosaic with 32-bit color
    // Returns a pair of Bitmap<int>
    BitmapVH FilterVH(const Bitmap& cfa32);
    // NOTE: Simplified low-pass. Due to optimization
    // returns pixels after FIR [1 0 1] for two directions
    // To get high-pass R: hpR = (2 * R - lpR) / 3


    // Computes High-pass filter for each GREEN pixel
    // lpVH  - simplified low-pass filter for two directions
    // green - green layer
    // diff  - difference between classifiers
    // Returns Bitmap<int>
    Bitmap HighpassG(const BitmapVH& lpVH, const Bitmap& green, const Bitmap& diff);

    // Computes High-pass filter for each red and blue pixel
    // rb - pair where rb.V is red and rb.H is blue
    // diff  - difference between classifiers
    // Returns Bitmap<int>
    Bitmap HighpassRonR(const BitmapVH& rb, const Bitmap& diff);

    // Computes simplified low-pass filter for every pixel asynchronously
    std::future<BitmapVH> GetLowpassFilterVHAsync(const Bitmap& cfa32);

    // Computes high-pass filter for every GREEN pixel asynchronously
    std::future<Bitmap> GetHighpassFilterGAsync(const BitmapVH& lpVH, const Bitmap& green, const Bitmap& diff);

    // Computes high-pass R/B filter for every R/B pixel asynchronously; R for R and B for B
    std::future<Bitmap> GetHighpassFilterRonRAsync(const BitmapVH& rb, const Bitmap& diff);
} // namespace lp