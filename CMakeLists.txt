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

add_library(readtiff ${SRC}/io/format/tiff.cpp)
target_link_libraries(readtiff TinyTIFF)

add_executable (menon ${SRC}/main.cpp)
target_link_libraries(menon readtiff)
set_target_properties(menon PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../ )
