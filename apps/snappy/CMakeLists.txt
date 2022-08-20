cmake_minimum_required(VERSION 3.1)
project(Snappy-Apps VERSION 1.1.8 LANGUAGES C CXX)


add_subdirectory(snappy)


add_executable(compress compress.cc)
target_link_libraries(compress PUBLIC snappy)

add_executable(decompress decompress.cc)
target_link_libraries(decompress PUBLIC snappy)


add_executable(setup setup.cc)
target_link_libraries(setup PUBLIC snappy)