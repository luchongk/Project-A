cmake_minimum_required (VERSION 3.0.0)
cmake_policy(VERSION 3.0.0)
project(Gain VERSION 0.1)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_library(glad src/glad.c)
add_library(engine SHARED src/engine.cpp)

add_compile_definitions(UNICODE _UNICODE HIDEF_MOUSE)
add_definitions(/Zc:offsetof-)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

add_executable(Gain WIN32 src/platform_win32.cpp)

target_link_libraries(Gain engine glad opengl32 winmm)
target_link_libraries(engine glad opengl32)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/ThirdParty/)

add_custom_command(TARGET engine PRE_BUILD COMMAND del /Q ..\\bin\\engine*.pdb > NUL 2>&1)
set_target_properties(engine PROPERTIES PDB_NAME ..\\bin\\engine%time:~3,2%-%time:~6,2%-%RANDOM%)