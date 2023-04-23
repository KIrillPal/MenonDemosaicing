#pragma once
#include <thread>
#include "../support/bitmap.hpp"
#include "../support/bitmap_arithmetics.hpp"
#include "../support/pf.hpp"

namespace refine {

    // odd = 0 for red and 1 for blue
    void RefineConG(Bitmap& c, const BitmapVH& lpVH, const Bitmap& hpG, int odd) {
        size_t h = c.Height();
        size_t w = c.Width();
        auto c32 = CopyCast32(c);
        AddShifted(c32, c32, 0, 0);

        for (size_t x = 0; x < h; ++x) {
            SIZE_T_PF(x)
            bool is_c_row = (x & 1) == odd;
            for (size_t y = 1-pf; y < w; y += 2) {
                int lpC = (is_c_row ? lpVH.H : lpVH.V).Get<int>(x, y);
                int hpC = c32.Get<int>(x, y) - lpC;
                int normalized = c.Get<uint16_t>(x, y) + (hpG.Get<int>(x, y) - hpC) / 3;
                uint16_t new_c = std::min(std::max(normalized, 0), UINT16_MAX);
                c.Set(x, y, new_c);
            }
        }
    }

    // Refines red and blue colors ONLY FOR GREEN PIXELS
    void RefineRBonG(BitmapVH& rb, const BitmapVH& lpVH, const Bitmap& hpG) {
#if defined(PARALLEL)
        std::thread red ([&](){ RefineConG(rb.V, lpVH, hpG, 0); });
        std::thread blue([&](){ RefineConG(rb.H, lpVH, hpG, 1); });
        red.join();
        blue.join();
#else
        RefineConG(rb.V, lpVH, hpG, 0);
        RefineConG(rb.H, lpVH, hpG, 1);
#endif
    }

    void RefineGonRB(Bitmap& green, const Bitmap& hpG, const Bitmap& hpRR) {
        for (size_t x = 0; x < green.Height(); ++x) {
            SIZE_T_PF(x);
            for (size_t y = pf; y < green.Width(); y += 2) {
                int g = green.Get<uint16_t>(x, y);
                g += (hpRR.Get<int>(x, y) - hpG.Get<int>(x, y)) / 3;
                uint16_t new_g = std::min(std::max(g, 0), UINT16_MAX);
                green.Set(x, y, new_g);
            }
        }
    }

    void RefineRBonRB(BitmapVH& rb, const Bitmap& hpRR, const Bitmap& diff) {
        size_t h = rb.V.Height();
        size_t w = rb.V.Width();
        for (size_t x = 0; x < h; ++x) {
            SIZE_T_PF(x);
            bool is_red_row = (~x) & 1;
            Bitmap& c = (is_red_row ? rb.H : rb.V);
            for (size_t y = pf; y < w; y += 2) {
                int v = c.Get<uint16_t>(x, y);
                int his_hp = v << 1;
                int my_hp  = hpRR.Get<int>(x, y);
                if (diff.Get<int>(x, y) < 0) {
                    his_hp -= c.GetSafe<uint16_t>(x, y-1);
                    his_hp -= c.GetSafe<uint16_t>(x, y+1);
                } else {
                    his_hp -= c.GetSafe<uint16_t>(x-1, y);
                    his_hp -= c.GetSafe<uint16_t>(x+1, y);
                }
                v += (my_hp - his_hp) / 3;
                uint16_t new_c = std::min(std::max(v, 0), UINT16_MAX);
                c.Set(x, y, new_c);
            }
        }
    }
}
