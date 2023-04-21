#include <iostream>
#include "menon.hpp"

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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintHelpUsage();
        return 0;
    }
    auto bayer = ReadImage(argv[1]);
    std::cout << "Image size: " << bayer.Width() << " x " << bayer.Height() << '\n';
    std::cout << "Bytes per pixel: " << bayer.BytesPerPixel() << '\n';

    auto image = menon::Demosaicing(bayer);

    io::WriteRGBToTIFF(image, "result.tiff");
    std::cout << "Writing finished\n" << '\n';
    return 0;
}