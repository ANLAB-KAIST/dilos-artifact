#!/bin/bash
source config.sh
source scripts/config-bench.sh

echo $MEM_EXTRA_MB

OUT_PATH="$OUT_PATH/seq/${DATE}"
MEMORIES=(2560)
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

            FILE_OUT="${OUT_PATH}/seq-$M-$P-$TRY.txt"
            echo ${FILE_OUT}
            install_timeout 300 qemu-system-x86_64
            sleep 1
            MEMORY=$(expr $M + $MEM_EXTRA_MB)M PREFETCHER=$P ./run.sh /seq >${FILE_OUT}
            stop_timeout
        done
    done
done
