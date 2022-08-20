cmake_minimum_required(VERSION 3.0)
project(remote-driver CXX)

set(CMAKE_CXX_STANDARD 11)

# set(CMAKE_BUILD_TYPE "RelWithDebInfo")
# set(CMAKE_BUILD_TYPE "DILOSREL")

include_directories(../include)
include_directories(../../dilos/include)
include_directories(../../build/rdma-core/include)

add_library(remote_local STATIC local.cc)
add_library(remote_rdma STATIC rdma.cc)
add_library(remote_rdma_delayed STATIC rdma.cc)
add_library(remote_rdma_count STATIC rdma.cc)
target_compile_definitions(remote_rdma_delayed PRIVATE ENABLE_DELAY)
target_compile_definitions(remote_rdma_count PRIVATE ENABLE_NET_COUNT)
target_compile_definitions(remote_local PRIVATE REMOTE_LOCAL_SIZE_GB=${REMOTE_LOCAL_SIZE_GB})
# add_library(remote_rdma2 STATIC rdma2.cc)
