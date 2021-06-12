cmake_minimum_required(VERSION 3.15)
project(shaders)

set(CMAKE_CXX_STANDARD 14)

add_executable(shaders main.cpp glad.c)
target_link_libraries(shaders -lglu32 -lglfw3 -lopengl32 -lgdi32)