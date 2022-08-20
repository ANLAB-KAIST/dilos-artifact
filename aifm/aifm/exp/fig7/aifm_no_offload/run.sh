#!/bin/bash

log_folder=`pwd`

cd ../../../
source shared.sh

cache_sizes=(5 10 20 30 40)

sudo pkill -9 main

CXXFLAGS="-DDISABLE_OFFLOAD_UNIQUE -DDISABLE_OFFLOAD_COPY_DATA_BY_IDX -DDISABLE_OFFLOAD_SHUFFLE_DATA_BY_IDX -DDISABLE_OFFLOAD_ASSIGN -DDISABLE_OFFLOAD_AGGREGATE"
make clean
make -j CXXFLAGS="$CXXFLAGS"

cd DataFrame/AIFM/
rm -rf build
mkdir build
cd build
cmake -E env CXXFLAGS="$CXXFLAGS" cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++-9 ..
cd ..

TRIES=5
TRIES=$(seq 1 ${TRIES})

for TRY in ${TRIES[@]}; do
    for cache_size in ${cache_sizes[@]}
    do
        sed "s/constexpr uint64_t kCacheGBs.*/constexpr uint64_t kCacheGBs = $cache_size;/g" app/main_tcp.cc -i
        cd build
        make clean
        make -j
        rerun_local_iokerneld_noht
        rerun_mem_server
        run_program_noht ./bin/main_tcp 1>$log_folder/log.$cache_size.$TRY 2>&1
        cd ..
    done
    kill_local_iokerneld
done