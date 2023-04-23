#include <cmath>
#include "bitmap_arithmetics.hpp"

#if defined(SIMD)
#include "immintrin.h"
#include "emmintrin.h"
#endif

//Operation variants (only headers):
void AddShiftedSimple(Bitmap& b1, const Bitmap& b2, int dx, int dy);
void SubShiftedSimple(Bitmap& b1, const Bitmap& b2, int dx, int dy);
void AddShiftedWithSIMD(Bitmap& b1, const Bitmap& b2, int dx, int dy);
void SubShiftedWithSIMD(Bitmap& b1, const Bitmap& b2, int dx, int dy);

void AbsSimple(Bitmap& b);
void AbsWithSIMD(Bitmap& b);

void SubWithSIMD(Bitmap& b1, const Bitmap& b2);
void ShiftSimple(Bitmap& b, int offset);
void ShiftWithSIMD(Bitmap& b, int offset);
void SubDiv2Simple(Bitmap& b1, const Bitmap& b2);
void SubDiv2WithSIMD(Bitmap& b1, const Bitmap& b2);

Bitmap CopyCast32Simple(const Bitmap& b);
Bitmap CopyCast32WithSIMD(const Bitmap& b);
Bitmap CopyCast16Simple(const Bitmap& b);
//Bitmap CopyCast16WithSIMD(const Bitmap& b);


void AddShifted(Bitmap& b1, const Bitmap& b2, int dx, int dy) {
#ifdef SIMD
    AddShiftedWithSIMD(b1, b2, dx, dy);
#else
    AddShiftedSimple(b1, b2, dx, dy);
#endif
}

void SubShifted(Bitmap& b1, const Bitmap& b2, int dx, int dy) {
#ifdef SIMD
    SubShiftedWithSIMD(b1, b2, dx, dy);
#else
    SubShiftedSimple(b1, b2, dx, dy);
#endif
}

void Sub(Bitmap& b1, const Bitmap& b2) {
#if defined(SIMD)
    SubWithSIMD(b1, b2);
#else
    SubShiftedSimple(b1, b2, 0, 0);
#endif
}

void Add(Bitmap& b1, const Bitmap& b2) {
    AddShifted(b1, b2, 0, 0);
}

Bitmap CopyCast32(const Bitmap& b) {
#if defined(SIMD)
    return CopyCast32WithSIMD(b);
#else
    return CopyCast32Simple(b);
#endif
}

Bitmap CopyCast16(const Bitmap& b) {
    // No such SIMD instruction
    return CopyCast16Simple(b);
}

void Abs(Bitmap& b) {
#if defined(SIMD)
    AbsWithSIMD(b);
#else
    AbsSimple(b);
#endif
}

void Shift(Bitmap& b, int offset) {
#if defined(SIMD)
    ShiftWithSIMD(b, offset);
#else
    ShiftSimple(b, offset);
#endif
}

void SubDiv2(Bitmap& b1, const Bitmap& b2) {
#if defined(SIMD)
    SubDiv2WithSIMD(b1, b2);
#else
    SubDiv2Simple(b1, b2);
#endif
}

////////////////////////////////////////////////////////////////////////////////////
// Operation patterns:

#define SIMPLE_SHIFTED_OPERATION(PREPARE, IN_CYCLE)                                \
    {                                                                              \
        size_t min_x = std::max(0, -dx);                                           \
        size_t min_y = std::max(0, -dy);                                           \
                                                                                   \
        PREPARE                                                                    \
                                                                                   \
        for (size_t x = min_x; x + dx < b1.Height() && x < b1.Height(); ++x) {     \
            for (size_t y = min_y; y + dy < b1.Width() && y < b1.Width(); ++y) {   \
                IN_CYCLE                                                           \
            }                                                                      \
        }                                                                          \
    }                                                                              \

