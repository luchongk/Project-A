cmake_minimum_required (VERSION 3.0.0)
cmake_policy(VERSION 3.0.0)
project(Gain VERSION 0.1)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_library(glad src/glad.c)

add_definitions(-DUNICODE -D_UNICODE)

add_executable(Gain WIN32 src/test.cpp)

target_link_libraries(Gain glad opengl32)

include_directories(${CMAKE_SOURCE_DIR}/include)