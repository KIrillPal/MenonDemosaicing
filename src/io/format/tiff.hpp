#pragma once
#include "../../support/bitmap.hpp"
#include "../../support/rgb.hpp"

namespace io {
    // Reads one-sampled TIFF format image.
    // Works only with one-sampled images
    // Returns bitmap - not packed pixel data
    //
    // Exception on failure
    Bitmap ReadBitmapFromTIFF(const char *file_path);

    // Saves one-sampled TIFF format image to './filename'.
    // Except on failure
    void WriteGreyscaleToTIFF(const Bitmap& image, const char* filename);

    // Saves rgb TIFF format image to './filename'.
    // Except on failure
    void WriteRGBToTIFF(const Bitmap& R, const Bitmap& G, const Bitmap& B, const char* filename);

    // Saves rgb TIFF format image to './filename'.
    // Except on failure
    void WriteRGBToTIFF(const rgb::BitmapRGB& image, const char* filename);
}