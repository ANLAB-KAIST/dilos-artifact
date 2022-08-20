#!/bin/bash
source config.sh
source scripts/config-bench.sh

echo $MEM_EXTRA_MB

OUT_PATH="$OUT_PATH/redis/${DATE}"
PREFETCHERS=(no readahead majority redis)
WORKLOADS=(get_4k get_64k get_mix lrange)

declare -A MEMORIES
MEMORIES[get_4k]="${FULL_MB} 10240 5120 2560"
MEMORIES[get_64k]="${FULL_MB} 10240 5120 2560"
MEMORIES[get_mix]="${FULL_MB} 10240 5120 2560"
MEMORIES[lrange]="${FULL_MB} 10240 5120 2560"

declare -A POPULATE
POPULATE[get_4k]="-d -4096 -n 4000000  -r 4000000 -t set"
POPULATE[get_64k]="-d -65536 -n 250000  -r 250000 -t set"
POPULATE[get_mix]="-d -1 -n 396000 -r 396000 -t set"
POPULATE[lrange]="-t random_lpush -d 1024 -n 20000000 -r 100000"

declare -A OPERATION
OPERATION[get_4k]="-d 4096 -n 4000000 -r 4000000 -t get"
OPERATION[get_64k]="-d 65536 -n 250000 -r 250000 -t get"
OPERATION[get_mix]="-d 1 -n 396000 -r 396000 -t get"
OPERATION[lrange]="-t random_lrange_600 -d 1024 -n 100000 -r 100000"

if [[ "${FULL_CLEAN}" = "y" ]]; then
    ./clean.sh
fi
./clean-app.sh
export NO_SG=y
./build.sh redis

./scripts/prepare-redis.sh

./remote.sh down
./remote.sh clean
./remote.sh build

mkdir -p $OUT_PATH

function sleep_until_redis() {
    for i in $(seq 1 60); do
        sleep 5
        PING_RESULT=$(build/redis-cli -h $IP ping 2>/dev/null | grep "PONG")
        if [ ! -z "$PING_RESULT" ]; then
            pkill -f build/redis-cli
            return
        fi
        pkill -f build/redis-cli
    done

}

export CPUS=8
export PIN=6

for W in ${WORKLOADS[@]}; do
    for TRY in ${TRIES[@]}; do
        for M in ${MEMORIES[${W}]}; do
            for P in ${PREFETCHERS[@]}; do
                pkill -f qemu-system-x86
                ./remote.sh down
                sleep 1
                ./remote.sh up
                sleep 1

                FILE_OUT_BASE="${OUT_PATH}/redis-$W-$M-$P-$TRY"
                echo ${FILE_OUT_BASE}

                MEMORY=$(expr $M + $MEM_EXTRA_MB)M PREFETCHER=$P ./run.sh >${FILE_OUT_BASE}-server.txt &
                echo "Loading..."
                sleep_until_redis

                PING_RESULT=$(build/redis-cli -h $IP ping 2>/dev/null | grep "PONG")
                pkill -f build/redis-cli
                if [ -z "$PING_RESULT" ]; then
                    echo "Ping Fail..."
                    pkill -f qemu-system-x86_64
                    continue
                fi
                echo "Redis Up"

                install_timeout 10m redis-benchmark
                sleep 1
                build/redis-benchmark -h ${IP} ${POPULATE[$W]} >"${FILE_OUT_BASE}-populate.txt"
                stop_timeout
                echo "Populate done"

                install_timeout 10m redis-benchmark
                sleep 1
                build/redis-benchmark -h ${IP} ${OPERATION[$W]} >"${FILE_OUT_BASE}-operation.txt"
                stop_timeout
                echo "Operation done"

                pkill -f qemu-system-x86_64
            done
        done
    done
done
