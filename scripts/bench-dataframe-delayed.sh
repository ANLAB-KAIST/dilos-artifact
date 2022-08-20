#!/bin/bash
source config.sh
source scripts/config-bench.sh

echo $MEM_EXTRA_MB

OUT_PATH="$OUT_PATH/dataframe/${DATE}"
MEMORIES=(5120 10240 20480 40960)
PREFETCHERS=(readahead)

if [[ "${FULL_CLEAN}" = "y" ]]; then
    ./clean.sh
fi
./clean-app.sh
export NO_SG=y
REMOTE_DRIVER=rdma_delayed ./build.sh dataframe

export CPUS=8
export PIN=6
export MS_MEMORY_GB=128

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

            FILE_OUT="${OUT_PATH}/dataframe-$M-$P-$TRY.txt"
            echo ${FILE_OUT}
            install_timeout 15m qemu-system-x86_64
            sleep 1
            MEMORY=$(expr $M + $MEM_EXTRA_MB)M PREFETCHER=$P DISK=${TRIP_DATA_CSV}.raw ./run.sh >${FILE_OUT}
            stop_timeout
        done
    done

done
