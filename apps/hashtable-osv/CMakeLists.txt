project(hashtable CXX)
cmake_minimum_required(VERSION 3.20)

set (CMAKE_CXX_STANDARD 17)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(../../dilos/build/last/gen/include)
include_directories(../../dilos/include)
include_directories(../../dilos/include/api)
include_directories(../../dilos/include/api/x64)
include_directories(../../dilos)
include_directories(../../dilos/arch/x64)
include_directories(include)

set(CMAKE_CXX_FLAGS "-fno-plt")

get_filename_component(dummy_path ../../dilos/build/last REALPATH)
link_directories(${dummy_path})



add_library(local_concurrent_hopscotch src/local_concurrent_hopscotch.cpp src/MurmurHash3.cpp src/slab.cpp)
target_compile_options(local_concurrent_hopscotch PRIVATE -fpermissive)
target_link_libraries(local_concurrent_hopscotch osv)


# add_executable(main main.cpp)
# target_link_libraries(main PRIVATE local_concurrent_hopscotch)