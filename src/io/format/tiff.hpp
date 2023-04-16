#pragma once
#include "../../support/bitmap.hpp"

namespace io {
    Bitmap ReadBitmapFromTIFF(const char *file_path);
}