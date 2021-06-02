cmake_minimum_required(VERSION 3.15)
project(testClipping2)

set(CMAKE_CXX_STANDARD 14)

add_executable(testClipping2 main.cpp)
target_link_libraries(testClipping2 -lglfw3 -lopengl32 -lgdi32)