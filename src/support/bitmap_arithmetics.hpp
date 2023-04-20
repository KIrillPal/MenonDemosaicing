#pragma once
#include "bitmap.hpp"

//#define SIMD

// Operation b1 := b1 - b2, where b1[i,j] -= b2[i+dx,j+dy]
// or b1[i,j] doesn't change if b2[i+dx,j+dy] is out of bounds
// BE CAREFUL: b1 size must be equal to b2 size
void SubShifted(Bitmap& b1, const Bitmap& b2, int dx, int dy);

// Operation b1 := b1 - b2, where b1[i,j] -= b2[i,j]
// BE CAREFUL: b1 size must be equal to b2 size
void Sub(Bitmap& b1, const Bitmap& b2);

// Operation b := b/2; unsigned
void Div2(Bitmap& b);

// Operation b := |b|
void Abs(Bitmap& b);