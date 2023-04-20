#include <algorithm>
#include <immintrin.h>
#include <array>
#include <thread>
#include "directional.hpp"

#define GET_DIRECTIONAL(bmp, x, y, dir) (dir == HORIZONTAL ? bmp.Get(x, y) : bmp.Get(y, x))
#define GET_INT_DIRECTIONAL(bmp, x, y, dir) (static_cast<int>(dir == HORIZONTAL ? bmp.Get(x, y) : bmp.Get(y, x)))

namespace menon {

    //Interpolation variants (only headers):
    Bitmap InterpolateDirectionalSimple(const Bitmap& mosaic, Direction d);
    Bitmap InterpolateDirectionalWithSIMD(const Bitmap& mosaic, Direction d);

    // Interpolates green color in Bayer mosaic by direction d
    Bitmap InterpolateDirectional(const Bitmap& mosaic, Direction d) {
#ifdef SIMD
        return InterpolateDirectionalWithSIMD(mosaic, d);
#else
        return InterpolateDirectionalSimple(mosaic, d);
#endif
    }

    Bitmap InterpolateVertical(const Bitmap& mosaic) {
        return InterpolateDirectional(mosaic, Direction::VERTICAL);
    }
    Bitmap InterpolateHorizontal(const Bitmap& mosaic) {
        return InterpolateDirectional(mosaic, Direction::HORIZONTAL);
    }

    BitmapVH InterpolateVHInParallel(const Bitmap& mosaic) {
        BitmapVH result;
#ifdef PARALLEL
        std::thread vertical([&]() {
            result.V = std::move(InterpolateVertical(mosaic));
        });
        std::thread horizontal([&]() {
            result.H = std::move(InterpolateHorizontal(mosaic));
        });

        vertical.join();
        horizontal.join();
#else
        result.V = std::move(InterpolateVertical(mosaic));
        result.H = std::move(InterpolateHorizontal(mosaic));
#endif
        return result;
    }
    ////////////////////////////////////////////////////////////////////////////////////
    // Utils:

    // 4 * (FIR filter proposed in the article)
    constexpr int kMenonFilter4[] = {-1, 2, 2, 2, -1};
    constexpr size_t kMenonFilterSize = sizeof(kMenonFilter4) / sizeof(kMenonFilter4[0]);

    constexpr auto CreateFilterSubMatrix() {
        std::array<std::array<int, kMenonFilterSize>, kMenonFilterSize> matrix{0};
        for (size_t i = 0; i < kMenonFilterSize; ++i) {
            int segment_sum = 0;
            for (size_t j = i; j < kMenonFilterSize; ++j) {
                segment_sum += kMenonFilter4[j];
                matrix[i][j] = segment_sum;
            }
        }
        return matrix;
    }

    // Matrix of the sum of weights in the segment [i, j] of the filter
    constexpr auto kFilterSubMatrix = CreateFilterSubMatrix();


    ////////////////////////////////////////////////////////////////////////////////////
    // Implementations:

    // The simplest implementation. Safe to use in several threads
    Bitmap InterpolateDirectionalSimple(const Bitmap& mosaic, Direction d) {

        Bitmap dest = mosaic;
        size_t w = mosaic.Width();
        size_t h = mosaic.Height();
        if (d == Direction::VERTICAL) {
            std::swap(w, h);
        }

        // (FIR Filter proposed in the article) * 4
        constexpr size_t kFilterSize = 5;
        const int filter[kFilterSize] = {-1, 2, 2, 2, -1};

        // kOff - filter offset. Offset from begin of filter to the result pixel
        constexpr size_t kOff = kFilterSize >> 1;

        for (size_t x = 0; x < h; ++x)
        {
            // position of the first R or B in a row
            SIZE_T_PF(x);

            for (size_t y = pf; y < w; y += 2) {

                // Apply the filter
                int sum = 0;
                // i from y - kOFF to y + kOff; y - center of the filter
                for (size_t i = (y >= kOff ? y - kOff : 0); i <= y + kOff && i < w; ++i) {
                    sum += GET_INT_DIRECTIONAL(mosaic, x, i, d) * filter[i - y + kOff];
                }

                // Compensation if a part of the filter is out of border
                if (y < kOff || y + kOff >= w) {
                    sum *= static_cast<int>(kFilterSubMatrix[0][kFilterSize - 1]);
                    sum /= kFilterSubMatrix
                            [y < kOff ? kOff - y : 0] /*first in filter*/
                            [y + kOff >= w ? kFilterSize - 2 - y - kOff + w : kFilterSize - 1]; /*last*/
                }


                // finally divide by 4 (because we chose filter * 4)
                sum >>= 2;
                // avoid overflow
                sum = std::min(std::max(sum, 0), UINT16_MAX);

                // Set to the matrix
                if (d == HORIZONTAL) {
                    dest.Set(x, y, static_cast<uint16_t>(sum));
                }
                else {
                    dest.Set(y, x, static_cast<uint16_t>(sum));
                }
            }
        }
        return dest;
    }

