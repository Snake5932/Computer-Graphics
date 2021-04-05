cmake_minimum_required(VERSION 3.15)
project(rotating_cube)

set(CMAKE_CXX_STANDARD 14)

add_executable(rotating_cube main.cpp)
target_link_libraries(rotating_cube -lglfw3 -lopengl32 -lgdi32)