#define SIMD_OPERATION(PREPARE, IN_CYCLE, IN_REST)                                 \
    {                                                                              \
        PREPARE                                                                    \
        /* Position of the x-th row in data*/                                      \
        /*Initially zero*/                                                         \
        size_t row_pos = 0;                                                        \
        /*For each row grab elems by 128b and do some with them*/                  \
        for (size_t x = 0; x < h; ++x) {                                           \
            size_t y = 0;                                                          \
            for (; y + SIMD_SIZE_ITEMS <= w; y += SIMD_SIZE_ITEMS) {               \
                IN_CYCLE                                                           \
            }                                                                      \
            /* Deal with the rest */                                               \
            while (y < w) {                                                        \
                IN_REST                                                            \
                ++y;                                                               \
            }                                                                      \
            /* Update row_pos */                                                   \
            row_pos += w;                                                          \
        }                                                                          \
    }

#define SIMD_SHIFTED_OPERATION(PREPARE, IN_CYCLE, IN_REST)                         \
    {                                                                              \
        size_t h = b1.Height();                                                    \
        size_t w = b1.Width();                                                     \
        size_t min_x = std::max(0, -dx);                                           \
        size_t min_y = std::max(0, -dy);                                           \
                                                                                   \
        PREPARE                                                                    \
                                                                                   \
        size_t row1_pos = min_x * w;                                               \
        size_t row2_pos = (min_x + dx) * w;                                        \
        /*Just shorter constant*/                                                  \
        constexpr size_t STEP = SIMD_SIZE_ITEMS;                                   \
        /*For each row grab elems by 128b and do some with them*/                  \
        for (size_t x = min_x; x < h && x + dx < h; ++x) {                         \
            size_t y = min_y;                                                      \
            for (; y + STEP <= w && y + dy + STEP <= w; y += STEP) {               \
                IN_CYCLE                                                           \
            }                                                                      \
            /* Deal with the rest */                                               \
            while (y < w && y + dy < w) {                                          \
                IN_REST                                                            \
                ++y;                                                               \
            }                                                                      \
            /* Update row_pos */                                                   \
            row1_pos += w;                                                         \
            row2_pos += w;                                                         \
        }                                                                          \
    }

#define FOR_EVERY_PIXEL(bmp, IN_CYCLE)                 \
    {                                                  \
        for (size_t x = 0; x < bmp.Height(); ++x) {    \
            for (size_t y = 0; y < bmp.Width(); ++y) { \
                IN_CYCLE                               \
            }                                          \
        }                                              \
    }

    void Div2(Bitmap& b) {
        FOR_EVERY_PIXEL(b, {
            // like signed short
            b.Set(x, y, static_cast<int16_t>(b.Get<int16_t>(x, y) / 2));
        })
    }

    void AbsSimple(Bitmap& b) {
        switch(b.BytesPerPixel()) {
            case sizeof(uint16_t):
                FOR_EVERY_PIXEL(b, {
                    /* like signed short */
                    int16_t v = b.Get<int16_t>(x, y);
                    b.Set(x, y, static_cast<int16_t>(v < 0 ? -v : v));
                }) break;
            case sizeof(int):
                FOR_EVERY_PIXEL(b, {
                    int v = b.Get<int>(x, y);
                    b.Set(x, y, static_cast<int>(v < 0 ? -v : v));
                }) break;
        }
    }

////////////////////////////////////////////////////////////////////////////////////
// Simple implementations:

void AddShiftedSimple(Bitmap& b1, const Bitmap& b2, int dx, int dy) {
    switch(b1.BytesPerPixel()) {
        case sizeof(uint16_t):
        SIMPLE_SHIFTED_OPERATION({}, {
            // like signed short
            b1.Get<int16_t>(x, y) += b2.Get<int16_t>(x + dx, y + dy);
        }) break;
        case sizeof(int):
        SIMPLE_SHIFTED_OPERATION({}, {
            b1.Get<int>(x, y) += b2.Get<int>(x + dx, y + dy);
        }) break;
    }
}

void SubShiftedSimple(Bitmap& b1, const Bitmap& b2, int dx, int dy) {
    switch(b1.BytesPerPixel()) {
        case sizeof(uint16_t):
        SIMPLE_SHIFTED_OPERATION({}, {
            // like signed short
            b1.Get<int16_t>(x, y) -= b2.Get<int16_t>(x + dx, y + dy);
        }) break;
        case sizeof(int):
        SIMPLE_SHIFTED_OPERATION({}, {
            b1.Get<int>(x, y) -= b2.Get<int>(x + dx, y + dy);
        }) break;
    }
}

