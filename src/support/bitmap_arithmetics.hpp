#pragma once
#include "bitmap.hpp"

//#define SIMD

// Operation b1 := b1 + b2, where b1[i,j] += b2[i+dx,j+dy]
// or b1[i,j] doesn't change if b2[i+dx,j+dy] is out of bounds
// BE CAREFUL: signed
// BE CAREFUL: b1 size must be equal to b2 size
void AddShifted(Bitmap& b1, const Bitmap& b2, int dx, int dy);

// Operation b1 := b1 - b2, where b1[i,j] -= b2[i+dx,j+dy]
// or b1[i,j] doesn't change if b2[i+dx,j+dy] is out of bounds
// BE CAREFUL: signed
// BE CAREFUL: b1 size must be equal to b2 size
void SubShifted(Bitmap& b1, const Bitmap& b2, int dx, int dy);

// Operation b1 := b1 - b2, where b1[i,j] -= b2[i,j]
// BE CAREFUL: b1 size must be equal to b2 size
void Sub(Bitmap& b1, const Bitmap& b2);

// Operation b1 := b1 + b2, where b1[i,j] += b2[i,j]
// BE CAREFUL: b1 size must be equal to b2 size
void Add(Bitmap& b1, const Bitmap& b2);

// Operation b := b >> offset; unsigned
void Shift(Bitmap& b, int offset);

// Operation b := b / 2; signed
void Div2(Bitmap& b);

// Operation b1 := (b1 - b2) / 2; signed
// where b1[i,j] = (b1[i,j] - b2[i,j]) / 2
// BE CAREFUL: b1 size must be equal to b2 size
void SubDiv2(Bitmap& b1, const Bitmap& b2);

// Operation b := |b|
void Abs(Bitmap& b);

// Copies b with casting its values to int from uint16_t
// Returns Bitmap<int>
Bitmap CopyCast32(const Bitmap& b);

// Copies b with casting its values to uint16_t from int
// Returns Bitmap<uint16_t>
Bitmap CopyCast16(const Bitmap& b);

// Fills bitmap b with zeros
void FillWithZeros(Bitmap& b);