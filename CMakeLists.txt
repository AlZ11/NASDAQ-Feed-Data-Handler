cmake_minimum_required(VERSION 3.10)
project(NASDAQFeedHandler)
set(CMAKE_CXX_STANDARD 17)

include_directories(include)
file(GLOB SOURCES "src/*.cpp")
add_executable(main ${SOURCES})
add_compile_options(-O3 -march=native)
