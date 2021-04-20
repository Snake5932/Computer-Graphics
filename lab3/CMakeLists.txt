cmake_minimum_required(VERSION 3.15)
project(rasterization)

set(CMAKE_CXX_STANDARD 14)

add_executable(rasterization main.cpp)
target_link_libraries(rasterization -lglfw3 -lopengl32 -lgdi32)