#!/bin/bash
source config.sh
source scripts/config-bench.sh
echo $MEM_EXTRA_MB
export MS_MEMORY_GB=128

OUT_PATH="$OUT_PATH/redis-sg/${DATE}"
PREFETCHERS=(readahead)
WORKLOADS=(get_128)

declare -A MEMORIES
MEMORIES[get_128]="2560"

declare -A POPULATE
POPULATE[get_128]="-d -128 -n 128000000  -r 128000000 -t set"

declare -A DELETE
DELETE[get_128]="-d 128 -n 166400000  -r 128000000 -t del"

declare -A OPERATION
OPERATION[get_128]="-d 128 -n 128000000 -r 128000000 -t get"

if [[ "${FULL_CLEAN}" = "y" ]]; then
    ./clean.sh
fi

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
export REMOTE_DRIVER=rdma_count

function run_test() {
    for W in ${WORKLOADS[@]}; do
        for TRY in ${TRIES[@]}; do
            for M in ${MEMORIES[${W}]}; do
                for P in ${PREFETCHERS[@]}; do
                    pkill -f qemu-system-x86
                    ./remote.sh down
                    sleep 1
                    ./remote.sh up
                    sleep 1

                    FILE_OUT_BASE="${OUT_PATH}/redis-$1-$W-$M-$P-$TRY"
                    echo ${FILE_OUT_BASE}

                    MEMORY=$(expr $M + $MEM_EXTRA_MB)M PREFETCHER=$P ./run.sh >${FILE_OUT_BASE}-server.txt &
                    echo "Loading..."
                    #sleep_until_redis

                    #PING_RESULT=$(build/redis-cli -h $IP ping 2>/dev/null | grep "PONG")
                    #pkill -f build/redis-cli
                    #if [ -z "$PING_RESULT" ]; then
                    #    echo "Ping Fail..."
                    #    pkill -f qemu-system-x86_64
                    #    continue
                    #fi
                    sleep 30
                    build/redis-cli -h $IP ping
                    echo "Redis Up"

                    install_timeout 80m redis-benchmark
                    sleep 1
                    build/redis-benchmark -h ${IP} ${POPULATE[$W]} >"${FILE_OUT_BASE}-populate.txt"
                    stop_timeout
                    echo "Populate done"
                    sleep 5

                    install_timeout 80m redis-benchmark
                    sleep 1
                    build/redis-benchmark -h ${IP} ${DELETE[$W]} >"${FILE_OUT_BASE}-delete.txt"
                    stop_timeout
                    echo "Delete done"
                    sleep 5

                    install_timeout 80m redis-benchmark
                    sleep 1
                    build/redis-benchmark -h ${IP} ${OPERATION[$W]} >"${FILE_OUT_BASE}-operation.txt"
                    stop_timeout
                    echo "Operation done"

                    pkill -f qemu-system-x86_64
                done
            done
        done
    done

}
./clean-app.sh
MIMALLOC=mimalloc-bitmap MIMALLOC_SUFFIX=-no-sg MIMALLOC_OPTION=-DMI_NO_SG=ON ./build.sh redis
run_test "bitmap-no-sg"

./clean-app.sh
MIMALLOC=mimalloc-bitmap ./build.sh redis
run_test "bitmap-sg"

./clean-app.sh
MIMALLOC=mimalloc ./build.sh redis
run_test "linked-list"
