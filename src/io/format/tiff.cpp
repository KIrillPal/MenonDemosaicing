#include "../../support/rgb.hpp"
#include <tinytiffreader.h>
#include <tinytiffwriter.h>
#include <stdexcept>
#include <cassert>
#include "tiff.hpp"

namespace io {

    Bitmap ReadBitmapFromTIFF(const char *file_path) {
        TinyTIFFReaderFile* tiffr = TinyTIFFReader_open(file_path);
        if (!tiffr) {
            throw std::runtime_error("File not existent or not accessible");
        }

        const uint32_t width = TinyTIFFReader_getWidth(tiffr);
        const uint32_t height = TinyTIFFReader_getHeight(tiffr);
        const uint16_t bits_per_sample = TinyTIFFReader_getBitsPerSample(tiffr, 0);

        // Assert on appropriate size of sample
        assert((bits_per_sample & 7) == 0);

        Bitmap bmp(height, width, bits_per_sample >> 3);
        TinyTIFFReader_getSampleData(tiffr, bmp.Data(), 0);

        auto eptr = TinyTIFFReader_getLastError(tiffr);
        if (eptr != nullptr && strcmp(eptr, "") != 0) {
            throw std::runtime_error(eptr);
        }

        TinyTIFFReader_close(tiffr);
        return bmp;
    }

    void WriteGreyscaleToTIFF(const Bitmap& image, const char* filename) {
        TinyTIFFWriterFile* tiffw = TinyTIFFWriter_open(
                filename,
                image.BytesPerPixel() * 8,
                TinyTIFFWriter_UInt,
                0,
                image.Width(),
                image.Height(),
                TinyTIFFWriter_Greyscale
        );
        if (!tiffw) {
            throw std::runtime_error("File already exists or need more rules");
        }
        TinyTIFFWriter_writeImage(tiffw, image.Data());
        auto eptr = TinyTIFFWriter_getLastError(tiffw);
        if (eptr != nullptr && strcmp(eptr, "") != 0) {
            throw std::runtime_error(eptr);
        }
    }

    void WriteRGBToTIFF(const Bitmap& R, const Bitmap& G, const Bitmap& B, const char* filename) {
        std::vector<uint16_t> image = rgb::PackRGB(R, G, B);

        TinyTIFFWriterFile* tiffw = TinyTIFFWriter_open(
                filename,
                // 3 channels of BPP bytes * (bits in byte)
                R.BytesPerPixel() * 8,
                TinyTIFFWriter_UInt,
                3,
                R.Width(),
                R.Height(),
                TinyTIFFWriter_RGB
        );
        if (!tiffw) {
            throw std::runtime_error("File already exists or need more rules");
        }
        TinyTIFFWriter_writeImage(tiffw, image.data());
        auto eptr = TinyTIFFWriter_getLastError(tiffw);
        if (eptr != nullptr && strcmp(eptr, "") != 0) {
            throw std::runtime_error(eptr);
        }
    }

} // namespace io