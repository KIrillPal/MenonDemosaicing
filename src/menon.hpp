#pragma once

// For debug
#include <iostream>
#include <chrono>

#include "support/bitmap_arithmetics.hpp"
#include "io/format/tiff.hpp"
#include "interpolation/directional.hpp"
#include "decision/posteriori.hpp"
#include "interpolation/rb.hpp"
#include "support/rgb.hpp"
#include "refining/lowpass.hpp"

#define TIMESTAMP { \
auto now = std::chrono::system_clock::now(); \
std::chrono::duration duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start); \
std::cout << duration.count() << " ms \n";       \
}

namespace menon {

    // Gets an RGB image from the CFA mosaic using the Menon Decfaing algorithm
    // cfa - RGGB Bayer CFA mosaic.
    // For GRBG remove define RGGB in /CMakeLists.txt row 25
    //
    rgb::BitmapRGB Demosaicing(const Bitmap& cfa) {
        auto start = std::chrono::system_clock::now();
        TIMESTAMP

        Bitmap cfa32 = std::move(CopyCast32(cfa));

        // Get low-pass values in two directions
        auto lpVH_future = lp::GetLowpassFilterVHAsync(cfa32);

        auto green_vh = menon::InterpolateGreenVH(cfa);

        std::cout << "VH are finished\n" << ' ';
        TIMESTAMP

        auto class_diff = menon::GetClassifierDifference(cfa, green_vh);

        std::cout << "Classifiers found " << ' ';
        TIMESTAMP

        auto green = menon::Posteriori(green_vh, class_diff);

        std::cout << "Green layer found " << ' ';
        TIMESTAMP

        auto rb = menon::InterpolateRBonGreen(cfa, green);
        std::cout << "RB on Green found " << ' ';
        TIMESTAMP

        lpVH_future.wait();

        menon::FillRBonRB(rb, class_diff);
        std::cout << "RB on RB found " << ' ';
        TIMESTAMP

        std::cout << "Red and blue layers found " << ' ';
        TIMESTAMP

        auto lpVH = lpVH_future.get();
        io::WriteGreyscaleToTIFF(CopyCast16(lpVH.V), "lpv.tiff");
        io::WriteGreyscaleToTIFF(CopyCast16(lpVH.H), "lph.tiff");

        return rgb::BitmapRGB{
            std::move(rb.V),
            std::move(green),
            std::move(rb.H)
        };
    }
    //
    // Example to load cfa from one-sampled tiff image:
    //      Bitmap cfa = io::ReadImage("cfa.tiff");
    //
    // To save result use io::WriteRGBToTIFF(result);
    //
    // To disable execution in several threads
    // remove define PARALLEL in /CMakeLists.txt row 19
    //
    // To disable using SSE3 and SSE4.1
    // remove define SIMD in /CMakeLists.txt row 21
}