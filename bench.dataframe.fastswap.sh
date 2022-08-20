#!/bin/bash
cd "$(dirname $0)" || exit 0
source ./config.sh

OUT_PATH="$OUT_PATH/dataframe/${DATE}"
MEMORIES=(5120 10240 20480 40960)
TIMEOUT=500

export CPUS=8
stop_lxc
mkdir -p $OUT_PATH
for TRY in ${TRIES[@]}; do
    for M in ${MEMORIES[@]}; do
        echo "${OUT_PATH}/dataframe-$M-$TRY.txt"
        start_lxc ${M}M
        install_timeout ${TIMEOUT}
        lxc-attach -n ${LXC_NAME} -L ${OUT_PATH}/dataframe-$M-$TRY.txt -- taskset -c 1 /apps/dataframe/build/bin/main
        stop_timeout
        stop_lxc
    done
done
