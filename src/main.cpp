#include <iostream>
#include "menon.hpp"

void Abort(int code = 0) {
    std::cout << "ABORTING\n";
    exit(code);
}

void PrintHelpUsage() {
    std::cout << "Usage: menon <file.tiff>\n";
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

void Make16Bit(Bitmap& cfa) {
    Bitmap cfa16(cfa.Height(), cfa.Width(), sizeof(uint16_t));
    for (size_t x = 0; x < cfa.Height(); ++x) {
        for (size_t y = 0; y < cfa.Width(); ++y) {
            cfa16.Set(x, y, static_cast<uint16_t>(cfa.Get<uint8_t>(x, y)) << 8);
        }
    }
    cfa = std::move(cfa16);
}

//#define TEST
#define NTESTS 100

int main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintHelpUsage();
        return 0;
    }
    auto bayer = ReadImage(argv[1]);
    std::cout << "Image size: " << bayer.Width() << " x " << bayer.Height() << '\n';
    std::cout << "Bytes per pixel: " << bayer.BytesPerPixel() << '\n';
    if (bayer.BytesPerPixel() != sizeof(uint16_t)) {
        Make16Bit(bayer);
    }

    rgb::BitmapRGB image;
#if defined(TEST)
    auto start = std::chrono::system_clock::now();

    for (size_t i = 0; i < NTESTS; ++i) {
        std::cout << "Test " << i << ":\n";
        auto start = std::chrono::system_clock::now();
        image = menon::Demosaicing(bayer);
        std::cout << "Total test time: ";
        TIMESTAMP
    }
    std::cout << "Total time: ";
    TIMESTAMP
#else
    image = menon::Demosaicing(bayer);
#endif

    io::WriteRGBToTIFF(image, "result.tiff");
    std::cout << "Writing finished\n";
    return 0;
}
