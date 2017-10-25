cmake_minimum_required(VERSION 3.3)
project(mbirla_mselli_rmeda_a1)
find_package(png REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
ADD_DEFINITIONS(-lpng)
set(SOURCE_FILES
    a1.cpp
    DrawText.cpp
    DrawText.h
    DTwoDimArray.h
    SImage.h
    SImageIO.h)

add_executable(mbirla_mselli_rmeda_a1 ${SOURCE_FILES})

target_link_libraries(mbirla_mselli_rmeda_a1 ${png})
