project(bench C CXX)
cmake_minimum_required(VERSION 3.20)

FILE(GLOB SRCS *.c *.cc)

set (CMAKE_C_STANDARD 11)
set (CMAKE_CXX_STANDARD 11)

set(CMAKE_SKIP_BUILD_RPATH TRUE)

include_directories(../../dilos/include)

# add_subdirectory(../../dummy dummy)

get_filename_component(dummy_path ../../dilos/build/last REALPATH)
link_directories(${dummy_path})

get_filename_component(remote_path ../../build/remote REALPATH)
link_directories(${remote_path}/lib ${remote_path}/driver)



file(WRITE usr.manifest "")
foreach(src ${SRCS})
    get_filename_component(target "${src}" NAME_WE)
    add_executable(${target} ${src})
    target_link_libraries(${target} osv)
    file(APPEND usr.manifest "/${target}: ${CMAKE_BINARY_DIR}/${target}\n")
    target_compile_options(${target} PRIVATE -fPIC)
endforeach()
add_executable(sg-nonosv sg.cc)
target_link_libraries(sg-nonosv remote_rdma)
target_link_libraries(sg-nonosv ibverbs)
target_link_libraries(sg-nonosv mlx5)
target_link_libraries(sg-nonosv nonosv)
target_compile_definitions(sg-nonosv PRIVATE NONOSV)
target_include_directories(sg-nonosv PRIVATE ../../remote/include)
target_compile_definitions(sg-nonosv PRIVATE DO_SG)


add_executable(sg2-nonosv sg2.cc)
target_link_libraries(sg2-nonosv remote_rdma)
target_link_libraries(sg2-nonosv ibverbs)
target_link_libraries(sg2-nonosv mlx5)
target_link_libraries(sg2-nonosv nonosv)
target_compile_definitions(sg2-nonosv PRIVATE NONOSV)
target_include_directories(sg2-nonosv PRIVATE ../../remote/include)


add_executable(sg-nonosv-seq sg.cc)
target_link_libraries(sg-nonosv-seq remote_rdma)
target_link_libraries(sg-nonosv-seq ibverbs)
target_link_libraries(sg-nonosv-seq mlx5)
target_link_libraries(sg-nonosv-seq nonosv)
target_compile_definitions(sg-nonosv-seq PRIVATE NONOSV)
target_include_directories(sg-nonosv-seq PRIVATE ../../remote/include)
