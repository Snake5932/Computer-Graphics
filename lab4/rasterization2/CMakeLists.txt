cmake_minimum_required(VERSION 3.15)
project(rasterization2)

set(CMAKE_CXX_STANDARD 14)

add_executable(rasterization2 main.cpp)
target_link_libraries(rasterization2 -lglfw3 -lopengl32 -lgdi32)