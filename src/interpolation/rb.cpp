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

    // FIll C color on Green pixels
    // Implementation without SIMD
    // odd = 0 for red and 1 for blue
    void FillGreenCSimple(Bitmap& color, const Bitmap& chrom, int odd) {
        size_t h = chrom.Height();
        size_t w = chrom.Width();
        for (size_t x = 0; x < h; ++x) {
            SIZE_T_PF(x)
            bool is_c_row = (x & 1) == odd;

            for (size_t y = 1-pf; y < w; y += 2) {
                int c = color.Get<uint16_t>(x, y);
                if (is_c_row) {
                    c += chrom.GetSafe<int16_t>(x, y - 1);
                    c += chrom.GetSafe<int16_t>(x, y + 1);
                }
                else {
                    c += chrom.GetSafe<int16_t>(x - 1, y);
                    c += chrom.GetSafe<int16_t>(x + 1, y);
                }
                c = std::min(std::max(c, 0), UINT16_MAX);
                color.Set(x, y, static_cast<uint16_t>(c));
            }
        }
    }

    void FillGreenRBSimple(Bitmap& red, Bitmap& blue, const Bitmap& chrom) {
#if not defined (PARALLEL)
        std::thread fill_red ([&](){ FillGreenCSimple(red, chrom, 0); });
        std::thread fill_blue([&](){ FillGreenCSimple(blue, chrom, 1); });
        fill_red.join();
        fill_blue.join();
#else
        FillGreenCSimple(red, chrom, 0);
        FillGreenCSimple(blue, chrom, 1);
#endif
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

    BitmapVH InterpolateRBonGreen(const Bitmap& mosaic, const Bitmap& green) {
        Bitmap red (mosaic);
        Bitmap blue(mosaic);

        // Chrominance (R - G) / 2 or (B - G) / 2.
        auto chrom = mosaic;
        SubDiv2(chrom, green);

        FillGreenRB(red, blue, chrom);

        return BitmapVH{std::move(red), std::move(blue)};
    }

    void FillRBonRB(BitmapVH& rb, const Bitmap& diff) {
        FillRBRB(rb.V, rb.H, diff);
    }




} // namespace menon