    // The implementation with SIMD. Safe to use in several threads
    // BE CAREFUL: filter length must not exceed SIMD_ITEMS_SIZE, it's SIMD_BITS_SIZE bits
    Bitmap InterpolateDirectionalWithSIMD(const Bitmap& mosaic, Direction d) {
        constexpr size_t SIMD_BITS_SIZE = 128;
        constexpr size_t SIMD_ITEMS_SIZE = (SIMD_BITS_SIZE >> 3) / sizeof(int);

        Bitmap dest = mosaic;
        size_t w = mosaic.Width();
        size_t h = mosaic.Height();
        if (d == Direction::VERTICAL) {
            std::swap(w, h);
        }

        // (FIR Filter proposed in the article) * 4
        // To use SIMD we need 128b filter
        // Put in reversed order [-1 <- not included, 2, 2, 2, -1];
        // -1 included in the last part of computation
        __m128i filter_128b_part = _mm_set_epi32(2, 2, 2, -1); // -1 rest
        constexpr size_t kFilterSize = 5;

        // OFF - filter offset. Offset from begin of filter to the result pixel
        constexpr size_t OFF = kFilterSize >> 1;

        constexpr size_t ZERO_OFFSET = SIMD_ITEMS_SIZE;
        // Row to get SIMD_BITS_SIZE bits (for Menon Filter 5 values and 3 zeroes: [a,b,c,d,e,0,0,0]) to use intrinsics
        std::vector<int> row(w + 2 * ZERO_OFFSET/*offset to grab all zeros at the start and the end of the row*/);

        for (size_t x = 0; x < h; ++x)
        {
            // position of the first R or B in a row
            SIZE_T_PF(x)

            row.assign(row.size(), 0);

            // set 0-st value if pf
            row[ZERO_OFFSET] = GET_INT_DIRECTIONAL(mosaic, x, 0, d);

            for (size_t y = pf; y < OFF + pf && y < w; ++y) {
                row[y + ZERO_OFFSET] = GET_INT_DIRECTIONAL(mosaic, x, y, d);
            }
            for (size_t y = OFF + pf; y < w + OFF; y += 2) {
                if (y-1 < w) {
                    row[y + ZERO_OFFSET - 1] = GET_INT_DIRECTIONAL(mosaic, x, y - 1, d);
                }

                // Apply the filter
                int* values_128b = row.data() + y - 2 * OFF + ZERO_OFFSET;
                __m128i values = _mm_loadu_si128((__m128i*)values_128b);
                __m128i mult = _mm_mullo_epi32(values, filter_128b_part);
                int mult_128b[SIMD_ITEMS_SIZE];
                _mm_storeu_si128((__m128i*)mult_128b, mult);

                // In the future we need to add last item to the sum (missed -1). So load its level to the row now
                if (y < w) {
                    row[y + ZERO_OFFSET] = GET_INT_DIRECTIONAL(mosaic, x, y, d);
                }

                // Count a part of sum
                int sum = -row[y + ZERO_OFFSET];
                for (size_t i = 0; i < SIMD_ITEMS_SIZE; ++i) {
                    sum += mult_128b[i];
                }

                // if the filter is partially out of bounds, offset brightness
                if (y < 2 * OFF || w <= y) {
                    sum *= kFilterSubMatrix[0][kFilterSize - 1];
                    sum /= kFilterSubMatrix
                            [y < 2*OFF ? kFilterSize - y - 1 : 0]  /*first in filter*/
                            [y >= w ? kFilterSize - y - 2 + w : kFilterSize - 1]; /*last*/
                }


                // finally divide by 4 (because we chose filter * 4)
                sum >>= 2;
                // avoid overflow
                sum = std::min(std::max(sum, 0), UINT16_MAX);

                // Set to the matrix
                if (d == HORIZONTAL) {
                    dest.Set(x, y-OFF, static_cast<uint16_t>(sum));
                }
                else {
                    dest.Set(y-OFF, x, static_cast<uint16_t>(sum));
                }
            }
        }
        return dest;
    }
} // namespace menon