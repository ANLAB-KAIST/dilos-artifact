#!/bin/bash

source ../../../shared.sh

local_rams=(2304 4608 9216 13824 18432)

rm log.*
sudo pkill -9 main

TRIES=5
TRIES=$(seq 1 ${TRIES})

for TRY in ${TRIES[@]}; do
    for local_rams in "${local_rams[@]}" 
    do
        sed "s/constexpr uint64_t kCacheSize = .*/constexpr uint64_t kCacheSize = $local_rams \* Region::kSize;/g" main.cpp -i
        make clean
        make -j
        rerun_local_iokerneld_noht
        rerun_mem_server
        run_program_noht ./main 1>log.$local_rams.$TRY 2>&1 &
        ( tail -f -n0 log.$local_rams.$TRY & ) | grep -q "Force existing..."
        sudo pkill -9 main
    done
    kill_local_iokerneld
done