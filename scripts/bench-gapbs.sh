#!/bin/bash
source config.sh
source scripts/config-bench.sh

echo $MEM_EXTRA_MB

OUT_PATH="$OUT_PATH/gapbs/${DATE}"
MEMORIES=(7168 ${FULL_MB} 7168 3584 1792)
PREFETCHERS=(no readahead majority)
ALGO=(bc pr)
GRAPH_TRIAL=1

declare -A ALGO_PARAMS
ALGO_PARAMS[pr]=" -f /mnt/twitter.sg -i1000 -t1e-4 "
ALGO_PARAMS[bc]=" -f /mnt/twitter.sg -i1 "

export NO_SG=y

if [[ "${FULL_CLEAN}" = "y" ]]; then
    ./clean.sh
fi

./clean-app.sh
./build.sh gapbs

export CPUS=8
export OMP_CPUS=4
export GOMP_CPU_AFFINITY="0-3"

./remote.sh down
./remote.sh clean
./remote.sh build

mkdir -p $OUT_PATH
for A in ${ALGO[@]}; do
    for TRY in ${TRIES[@]}; do
        for M in ${MEMORIES[@]}; do
            for P in ${PREFETCHERS[@]}; do
                pkill -f qemu-system-x86_64
                ./remote.sh down
                sleep 1
                ./remote.sh up
                sleep 1

                FILE_OUT="${OUT_PATH}/gapbs-$A-$M-$P-$TRY.txt"
                echo ${FILE_OUT}
                install_timeout 300 qemu-system-x86_64
                sleep 1
                MEMORY=$(expr $M + $MEM_EXTRA_MB)M PREFETCHER=$P DISK=${TWITTER}.raw ./run.sh --env=GOMP_CPU_AFFINITY=$GOMP_CPU_AFFINITY --env=OMP_NUM_THREADS=$OMP_CPUS /${A} ${ALGO_PARAMS[$A]} -n${GRAPH_TRIAL} >${FILE_OUT}
                stop_timeout
            done
        done
    done
done
