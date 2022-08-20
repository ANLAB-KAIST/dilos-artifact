#!/bin/bash
source config.sh
source scripts/config-bench.sh

echo $MEM_EXTRA_MB

OUT_PATH="$OUT_PATH/kmeans/${DATE}"
MEMORIES=(608 ${FULL_MB} 2432 1216)
PREFETCHERS=(no readahead majority)


if [[ "${FULL_CLEAN}" = "y" ]]; then
    ./clean.sh
fi
./clean-app.sh
export NO_SG=y
./build.sh kmeans

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

            FILE_OUT="${OUT_PATH}/kmeans-$M-$P-$TRY.txt"
            echo ${FILE_OUT}
            install_timeout 300 qemu-system-x86_64
            sleep 1
            MEMORY=$(expr $M + $MEM_EXTRA_MB)M PREFETCHER=$P ./run.sh >${FILE_OUT}
            stop_timeout
        done
    done
done
