#pragma once
#include "../support/bitmap.hpp"

namespace io {
    SharedBitmap ReadBitmapFromTIFF(const char *file_path);
}