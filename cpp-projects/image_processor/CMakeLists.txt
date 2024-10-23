cmake_minimum_required(VERSION 3.17)
project(image_processor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${MAKE_CXX_FLAGS} -Wall -Wextra -pedantic -std=c++20")


add_executable(image_processor
    filters/blur.cpp
    filters/convolution.cpp
    filters/crop.cpp
    filters/glass.cpp
    filters/edge_detection.cpp
    filters/greyscale.cpp
    filters/negative.cpp
    filters/point_filter.cpp
    filters/sharpening.cpp

    image/Image.cpp
    image/RGB.cpp
    image/matrix.cpp

    io/args.cpp
    io/open_save.cpp

    pipeline/image_processor.cpp
    pipeline/pipeline.cpp
)

target_include_directories(image_processor PUBLIC include)