Bitmap CopyCast32Simple(const Bitmap& b) {
    size_t h = b.Height();
    size_t w = b.Width();
    Bitmap cp(h, w, sizeof(int));
    FOR_EVERY_PIXEL(b, {
        cp.Set(x, y, static_cast<int>(b.Get<uint16_t>(x, y)));
    })
    return cp;
}

Bitmap CopyCast16Simple(const Bitmap& b) {
    size_t h = b.Height();
    size_t w = b.Width();
    Bitmap cp(h, w, sizeof(uint16_t));
    FOR_EVERY_PIXEL(b, {
        int v = std::min(std::max(b.Get<int>(x, y), 0), UINT16_MAX);
        cp.Set(x, y, static_cast<uint16_t>(v));
    })
    return cp;
}

void FillWithZeros(Bitmap& b) {
    std::memset(b.Data(), 0, b.Width()*b.Height()*b.BytesPerPixel());
}

void ShiftSimple(Bitmap& b, int offset) {
    FOR_EVERY_PIXEL(b, {
        // like unsigned short
        b.Set(x, y, static_cast<uint16_t>(b.Get<uint16_t>(x, y) >> offset));
    })
}

void SubDiv2Simple(Bitmap& b1, const Bitmap& b2) {
    FOR_EVERY_PIXEL(b1, {
        uint16_t value1 = b1.Get<uint16_t>(x, y) >> 1;
        uint16_t value2 = b2.Get<uint16_t>(x, y) >> 1;
        b1.Set(x, y, static_cast<int16_t>(value1 - value2));
    })
}


////////////////////////////////////////////////////////////////////////////////////
// SIMD implementations:
#if defined(SIMD)

void SubWithSIMD(Bitmap& b1, const Bitmap& b2) {
    constexpr size_t SIMD_SIZE_BITS = 128;
    size_t h = b1.Height();
    size_t w = b1.Width();
    switch(b1.BytesPerPixel()) {
        case sizeof(int16_t): {
            constexpr size_t SIMD_SIZE_ITEMS = (SIMD_SIZE_BITS >> 3) / sizeof(int16_t);
            SIMD_OPERATION(
                    auto b1_data = reinterpret_cast<int16_t *>(b1.Data());
                    auto b2_data = reinterpret_cast<const int16_t *>(b2.Data());       ,
                    __m128i row1 = _mm_loadu_si128((__m128i *) (&b1_data[row_pos + y]));
                    __m128i row2 = _mm_loadu_si128((__m128i *) (&b2_data[row_pos + y]));
                    __m128i sub = _mm_sub_epi16(row1, row2);
                    _mm_storeu_si128((__m128i *) (&b1_data[row_pos + y]), sub);        ,
                    b1_data[row_pos + y] -= b2_data[row_pos + y];
            ) break;
        }
        case sizeof(int): {
            constexpr size_t SIMD_SIZE_ITEMS = (SIMD_SIZE_BITS >> 3) / sizeof(int);
            SIMD_OPERATION(
                    auto b1_data = reinterpret_cast<int *>(b1.Data());
                    auto b2_data = reinterpret_cast<const int *>(b2.Data());           ,
                    __m128i row1 = _mm_loadu_si128((__m128i *) (&b1_data[row_pos + y]));
                    __m128i row2 = _mm_loadu_si128((__m128i *) (&b2_data[row_pos + y]));
                    __m128i sub = _mm_sub_epi32(row1, row2);
                    _mm_storeu_si128((__m128i *) (&b1_data[row_pos + y]), sub);        ,
                    b1_data[row_pos + y] -= b2_data[row_pos + y];
            ) break;
        }
    }
}

