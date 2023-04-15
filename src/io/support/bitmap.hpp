#pragma once
#include <cstdint>
#include <memory>
#include <cassert>
#include <cstring>

// Class of pixel array with only one channel
// Its purpose to share one bitmap between several threads
// Trivially copyable and movable
class SharedBitmap {
    // Type of the maximum sample(pixel) size.
    // Used in 'Get' function to make it able to return code of each size.
    using LARGEST_TYPE = uint64_t;
    // Offset to let us read entire LARGEST_TYPE variable even at the last pixel
    static constexpr size_t DATA_SAFE_OFFSET = sizeof(uint64_t);
public:
    SharedBitmap(size_t height, size_t width, uint16_t bytes_per_pixel)
            : h_{height},
              w_{width},
              p_{bytes_per_pixel},
              data_{new uint8_t[height * width * bytes_per_pixel + DATA_SAFE_OFFSET]},
              mask_{(1ULL << (p_ << 3)) - 1} {
    }

    // Trivially copyable and movable
    SharedBitmap(const SharedBitmap&) = default;
    SharedBitmap& operator =(const SharedBitmap&) = default;

    SharedBitmap(SharedBitmap&&) = default;
    SharedBitmap& operator =(SharedBitmap&&) = default;


    size_t Width() const {
        return w_;
    }
    size_t Height() const {
        return h_;
    }
    size_t BytesPerPixel() const {
        return p_;
    }
    uint8_t* Data() {
        return data_.get();
    }

    // Returns the value of pixel (x, y) of Bitmap
    // 0 <= x < height, 0 <= y < width
    LARGEST_TYPE Get(size_t x, size_t y) const {
        assert(x < h_ && y < w_);
        size_t offset = (x * w_ + y) * p_;
        LARGEST_TYPE result = *reinterpret_cast<LARGEST_TYPE*>(data_.get() + offset);
        return result & mask_;
    }

    template <typename T>
    T Get(size_t x, size_t y) const {
        return static_cast<T>(Get(x, y));
    }
private:
    std::shared_ptr<uint8_t> data_;
    size_t w_{0}; // width
    size_t h_{0}; // height
    size_t p_{0}; // bytes per pixel
    LARGEST_TYPE mask_{0}; // mask to get first p_ bytes from LARGEST_TYPE
};
