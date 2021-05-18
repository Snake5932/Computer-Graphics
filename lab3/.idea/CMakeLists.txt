cmake_minimum_required(VERSION 3.15)
project(rotating_oval_tube)

set(CMAKE_CXX_STANDARD 14)

add_executable(rotating_oval_tube main.cpp)
target_link_libraries(rotating_oval_tube -lglfw3 -lopengl32 -lgdi32)