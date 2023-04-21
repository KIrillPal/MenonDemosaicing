#pragma once
#include "rgb.hpp"

namespace rgb {
    std::vector<uint16_t> PackRGB(const Bitmap& R, const Bitmap& G, const Bitmap& B) {
        size_t h = R.Height();
        size_t w = R.Width();
        size_t p = R.BytesPerPixel();
        std::vector<uint16_t> data;
        data.reserve(h * w * p);

        auto r = reinterpret_cast<const uint16_t*>(R.Data());
        auto g = reinterpret_cast<const uint16_t*>(G.Data());
        auto b = reinterpret_cast<const uint16_t*>(B.Data());

        for (size_t i = 0; i < h * w; ++i) {
            data.push_back(r[i]);
            data.push_back(g[i]);
            data.push_back(b[i]);
        }
        return data;
    }
} // namespace rgb
