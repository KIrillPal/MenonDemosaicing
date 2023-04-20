#include <thread>
#include <iostream>
#include "posteriori.hpp"
#include "../support/bitmap_arithmetics.hpp"

namespace menon {
    // turns
    Bitmap GetChrominance(const Bitmap& mosaic, const Bitmap& layer) {
        Bitmap chrominance = mosaic;
        Sub(chrominance, layer);
        //std::cout << "copy-layer " << chrominance.Get<uint16_t>(8, 12) << '\n';
        Abs(chrominance);
        return chrominance;
    }

    Bitmap GetGradient(const Bitmap& mosaic, const Bitmap& layer, size_t dx, size_t dy) {
        Bitmap chrominance = GetChrominance(mosaic, layer);
        Div2(chrominance);
        SubShifted(chrominance, chrominance, dx, dy);
        Abs(chrominance);
        return chrominance;
    }

    BitmapVH GetGradients(const Bitmap& mosaic, const BitmapVH& layers) {
#ifdef PARALLEL
        BitmapVH grads;
        std::thread vertical([&]() {
            grads.V = GetGradient(mosaic, layers.V, 2, 0);
        });
        std::thread horizontal([&]() {
            grads.H = GetGradient(mosaic, layers.H, 0, 2);
        });
        vertical.join();
        horizontal.join();
        return grads;
#else
        return BitmapVH{
            GetGradient(mosaic, layers.V, 2, 0),
            GetGradient(mosaic, layers.H, 0, 2)
        };
#endif
    }

    Bitmap GetClassifierDifference(const Bitmap& mosaic, const BitmapVH& interpolation) {
        constexpr size_t AREA_SIZE = 5;
        constexpr size_t AREA_HALF = AREA_SIZE >> 1;

        auto grads = GetGradients(mosaic, interpolation);
        Sub(grads.H, grads.V);
        // now grads.H is the difference beween gradients.
        // To get classifier difference let's sum grades by the area of 5x5.
        // if out of bounds just sum the rest

        size_t w = mosaic.Width();
        size_t h = mosaic.Height();

        // Prepare sum of column y: sum of the window [x - AREA_HALF, ..., x + AREA_HALF]
        std::vector<unsigned> column_sum(w);
        for (size_t y = 0; y < w; ++y) {
            for (size_t x = 0; x <= AREA_HALF && x < h; ++x) {
                column_sum[y] += grads.H.Get<int16_t>(x, y);
            }
        }

        Bitmap diff(h, w, sizeof(int));

        for (size_t x = 0; x < h; ++x) {
            int current_area_sum = 0;
            for (size_t y = 0; y <= AREA_HALF && y < w; ++y) {
                current_area_sum += column_sum[y];
            }

            for (size_t y = 0; y < w; ++y) {
                diff.Set(x, y, current_area_sum);

                // Move the square window
                if (y + AREA_HALF + 1 < w) {
                    current_area_sum += column_sum[y + AREA_HALF + 1];
                }
                if (y >= AREA_HALF) {
                    current_area_sum -= column_sum[y - AREA_HALF];
                    // Move the column window
                    if (x >= AREA_HALF) {
                        column_sum[y - AREA_HALF] -= grads.H.Get<int16_t>(x - AREA_HALF, y - AREA_HALF);
                    }
                    if (x + AREA_HALF + 1 < h) {
                        column_sum[y - AREA_HALF] += grads.H.Get<int16_t>(x + AREA_HALF + 1, y - AREA_HALF);
                    }
                }
            }
            for (size_t p = 0; p < AREA_HALF && p < h; ++p) {
                // Move the column window
                if (x >= AREA_HALF) {
                    column_sum[h-p-1] -= grads.H.Get<int16_t>(x - AREA_HALF, h-p-1);
                }
                if (x + AREA_HALF + 1 < h) {
                    column_sum[h-p-1] += grads.H.Get<int16_t>(x + AREA_HALF + 1, h-p-1);
                }
            }
        }
        return diff;
    }

    Bitmap Posteriori(const BitmapVH& interpolation, const Bitmap& diff) {
        size_t w = diff.Width();
        size_t h = diff.Height();
        Bitmap merged(h, w, interpolation.V.BytesPerPixel());
        for (size_t x = 0; x < h; ++x) {
            for (size_t y = 0; y < w; ++y) {
                // check if classifier h < classifier v
                if (diff.Get<int>(x, y) < 0) {
                    merged.Set(x, y, interpolation.H.Get<uint16_t>(x, y));
                }
                else {
                    merged.Set(x, y, interpolation.V.Get<uint16_t>(x, y));
                }
            }
        }
        return merged;
    }
} // namespace menon
