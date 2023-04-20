#include "bitmap_arithmetics.hpp"
#include <cmath>

//Operation variants (only headers):
void SubShiftedSimple(Bitmap& b1, const Bitmap& b2, int dx, int dy);
void SubShiftedRightBottomWithSIMD(Bitmap& b1, const Bitmap& b2, unsigned dx, unsigned dy);


void SubShifted(Bitmap& b1, const Bitmap& b2, int dx, int dy) {
#ifdef SIMD
    SubShiftedRightBottomWithSIMD(b1, b2, dx, dy);
#else
    SubShiftedSimple(b1, b2, dx, dy);
#endif
}

////////////////////////////////////////////////////////////////////////////////////
// Implementations:

void SubShiftedSimple(Bitmap& b1, const Bitmap& b2, int dx, int dy) {
    size_t min_x = std::max(0, -dx);
    size_t min_y = std::max(0, -dy);

    for (size_t x = min_x; x + dx < b1.Height(); ++x) {
        for (size_t y = min_y; y + dy < b1.Width(); ++y) {
            // like signed short
            b1.Get<int16_t>(x, y) -= b2.Get<int16_t>(x + dx, y + dy);
        }
    }
}

void Sub(Bitmap& b1, const Bitmap& b2) {
    SubShifted(b1, b2, 0, 0);
}

void Div2(Bitmap& b) {
    for (size_t x = 0; x < b.Height(); ++x) {
        for (size_t y = 0; y < b.Width(); ++y) {
            // like signed short
            b.Set(x, y, static_cast<uint16_t>(b.Get<uint16_t>(x, y) >> 1));
        }
    }
}

void Abs(Bitmap& b) {
    for (size_t x = 0; x < b.Height(); ++x) {
        for (size_t y = 0; y < b.Width(); ++y) {
            // like signed short
            if (x == 8 && y == 12) {
                int k = 0;
            }
            int16_t v = b.Get<int16_t>(x, y);
            b.Set(x, y, static_cast<int16_t>(v < 0 ? -v : v));
        }
    }
}
