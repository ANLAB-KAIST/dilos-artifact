#!/bin/bash
source config.sh
source scripts/config-bench.sh

echo $MEM_EXTRA_MB

OUT_PATH="$OUT_PATH/quicksort/${DATE}"
MEMORIES=(1024 2048 4096 ${FULL_MB})
PREFETCHERS=(no readahead majority)

if [[ "${FULL_CLEAN}" = "y" ]]; then
    ./clean.sh
fi
./clean-app.sh
export NO_SG=y
./build.sh microbench

export CPUS=8
export PIN=6

./remote.sh down
./remote.sh clean
./remote.sh build

mkdir -p $OUT_PATH
for TRY in ${TRIES[@]}; do
    for M in ${MEMORIES[@]}; do
        for P in ${PREFETCHERS[@]}; do
            pkill -f qemu-system-x86_64
            ./remote.sh down
            sleep 1
            ./remote.sh up
            sleep 1

            FILE_OUT="${OUT_PATH}/quicksort-$M-$P-$TRY.txt"
            echo ${FILE_OUT}
            install_timeout 600 qemu-system-x86_64
            sleep 1
            MEMORY=$(expr $M + $MEM_EXTRA_MB)M PREFETCHER=$P ./run.sh /quicksort 8192 >${FILE_OUT}
            stop_timeout
        done
    done
done
