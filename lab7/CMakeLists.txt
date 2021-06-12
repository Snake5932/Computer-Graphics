cmake_minimum_required(VERSION 3.15)
project(optimization)

set(CMAKE_CXX_STANDARD 14)

add_executable(optimization main.cpp stb_image.cpp json.h stb_image.h)
target_link_libraries(optimization -lglu32 -lglfw3 -lopengl32 -lgdi32)