cmake_minimum_required (VERSION 3.10)

set(REPOSITORY https://github.com/KIrillPal/MenonDemosaicing.git)
set(SRC "src")

# search TinyTIFF Package
find_package(TinyTIFF REQUIRED)

add_subdirectory(${SRC}/io/TinyTIFF ./TinyTIFF)
add_executable (MenonDemosaicing ${SRC}/main.cpp)
target_link_libraries(MenonDemosaicing TinyTIFF)
