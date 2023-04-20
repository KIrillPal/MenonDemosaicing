#pragma once
#include <cstdint>
#include <memory>
#include <cassert>
#include <cstring>
#include <vector>

// Class of pixel array with only one channel
// Copyable
// Trivially movable
class Bitmap {
public:
    // Type of the maximum sample(pixel) size.
    // Used in 'Get' function to make it able to return code of each size.
    using LARGEST_TYPE = uint64_t;
    // Offset to let us read entire LARGEST_TYPE variable even at the last pixel
    static constexpr size_t DATA_SAFE_OFFSET = sizeof(uint64_t);
//////////////////////////////////////////////////////////////////////////////////

    Bitmap() = default;

    Bitmap(size_t height, size_t width, uint16_t bytes_per_pixel)
            : h_{height},
              w_{width},
              p_{bytes_per_pixel},
              data_{new uint8_t[height * width * bytes_per_pixel + DATA_SAFE_OFFSET]},
              mask_{(1ULL << (bytes_per_pixel << 3)) - 1} {
    }

    // Copyable
    Bitmap(const Bitmap& other)
      : Bitmap(std::move(other.Copy())) {
    }
    Bitmap& operator =(const Bitmap& other) {
        return *this = std::move(other.Copy());;
    }

    // Trivially movable
    Bitmap(Bitmap&&) = default;
    Bitmap& operator =(Bitmap&&) = default;


    size_t Width() const {
        return w_;
    }
    size_t Height() const {
        return h_;
    }
    size_t BytesPerPixel() const {
        return p_;
    }
    const uint8_t* Data() const {
        return data_.get();
    }
    uint8_t* Data() {
        return data_.get();
    }

    // Copies current bitmap
    Bitmap Copy() const {
        Bitmap cp(h_, w_, p_);
        std::copy(data_.get(), data_.get() + h_*w_*p_, cp.data_.get());
        return cp;
    }

    // Returns the value of pixel (x, y) of Bitmap
    // 0 <= x < height, 0 <= y < width
    LARGEST_TYPE Get(size_t x, size_t y) const {
        assert(x < h_ && y < w_);
        size_t offset = (x * w_ + y) * p_;
        LARGEST_TYPE result = *reinterpret_cast<LARGEST_TYPE*>(data_.get() + offset);
        return result & mask_;
    }

    // Just reads sizeof(T) bytes from pixel data
    // The same as Get but faster and unsafe
    template <typename T>
    T Get(size_t x, size_t y) const {
        assert(x < h_ && y < w_);          // the same as Get()
        size_t offset = (x * w_ + y) * p_; // the same as Get()
        return *reinterpret_cast<T*>(data_.get() + offset);
    }
    // non-const version
    template <typename T>
    T& Get(size_t x, size_t y) {
        assert(x < h_ && y < w_);
        size_t offset = (x * w_ + y) * p_;
        return *reinterpret_cast<T*>(data_.get() + offset);
    }

    template <typename T>
    void Set(size_t x, size_t y, T value) {
        assert(x < h_ && y < w_);
        size_t offset = (x * w_ + y) * p_;
        *reinterpret_cast<T*>(data_.get() + offset) = value;
    }

private:
    std::unique_ptr<uint8_t> data_{nullptr};
    size_t w_{0}; // width
    size_t h_{0}; // height
    size_t p_{0}; // bytes per pixel
    LARGEST_TYPE mask_{0}; // mask to get first p_ bytes from LARGEST_TYPE
};

// a pair of bitmaps with different orientation
struct BitmapVH {
    Bitmap V, H;
};