void AddShiftedWithSIMD(Bitmap& b1, const Bitmap& b2, int dx, int dy)
{
    constexpr size_t SIMD_SIZE_BITS = 128;
    switch(b1.BytesPerPixel()) {
        case sizeof(int16_t): {
            constexpr size_t SIMD_SIZE_ITEMS = (SIMD_SIZE_BITS >> 3) / sizeof(int16_t);
            SIMD_SHIFTED_OPERATION(
                    auto b1_data = reinterpret_cast<int16_t *>(b1.Data());
                    auto b2_data = reinterpret_cast<const int16_t *>(b2.Data());
                    ,
                    __m128i row1 = _mm_loadu_si128((__m128i *) (&b1_data[row1_pos + y]));
                    __m128i row2 = _mm_loadu_si128((__m128i *) (&b2_data[row2_pos + y + dy]));
                    __m128i sum = _mm_add_epi16(row1, row2);
                    _mm_storeu_si128((__m128i *) (&b1_data[row1_pos + y]), sum);
                    ,
                    b1_data[row1_pos + y] += b2_data[row2_pos + y + dy];
            ) break;
        }
        case sizeof(int): {
            constexpr size_t SIMD_SIZE_ITEMS = (SIMD_SIZE_BITS >> 3) / sizeof(int);
            SIMD_SHIFTED_OPERATION(
                    auto b1_data = reinterpret_cast<int *>(b1.Data());
                    auto b2_data = reinterpret_cast<const int *>(b2.Data());
                    ,
                    __m128i row1 = _mm_loadu_si128((__m128i *) (&b1_data[row1_pos + y]));
                    __m128i row2 = _mm_loadu_si128((__m128i *) (&b2_data[row2_pos + y + dy]));
                    __m128i sum = _mm_add_epi32(row1, row2);
                    _mm_storeu_si128((__m128i *) (&b1_data[row1_pos + y]), sum);
                    ,
                    b1_data[row1_pos + y] += b2_data[row2_pos + y + dy];
            ) break;
        }
    }
}

void SubShiftedWithSIMD(Bitmap& b1, const Bitmap& b2, int dx, int dy)
{
    constexpr size_t SIMD_SIZE_BITS = 128;
    switch(b1.BytesPerPixel()) {
        case sizeof(int16_t): {
            constexpr size_t SIMD_SIZE_ITEMS = (SIMD_SIZE_BITS >> 3) / sizeof(int16_t);
            SIMD_SHIFTED_OPERATION(
                    auto b1_data = reinterpret_cast<int16_t *>(b1.Data());
                    auto b2_data = reinterpret_cast<const int16_t *>(b2.Data());
            ,
                    __m128i row1 = _mm_loadu_si128((__m128i *) (&b1_data[row1_pos + y]));
                    __m128i row2 = _mm_loadu_si128((__m128i *) (&b2_data[row2_pos + y + dy]));
                    __m128i sub = _mm_sub_epi16(row1, row2);
                    _mm_storeu_si128((__m128i *) (&b1_data[row1_pos + y]), sub);
            ,
                    b1_data[row1_pos + y] += b2_data[row2_pos + y + dy];
            ) break;
        }
        case sizeof(int): {
            constexpr size_t SIMD_SIZE_ITEMS = (SIMD_SIZE_BITS >> 3) / sizeof(int);
            SIMD_SHIFTED_OPERATION(
                    auto b1_data = reinterpret_cast<int *>(b1.Data());
                    auto b2_data = reinterpret_cast<const int *>(b2.Data());
            ,
                    __m128i row1 = _mm_loadu_si128((__m128i *) (&b1_data[row1_pos + y]));
                    __m128i row2 = _mm_loadu_si128((__m128i *) (&b2_data[row2_pos + y + dy]));
                    __m128i sub = _mm_sub_epi32(row1, row2);
                    _mm_storeu_si128((__m128i *) (&b1_data[row1_pos + y]), sub);
            ,
                    b1_data[row1_pos + y] += b2_data[row2_pos + y + dy];
            )
        } break;
    }
}

