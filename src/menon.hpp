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
#include "refining/refine.hpp"

#define TIMESTAMP { \
auto now = std::chrono::system_clock::now(); \
std::chrono::duration duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start); \
start = now;                                     \
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

#if defined(REFINE)
        Bitmap cfa32 = std::move(CopyCast32(cfa));
        // Get low-pass values in two directions
        auto lpVH_future = lp::GetLowpassFilterVHAsync(cfa32);
#endif

        auto green_vh = menon::InterpolateGreenVH(cfa);

        std::cout << "VH are finished\n" << ' ';
        TIMESTAMP

        auto class_diff = menon::GetClassifierDifference(cfa, green_vh);

        std::cout << "Classifiers found " << ' ';
        TIMESTAMP

        auto green = menon::Posteriori(green_vh, class_diff);

        std::cout << "Green layer found " << ' ';
        TIMESTAMP

#if defined(REFINE)
        auto lpVH = lpVH_future.get();
        auto hpG_future = lp::GetHighpassFilterGAsync(lpVH, green, class_diff);
#endif
        auto rb = menon::InterpolateRBonGreen(cfa, green);
        std::cout << "RB on Green found " << ' ';
        TIMESTAMP

#if defined(REFINE)
        auto hpG = hpG_future.get();
        auto hpRR_future = lp::GetHighpassFilterRonRAsync(rb, class_diff);
#endif
        menon::FillRBonRB(rb, class_diff);
        std::cout << "RB on RB found " << ' ';
        TIMESTAMP
#if defined(REFINE)
       auto hpRR = hpRR_future.get();
#endif
        std::cout << "Red and blue layers found " << ' ';
        TIMESTAMP

#if defined(REFINE)
        // Useless refining
        //refine::RefineRBonG(rb, lpVH, hpG);
        refine::RefineGonRB(green, hpG, hpRR);
        refine::RefineRBonRB(rb, hpRR, class_diff);
        std::cout << "Refining finished " << ' ';
        TIMESTAMP
#endif

        //io::WriteGreyscaleToTIFF(green, "green.tiff");
        //io::WriteGreyscaleToTIFF(green_vh.V, "green_v.tiff");
        //io::WriteGreyscaleToTIFF(green_vh.H, "green_h.tiff");
        //io::WriteGreyscaleToTIFF(rb.H, "blue.tiff");
        //io::WriteGreyscaleToTIFF(CopyCast16(hpG), "hpg.tiff");

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