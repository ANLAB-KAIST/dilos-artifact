cmake_minimum_required(VERSION 3.0)
project(remote-server CXX)

set(CMAKE_CXX_STANDARD 11)

# set(CMAKE_BUILD_TYPE "RelWithDebInfo")
# set(CMAKE_BUILD_TYPE "DILOSREL")

include_directories(../include)
include_directories(../../dilos/include)

add_executable(remote_server server.cc)

find_library(libibverbs ibverbs)
find_library(mlx5 mlx5)

target_link_libraries(remote_server ${libibverbs} ${mlx5})