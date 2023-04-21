cmake_minimum_required (VERSION 3.10)
project(MenonDemosaicing)

set(REPOSITORY https://github.com/KIrillPal/MenonDemosaicing.git)
set(SRC "src")

###############################################################
# include external packages

# search TinyTIFF Package
find_package(TinyTIFF REQUIRED)
add_subdirectory(${SRC}/io/TinyTIFF ./TinyTIFF)


###############################################################
# config

# Compute concurrently (use several threads)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPARALLEL")
# Use SIMD
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSIMD")
# Disable asserts
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")
# Set RGGB mosaic format
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DRGGB")
# Set maximum optimization
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

###############################################################

add_library(rgb_utils ${SRC}/support/rgb.cpp)

add_library(readtiff ${SRC}/io/format/tiff.cpp)
target_link_libraries(readtiff TinyTIFF rgb_utils)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
add_library(interpolate ${SRC}/interpolation/directional.cpp)


add_library(arithmetics ${SRC}/support/bitmap_arithmetics.cpp)

add_library(posteriori ${SRC}/decision/posteriori.cpp)
target_link_libraries(posteriori arithmetics)

add_library(rb ${SRC}/interpolation/rb.cpp)

add_executable (menon ${SRC}/main.cpp)
target_link_libraries(menon readtiff interpolate posteriori rb)
set_target_properties(menon PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../ )