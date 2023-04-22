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


    // Computes low-pass filter asynchronously
    std::future<BitmapVH> GetLowpassFilterVHAsync(const Bitmap& cfa32);
} // namespace lp