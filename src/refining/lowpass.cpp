#include <thread>
#include "../support/bitmap_arithmetics.hpp"
#include "lowpass.hpp"

namespace lp {
    BitmapVH FilterVH(const Bitmap& cfa32) {
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
        return lp;
    }

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
} // namespace lp
