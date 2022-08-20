#!/bin/bash
cd "$(dirname $0)" || exit 0
source ./config.sh

OUT_PATH="$OUT_PATH/kmeans/${DATE}"
MEMORIES=(608 $FULL_MB 2432 1216)
TIMEOUT=500


export CPUS=8
stop_lxc
mkdir -p $OUT_PATH
for TRY in ${TRIES[@]}; do
    for M in ${MEMORIES[@]}; do
        echo "${OUT_PATH}/kmeans-$M-$TRY.txt"
        start_lxc ${M}M
        install_timeout ${TIMEOUT}
        lxc-attach -n ${LXC_NAME} -L ${OUT_PATH}/kmeans-$M-$TRY.txt -- taskset -c 1 python3 /apps/kmeans/run.py
        stop_timeout
        stop_lxc
    done
done
