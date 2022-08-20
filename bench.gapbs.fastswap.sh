#!/bin/bash
cd "$(dirname $0)" || exit 0
source ./config.sh

OUT_PATH="$OUT_PATH/gapbs/${DATE}"
MEMORIES=($FULL_MB 7168 3584 1792)
TIMEOUT=500
export CPUS=4
stop_lxc


mkdir -p $OUT_PATH
for TRY in ${TRIES[@]}; do
    for M in ${MEMORIES[@]}; do
        echo "${OUT_PATH}/gapbs-pr-$M-$TRY.txt"
        start_lxc ${M}M
        install_timeout ${TIMEOUT}
        lxc-attach -n ${LXC_NAME} -v GOMP_CPU_AFFINITY=0-$((${CPUS} -1)) -v OMP_NUM_THREADS=${CPUS} -L ${OUT_PATH}/gapbs-pr-$M-$TRY.txt -- taskset -c 1-${CPUS} /apps/gapbs/pr -f /mnt/twitter.sg -i1000 -t1e-4 -n1
        stop_timeout
        stop_lxc
    done
done

for TRY in ${TRIES[@]}; do
    for M in ${MEMORIES[@]}; do
        echo "${OUT_PATH}/gapbs-bc-$M-$TRY.txt"
        start_lxc ${M}M
        install_timeout ${TIMEOUT}
        lxc-attach -n ${LXC_NAME} -v GOMP_CPU_AFFINITY=0-$((${CPUS} -1)) -v OMP_NUM_THREADS=${CPUS} -L ${OUT_PATH}/gapbs-bc-$M-$TRY.txt -- taskset -c 1-${CPUS} /apps/gapbs/bc -f /mnt/twitter.sg -i1 -n1
        stop_timeout
        stop_lxc
    done
done
