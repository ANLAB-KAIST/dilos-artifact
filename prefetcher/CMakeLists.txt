project(prefetcher)
cmake_minimum_required(VERSION 3.20)


set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_BUILD_TYPE "RelWithDebInfo")
set(CMAKE_CXX_FLAGS "-fno-plt")

include_directories(../dilos/include)




get_filename_component(dummy_path ../dilos/build/last REALPATH)
link_directories(${dummy_path})

add_subdirectory(redis)