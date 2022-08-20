#!/bin/bash
cd "$(dirname $0)" || exit 0
source ./config.sh

OUT_PATH="$OUT_PATH/seq/${DATE}"
MEMORIES=(2560 5120 10240 $FULL_MB)
TIMEOUT=1200

export CPUS=8
stop_lxc
mkdir -p $OUT_PATH
for TRY in ${TRIES[@]}; do
    for M in ${MEMORIES[@]}; do
        echo "${OUT_PATH}/seq-$M-$TRY.txt"
        start_lxc ${M}M
        install_timeout ${TIMEOUT} 
        lxc-attach -n ${LXC_NAME} -L ${OUT_PATH}/seq-$M-$TRY.txt -- taskset -c 1 /apps/microbench/build/seq
        stop_timeout
        stop_lxc
    done
done
