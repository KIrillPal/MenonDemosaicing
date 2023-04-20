#include <iostream>
#include "io/format/tiff.hpp"
#include "interpolation/directional.hpp"
#include "decision/posteriori.hpp"

void Abort(int code = 0) {
    std::cout << "ABORTING\n";
    exit(code);
}

void PrintHelpUsage() {
    std::cout << "Usage: menon <file .tiff>\n\n";
}

 Bitmap ReadImage(const char* file_path) {
    try {
        return io::ReadBitmapFromTIFF(file_path);
    }
    catch (const std::exception& e) {
        std::cout << "Reading failed: " << e.what() << '\n';
        Abort();
    }
    return Bitmap{};
}

void WriteGreyscaleImage(const Bitmap& image, const char* file_path) {
    try {
        io::WriteGreyscaleToTIFF(image, file_path);
    }
    catch (const std::exception& e) {
        std::cout << "Writing failed: " << e.what() << '\n';
        Abort();
    }
}

void GetPixelQuery(const Bitmap& bmp) {
    size_t x = 0, y = 0;
    std::cin >> y >> x;

    uint64_t abs_level = bmp.Get(x, y);
    uint64_t abs_white = 0xFFFF;
    uint64_t abs_scale = 0xFF;

    float level = static_cast<float>(abs_level) / (abs_white / abs_scale);
    std::cout << "Light level " << level << '\n';
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintHelpUsage();
        return 0;
    }
    auto image = ReadImage(argv[1]);
    std::cout << "Image size: " << image.Width() << " x " << image.Height() << '\n';
    std::cout << "Bytes per pixel: " << image.BytesPerPixel() << '\n';


    auto green_vh = menon::InterpolateVHInParallel(image);

    std::cout << "VH are finished\n" << '\n';

    auto class_diff = menon::GetClassifierDifference(image, green_vh);

    WriteGreyscaleImage(green_vh.V, "sample_v.tiff");
    WriteGreyscaleImage(green_vh.H, "sample_h.tiff");

    std::cout << "Classifiers are found\n" << '\n';

    auto green = menon::Posteriori(green_vh, class_diff);

    WriteGreyscaleImage(green, "green.tiff");

    std::cout << "Writing completed\n" << '\n';
    return 0;
}