project(hashtable CXX)
cmake_minimum_required(VERSION 3.20)

set (CMAKE_CXX_STANDARD 17)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(../../dilos/include)
include_directories(include)


get_filename_component(remote_path ../../build/remote REALPATH)
link_directories(${remote_path}/lib ${remote_path}/driver)

add_library(local_concurrent_hopscotch src/local_concurrent_hopscotch.cpp src/init.cc src/MurmurHash3.cpp src/slab.cpp)
target_compile_options(local_concurrent_hopscotch PRIVATE -fpermissive)
target_link_libraries(local_concurrent_hopscotch nonosv)
target_link_libraries(local_concurrent_hopscotch Threads::Threads)


add_executable(main main.cpp)
target_link_libraries(main PRIVATE local_concurrent_hopscotch)