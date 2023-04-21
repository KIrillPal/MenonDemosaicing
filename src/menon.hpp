#pragma once

// For debug
#include <iostream>

#include "io/format/tiff.hpp"
#include "interpolation/directional.hpp"
#include "decision/posteriori.hpp"
#include "interpolation/rb.hpp"
#include "support/rgb.hpp"

namespace menon {

    // Gets an RGB image from the mosaic using the Menon Demosaicing algorithm
    // mosaic - RGGB Bayer mosaic.
    // For GRBG remove define RGGB in /CMakeLists.txt row 25
    //
    rgb::BitmapRGB Demosaicing(const Bitmap& mosaic) {
        auto green_vh = menon::InterpolateVHInParallel(mosaic);

        std::cout << "VH are finished\n" << '\n';

        auto class_diff = menon::GetClassifierDifference(mosaic, green_vh);

        std::cout << "Classifiers found\n" << '\n';
        //WriteGreyscaleImage(green_vh.V, "sample_v.tiff");
        //WriteGreyscaleImage(green_vh.H, "sample_h.tiff");

        auto green = menon::Posteriori(green_vh, class_diff);

        std::cout << "Green layer found\n" << '\n';

        auto rb = menon::InterpolateRB(mosaic, green, class_diff);
        std::cout << "Red and blue layers found\n" << '\n';

        return rgb::BitmapRGB{
            std::move(rb.V),
            std::move(green),
            std::move(rb.H)
        };
    }
    // Example to load mosaic from one-sampled tiff image:
    //      Bitmap mosaic = io::ReadImage("mosaic.tiff");
    //
    // To save result use io::WriteRGBToTIFF(result);
    //
    // To disable execution in several threads
    // remove define PARALLEL in /CMakeLists.txt row 19
    //
    // To disable using SSE3 and SSE4.1
    // remove define SIMD in /CMakeLists.txt row 21
}