#!/bin/bash
cd "$(dirname $0)" || exit 0
source ./config.sh

OUT_PATH="$OUT_PATH/snappy/${DATE}"
MEMORIES=(2304 4608 9216 13824 $FULL_MB)
TIMEOUT=1600

export CPUS=8

stop_lxc
mkdir -p $OUT_PATH
for TRY in ${TRIES[@]}; do
    for M in ${MEMORIES[@]}; do
        echo "${OUT_PATH}/snappy-$M-$TRY.txt"
        start_lxc ${M}M
        install_timeout ${TIMEOUT} 
        lxc-attach -n ${LXC_NAME} -L ${OUT_PATH}/snappy-compress-$M-$TRY.txt -- taskset -c 1 /apps/snappy/build/compress
        stop_timeout
        stop_lxc
    done
done

stop_lxc
for TRY in ${TRIES[@]}; do
    for M in ${MEMORIES[@]}; do
        echo "${OUT_PATH}/snappy-$M-$TRY.txt"
        start_lxc ${M}M
        install_timeout ${TIMEOUT} 
        lxc-attach -n ${LXC_NAME} -L ${OUT_PATH}/snappy-decompress-$M-$TRY.txt -- taskset -c 1 /apps/snappy/build/decompress
        stop_timeout
        stop_lxc
    done
done
