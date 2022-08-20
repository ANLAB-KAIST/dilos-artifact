#!/bin/bash
source config.sh
source scripts/config-bench.sh

echo $MEM_EXTRA_MB

OUT_PATH="$OUT_PATH/snappy/${DATE}"
MEMORIES=(${FULL_MB} 8192 4096 2048)
PREFETCHERS=(no readahead majority)
ALGO=(compress decompress)

declare -A DISK_FILE
DISK_FILE[compress]=${ENWIKI_UNCOMP}
DISK_FILE[decompress]=${ENWIKI_COMP}

if [[ "${FULL_CLEAN}" = "y" ]]; then
    ./clean.sh
fi
./clean-app.sh
export NO_SG=y
./build.sh snappy

export CPUS=8
export PIN=6

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

                FILE_OUT="${OUT_PATH}/snappy-$A-$M-$P-$TRY.txt"
                echo ${FILE_OUT}
                install_timeout 5m qemu-system-x86_64
                sleep 1
                MEMORY=$(expr $M + $MEM_EXTRA_MB)M PREFETCHER=$P DISK=${DISK_FILE[$A]}.raw ./run.sh /${A} >${FILE_OUT}
                stop_timeout
            done
        done
    done
done
