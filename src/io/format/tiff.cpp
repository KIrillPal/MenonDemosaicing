#include <tinytiffreader.h>
#include <stdexcept>
#include <cassert>
#include "tiff.hpp"

namespace io {

    SharedBitmap ReadBitmapFromTIFF(const char *file_path) {
        TinyTIFFReaderFile* tiffr = TinyTIFFReader_open(file_path);
        if (!tiffr) {
            throw std::runtime_error("File not existent or not accessible");
        }

        const uint32_t width = TinyTIFFReader_getWidth(tiffr);
        const uint32_t height = TinyTIFFReader_getHeight(tiffr);
        const uint16_t bits_per_sample = TinyTIFFReader_getBitsPerSample(tiffr, 0);

        // Assert on appropriate size of sample
        assert((bits_per_sample & 7) == 0);

        SharedBitmap bmp(height, width, bits_per_sample >> 3);
        TinyTIFFReader_getSampleData(tiffr, bmp.Data(), 0);

        auto eptr = TinyTIFFReader_getLastError(tiffr);
        if (eptr != nullptr && strcmp(eptr, "") != 0) {
            throw std::runtime_error(eptr);
        }

        TinyTIFFReader_close(tiffr);
        return bmp;
    }

} // namespace io