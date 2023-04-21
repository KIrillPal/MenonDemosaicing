#include "rb.hpp"
#include "../support/bitmap_arithmetics.hpp"
#include <thread>

namespace menon {

    /////////////////////////////////////////////////////////////////////
    // Variants:
    void FillGreenRBSimple(Bitmap& red, Bitmap& blue, const Bitmap& chrom);
    void FillRBRBSimple(Bitmap& red, Bitmap& blue, const Bitmap& chrom);

    ////////////////////////////////////////////////////////////////////////////////////
    // Implementations:

    // Fills Red and Blue for ONLY green pixels of the mosaic
    // red and blue must be copies of the mosaic
    // chrom is a chrominance 'R-G and B-G' matrix
    void FillGreenRB(Bitmap& red, Bitmap& blue, const Bitmap& chrom) {
        // use of SIMD is ineffective here
        FillGreenRBSimple(red, blue, chrom);
    }

    // Fills Red and Blue for red and blue pixels of the mosaic
    // red and blue must be copies of the mosaic except green pixels
    // diff is a difference between classifiers
    void FillRBRB(Bitmap& red, Bitmap& blue, const Bitmap& diff) {
        // use of SIMD is ineffective here
        FillRBRBSimple(red, blue, diff);
    }

    // Implementation without SIMD
    void FillGreenRBSimple(Bitmap& red, Bitmap& blue, const Bitmap& chrom) {
        size_t h = chrom.Height();
        size_t w = chrom.Width();
        for (size_t x = 0; x < h; ++x) {
            SIZE_T_PF(x)
            bool is_red_row = (~x) & 1;

            for (size_t y = 1-pf; y < w; y += 2) {
                int r = red.Get<uint16_t>(x, y);
                int b = r;
                if (is_red_row) {
                    r += chrom.GetSafe<int16_t>(x, y - 1);
                    r += chrom.GetSafe<int16_t>(x, y + 1);
                    b += chrom.GetSafe<int16_t>(x - 1, y);
                    b += chrom.GetSafe<int16_t>(x + 1, y);
                }
                else {
                    r += chrom.GetSafe<int16_t>(x - 1, y);
                    r += chrom.GetSafe<int16_t>(x + 1, y);
                    b += chrom.GetSafe<int16_t>(x, y - 1);
                    b += chrom.GetSafe<int16_t>(x, y + 1);
                }
                r = std::min(std::max(r, 0), UINT16_MAX);
                b = std::min(std::max(b, 0), UINT16_MAX);
                red.Set(x, y, static_cast<uint16_t>(r));
                blue.Set(x, y, static_cast<uint16_t>(b));
            }
        }
    }

    // Implementation without SIMD
    void FillRBRBSimple(Bitmap& red, Bitmap& blue, const Bitmap& diff) {
        size_t h = red.Height();
        size_t w = red.Width();

        // (R - B) / 2;
        // we lose quality(last bit) but win speed
        auto rb_chrom = red;
        SubDiv2(rb_chrom, blue);

        for (size_t x = 0; x < h; ++x) {
            SIZE_T_PF(x)
            bool is_red_row = (~x) & 1;

            for (size_t y = pf; y < w; y += 2) {
                int c = (is_red_row ? red : blue).Get<uint16_t>(x, y);
                int sum = 0;
                if (diff.Get<int>(x, y) < 0) {
                    sum += rb_chrom.GetSafe<int16_t>(x, y - 1);
                    sum += rb_chrom.GetSafe<int16_t>(x, y + 1);
                }
                else {
                    sum += rb_chrom.GetSafe<int16_t>(x - 1, y);
                    sum += rb_chrom.GetSafe<int16_t>(x + 1, y);
                }
                c += (is_red_row ? -sum : sum );
                c = std::min(std::max(c, 0), UINT16_MAX);

                (is_red_row ? blue : red).Set(x, y, static_cast<uint16_t>(c));
            }
        }
    }

    BitmapVH InterpolateRB(const Bitmap& mosaic, const Bitmap& green, const Bitmap& diff) {
        Bitmap red (mosaic);
        Bitmap blue(mosaic);

        // Chrominance (R - G) / 2 or (B - G) / 2.
        auto chrom = mosaic;
        SubDiv2(chrom, green);

        FillGreenRB(red, blue, chrom);
        FillRBRB(red, blue, diff);

        return BitmapVH{std::move(red), std::move(blue)};
    }




} // namespace menon
