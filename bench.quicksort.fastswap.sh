#!/bin/bash
cd "$(dirname $0)" || exit 0
source ./config.sh

OUT_PATH="$OUT_PATH/quicksort/${DATE}"
MEMORIES=(1024 2048 4096 $FULL_MB)
TIMEOUT=1600

export CPUS=8
stop_lxc
mkdir -p $OUT_PATH
for TRY in ${TRIES[@]}; do
    for M in ${MEMORIES[@]}; do
        echo "${OUT_PATH}/quicksort-$M-$TRY.txt"
        start_lxc ${M}M
        install_timeout ${TIMEOUT} 
        lxc-attach -n ${LXC_NAME} -L ${OUT_PATH}/quicksort-$M-$TRY.txt -- taskset -c 1 /apps/microbench/build/quicksort 8192
        stop_timeout
        stop_lxc
    done
done
