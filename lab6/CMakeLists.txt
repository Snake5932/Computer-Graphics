cmake_minimum_required(VERSION 3.15)
project(realisticTube)

set(CMAKE_CXX_STANDARD 14)

add_executable(realisticTube main.cpp stb_image.cpp stb_image.h json.h)
target_link_libraries(realisticTube -lglew32s -lglu32 -lglfw3 -lopengl32 -lgdi32)