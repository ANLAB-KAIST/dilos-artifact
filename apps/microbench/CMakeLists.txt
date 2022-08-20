project(bench C CXX)
cmake_minimum_required(VERSION 3.20)

FILE(GLOB SRCS *.c *.cc)

set (CMAKE_C_STANDARD 11)
set (CMAKE_CXX_STANDARD 11)

set(CMAKE_SKIP_BUILD_RPATH TRUE)



file(WRITE usr.manifest "")
foreach(src ${SRCS})
    get_filename_component(target "${src}" NAME_WE)
    add_executable(${target} ${src})
endforeach()