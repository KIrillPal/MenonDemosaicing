#pragma once
#include "bitmap.hpp"

// Bitmap with three channels: R,G,B
// May be used to get access to the image from several threads
class ImageRGB {
public:
    ImageRGB(size_t height, size_t width, uint16_t bytes_per_pixel)
      : r_(height, width, bytes_per_pixel),
        g_(height, width, bytes_per_pixel),
        b_(height, width, bytes_per_pixel) {
    }

    ImageRGB(SharedBitmap R, SharedBitmap B, SharedBitmap G)
            : r_{std::move(R)}, g_{std::move(G)}, b_{std::move(B)} {
    }

    // Trivially copyable and movable

    SharedBitmap::LARGEST_TYPE R(size_t x, size_t y) const {
        return r_.Get(x, y);
    }
    SharedBitmap::LARGEST_TYPE G(size_t x, size_t y) const {
        return g_.Get(x, y);
    }
    SharedBitmap::LARGEST_TYPE B(size_t x, size_t y) const {
        return b_.Get(x, y);
    }
private:
    SharedBitmap r_, g_, b_;
};
