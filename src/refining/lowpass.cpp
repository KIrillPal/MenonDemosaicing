#include <thread>
#include "../support/bitmap_arithmetics.hpp"
#include "../support/pf.hpp"
#include "lowpass.hpp"

#include <iostream>

namespace lp {
    BitmapVH FilterVH(const Bitmap& cfa32) {
#if defined(PARALLEL) && defined(BUG_FIXED)
        // low pass for two directions
        BitmapVH lp;
        std::thread vertical([&](){
            lp.V = Bitmap{
                cfa32.Height(), cfa32.Width(),
                static_cast<uint16_t>(cfa32.BytesPerPixel())
            };
            FillWithZeros(lp.V);
            SubShifted(lp.V, cfa32, -1, 0);
            SubShifted(lp.V, cfa32,  1, 0);
        });
        std::thread horizontal([&](){
            lp.H = Bitmap{
                    cfa32.Height(), cfa32.Width(),
                    static_cast<uint16_t>(cfa32.BytesPerPixel())
            };
            FillWithZeros(lp.H);
            AddShifted(lp.H, cfa32, 0, -1);
            AddShifted(lp.H, cfa32, 0,  1);
        });
        vertical.join();
        horizontal.join();
#else
        BitmapVH lp = std::move(BitmapVH::Create(cfa32.Height(), cfa32.Width(), sizeof(int)));
        FillWithZeros(lp.V);
        FillWithZeros(lp.H);
        AddShifted(lp.V, cfa32, -1, 0);
        AddShifted(lp.V, cfa32,  1, 0);
        AddShifted(lp.H, cfa32, 0, -1);
        AddShifted(lp.H, cfa32, 0,  1);
#endif
        return lp;
    }

    void SubLowpassGonGreen(Bitmap& hp, const Bitmap& green, const Bitmap& diff) {
        auto data = reinterpret_cast<int*>(hp.Data());

        size_t h = green.Height();
        size_t w = green.Width();
        size_t row_pos = 0;
        for (size_t x = 0; x < h; ++x) {
            SIZE_T_PF(x)
            for (size_t y = 1-pf; y < w; y += 2) {
                // check if delta_H < delta_V => use H
                if (diff.Get<int>(x, y) < 0) {
                    data[row_pos + y] -= green.GetSafe<uint16_t>(x, y-1);
                    data[row_pos + y] -= green.GetSafe<uint16_t>(x, y+1);
                } else {
                    data[row_pos + y] -= green.GetSafe<uint16_t>(x-1, y);
                    data[row_pos + y] -= green.GetSafe<uint16_t>(x+1, y);
                }
            }
            row_pos += w;
        }
    }

    void SubLowpassGonRB(Bitmap& hp, const BitmapVH& lpVH, const Bitmap& diff) {
        size_t h = hp.Height();
        size_t w = hp.Width();

        auto data = reinterpret_cast<int*>(hp.Data());
        auto lpv = reinterpret_cast<const int*>(lpVH.V.Data());
        auto lph = reinterpret_cast<const int*>(lpVH.H.Data());

        for (size_t x = 0; x < h; ++x) {
            SIZE_T_PF(x)
            for (size_t y = pf; y < w; y += 2) {
                // check if delta_H < delta_V => use H
                if (diff.Get<int>(x, y) < 0) {
                    data[x * w + y] -= lph[x * w + y];
                } else {
                    data[x * w + y] -= lpv[x * w + y];
                }
            }
        }
    }

    Bitmap HighpassG(const BitmapVH& lpVH, const Bitmap& green, const Bitmap& diff) {
        // Get hp = 2 * green
        auto green32 = CopyCast32(green);
        Bitmap hp = green32;
        AddShifted(hp, green32, 0, 0);
#if defined(PARALLEL)
        std::thread on_green([&](){
            SubLowpassGonGreen(hp, green, diff);
        });
        std::thread on_rb([&](){
            SubLowpassGonRB(hp, lpVH, diff);
        });
        on_green.join();
        on_rb.join();
#else
        SubLowpassGonGreen(hp, green, diff);
        SubLowpassGonRB(hp, lpVH, diff);
#endif
        return hp;
    }

    Bitmap HighpassRonR(const BitmapVH& rb, const Bitmap& diff) {
        //auto r32 = CopyCast32(rb.V);
        //auto b32 = CopyCast32(rb.H);

        // Using the fact that rb has original values from the mosaic
        // on the r/b positions
        // so use them as mosaic items
        Bitmap hp = CopyCast32(rb.V);
        auto data = reinterpret_cast<int*>(hp.Data());

        size_t h = diff.Height();
        size_t w = diff.Width();
        size_t row_pos = 0;
        for (size_t x = 0; x < h; ++x) {
            SIZE_T_PF(x)
            bool is_red_row = (~x) & 1;
            for (size_t y = 1-pf; y < w; y += 2) {
                // check if delta_H < delta_V => use H
                const Bitmap& c = (is_red_row ? rb.V : rb.H);
                if (diff.Get<int>(x, y) < 0) {
                    data[row_pos + y] -= c.GetSafe<uint16_t>(x, y-1);
                    data[row_pos + y] -= c.GetSafe<uint16_t>(x, y+1);
                } else {
                    data[row_pos + y] -= c.GetSafe<uint16_t>(x-1, y);
                    data[row_pos + y] -= c.GetSafe<uint16_t>(x+1, y);
                }
            }
            row_pos += w;
        }
        return hp;
    }

//////////////////////////////////////////////////////////////////////////////////
// Async run:

    std::future<BitmapVH> GetLowpassFilterVHAsync(const Bitmap& cfa32) {
        std::promise<BitmapVH> result;
        auto future = result.get_future();
#if defined(PARALLEL)
        std::thread lowpass([&cfa32, p{std::move(result)}]() mutable{
            p.set_value(FilterVH(cfa32));
        });
        lowpass.detach();
#else
        result.set_value(FilterVH(cfa32));
#endif
        return future;
    }

    std::future<Bitmap> GetHighpassFilterGAsync(const BitmapVH& lpVH, const Bitmap& green, const Bitmap& diff) {
        std::promise<Bitmap> result;
        auto future = result.get_future();
#if defined(PARALLEL)
        std::thread highpass([&, p{std::move(result)}]() mutable {
            p.set_value(HighpassG(lpVH, green, diff));
        });
        highpass.detach();
#else
        result.set_value(HighpassG(lpVH, green, diff));
#endif
        return future;
    }

    std::future<Bitmap> GetHighpassFilterRonRAsync(const BitmapVH& rb, const Bitmap& diff) {
        std::promise<Bitmap> result;
        auto future = result.get_future();
#if defined(PARALLEL)
        std::thread highpass([&, p{std::move(result)}]() mutable {
            p.set_value(HighpassRonR(rb, diff));
        });
        highpass.detach();
#else
        result.set_value(HighpassRonR(rb, diff));
#endif
        return future;
    }
} // namespace lp
