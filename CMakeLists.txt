cmake_minimum_required (VERSION 3.0.0)
cmake_policy(VERSION 3.0.0)
project(Gain VERSION 0.1)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_library(glad src/glad.c)
add_library(game SHARED src/game.cpp)

add_compile_definitions(UNICODE _UNICODE)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

add_executable(Gain WIN32 src/platform_win32.cpp)

target_link_libraries(Gain game glad opengl32 winmm)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_custom_command(TARGET game PRE_BUILD COMMAND del /Q ..\\bin\\game*.pdb > NUL 2>&1)
add_custom_command(TARGET game POST_BUILD COMMAND ../scripts/renamePdb)