Bitmap CopyCast32WithSIMD(const Bitmap& b) {
    constexpr size_t SIMD_SIZE_BITS = 128;
    constexpr size_t SIMD_SIZE_ITEMS = (SIMD_SIZE_BITS >> 3) / sizeof(int);
    size_t h = b.Height();
    size_t w = b.Width();
    Bitmap cp(h, w, sizeof(int));

    {
        auto b_data = reinterpret_cast<const uint16_t*>(b.Data());
        auto cp_data = reinterpret_cast<int*>(cp.Data());
        /* Position of the x-th row in data*/
        /*Initially zero*/
        size_t row_pos = 0;
        /*For each row grab elems by 128b and do some with them*/
        for (size_t x = 0; x < h; ++x) {
            size_t y = 0;
            for (; y + SIMD_SIZE_ITEMS <= w; y += SIMD_SIZE_ITEMS) {
                // We need only 64b of elems, but we read 128.
                // They exist because of DATA_SAFE_OFFSET in bitmap.hpp
                __m128i row = _mm_loadu_si128((__m128i*)(&b_data[row_pos + y]));
                __m128i ints = _mm_cvtepu16_epi32(row);
                _mm_storeu_si128((__m128i*)(&cp_data[row_pos + y]), ints);
            }
            /* Deal with the rest */
            while (y < w) {
                cp_data[row_pos + y] = static_cast<int>(b_data[row_pos + y]);
                ++y;
            }
            /* Update row_pos */
            row_pos += w;
        }
    }
    return cp;
}

void AbsWithSIMD(Bitmap& b) {
    constexpr size_t SIMD_SIZE_BITS = 128;
    constexpr size_t SIMD_SIZE_ITEMS = (SIMD_SIZE_BITS >> 3) / sizeof(int16_t);
    size_t h = b.Height();
    size_t w = b.Width();
    SIMD_OPERATION(
            auto data = reinterpret_cast<int16_t *>(b.Data());
            ,
            __m128i row = _mm_loadu_si128((__m128i *) (&data[row_pos + y]));
            __m128i abs = _mm_abs_epi16(row);
            _mm_storeu_si128((__m128i *) (&data[row_pos + y]), abs);
            ,
            if (data[row_pos + y] < 0) {
                data[row_pos + y] = -data[row_pos + y];
            }
    )
}

void ShiftWithSIMD(Bitmap& b, int offset) {
    constexpr size_t SIMD_SIZE_BITS = 128;
    constexpr size_t SIMD_SIZE_ITEMS = (SIMD_SIZE_BITS >> 3) / sizeof(int16_t);
    size_t h = b.Height();
    size_t w = b.Width();
    SIMD_OPERATION(
            auto data = reinterpret_cast<int16_t *>(b.Data());
    ,
            __m128i row = _mm_loadu_si128((__m128i *) (&data[row_pos + y]));
            __m128i abs = _mm_srli_epi16(row, offset);
            _mm_storeu_si128((__m128i *) (&data[row_pos + y]), abs);
    ,
            data[row_pos + y] >>= offset;
    )
}

void SubDiv2WithSIMD(Bitmap& b1, const Bitmap& b2) {
    constexpr size_t SIMD_SIZE_BITS = 128;
    size_t h = b1.Height();
    size_t w = b1.Width();

    constexpr size_t SIMD_SIZE_ITEMS = (SIMD_SIZE_BITS >> 3) / sizeof(uint16_t);
    SIMD_OPERATION(
            auto b1_data = reinterpret_cast<uint16_t *>(b1.Data());
            auto b2_data = reinterpret_cast<const uint16_t *>(b2.Data());
            ,
            __m128i row1 = _mm_loadu_si128((__m128i *) (&b1_data[row_pos + y]));
            __m128i row2 = _mm_loadu_si128((__m128i *) (&b2_data[row_pos + y]));
            __m128i row1_2 = _mm_srli_epi16(row1, 1);
            __m128i row2_2 = _mm_srli_epi16(row2, 1);
            __m128i sub_2 = _mm_sub_epi16(row1_2, row2_2);
            _mm_storeu_si128((__m128i *) (&b1_data[row_pos + y]), sub_2);
            ,
            b1_data[row_pos + y] -= b2_data[row_pos + y];
            b1_data[row_pos + y] >>= 1;
    )
}

#endif