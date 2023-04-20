#pragma once
#include "../../support/bitmap.hpp"

namespace io {
    Bitmap ReadBitmapFromTIFF(const char *file_path);
    void WriteGreyscaleToTIFF(const Bitmap& image, const char* filename);
    void WriteRGBToTIFF(const Bitmap& R, const Bitmap& G, const Bitmap& B, const char* filename);
}