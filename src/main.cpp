#include <iostream>
#include "io/TinyTIFF/src/tinytiffreader.h"
#include "io/TinyTIFF/src/tinytiffreader.hxx"

int main() {
	TinyTIFFReaderFile* tiffr = NULL;
	tiffr = TinyTIFFReader_open("sample.tiff");
	if (!tiffr) {
		std::cout << "    ERROR reading (not existent, not accessible or no TIFF file)\n";
	}
	else {
		const uint32_t width = TinyTIFFReader_getWidth(tiffr);
		const uint32_t height = TinyTIFFReader_getHeight(tiffr);
		const uint16_t bitspersample = TinyTIFFReader_getBitsPerSample(tiffr, 0);
		uint8_t* image = (uint8_t*)calloc(width * height, bitspersample / 8);
		TinyTIFFReader_getSampleData(tiffr, image, 0);

		printf("BitsPerSample: %hu\n", bitspersample);
        for (size_t x = 0; x < height; ++x) {
            for (size_t y = 0; y < width; ++y) {
                printf("%hhx ", image[x * width + y]);
            }
            printf("\n");
        }

		free(image);
	}
	TinyTIFFReader_close(tiffr);
}