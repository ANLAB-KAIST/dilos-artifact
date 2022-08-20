#!/bin/bash
cd "$(dirname $0)" || exit 0
source ./config.sh

OUT_PATH="$OUT_PATH/redis/${DATE}"
TIMEOUT=1200

WORKLOADS=(get_128)
# get_4k get_64k get_mix lrange)

declare -A MEMORIES
MEMORIES[get_128]="${FULL_MB} 10240 5120 2560"
MEMORIES[get_4k]="${FULL_MB} 10240 5120 2560"
MEMORIES[get_64k]="${FULL_MB} 10240 5120 2560"
MEMORIES[get_mix]="${FULL_MB} 10240 5120 2560"
MEMORIES[lrange]="${FULL_MB} 10240 5120 2560"

declare -A POPULATE
POPULATE[get_128]="-d -128 -n 50000000  -r 50000000 -t set"
POPULATE[get_4k]="-d -4096 -n 4000000  -r 4000000 -t set"
POPULATE[get_64k]="-d -65536 -n 250000  -r 250000 -t set"
POPULATE[get_mix]="-d -1 -n 396000 -r 396000 -t set"
POPULATE[lrange]="-t random_lpush -d 1024 -n 20000000 -r 100000"

declare -A OPERATION
POPULATE[get_128]="-d 128 -n 50000000  -r 50000000 -t get"
OPERATION[get_4k]="-d 4096 -n 4000000 -r 4000000 -t get"
OPERATION[get_64k]="-d 65536 -n 250000 -r 250000 -t get"
OPERATION[get_mix]="-d 1 -n 396000 -r 396000 -t get"
OPERATION[lrange]="-t random_lrange_600 -d 1024 -n 100000 -r 100000"


stop_lxc
mkdir -p $OUT_PATH
for W in ${WORKLOADS[@]}; do
    for TRY in ${TRIES[@]}; do
        for M in ${MEMORIES[@]}; do
            FILE_OUT_BASE="${OUT_PATH}/redis-$W-$M-$TRY"
            echo "$FILE_OUT_BASE"
            start_lxc ${M}M

            # Start Redis
            lxc-attach -n ${LXC_NAME} -L ${FILE_OUT_BASE}-server.txt -- nohup taskset -c 1 /apps/redis/src/redis-server /apps/redis.conf &
            echo "Loading..."
            sleep 5
            IP=$(lxc-info -n ${LXC_NAME} -i -H)

            echo "Redis Up: ${IP}"

            # POPULATE
            install_timeout ${TIMEOUT}
            ./tools/redis-seq/src/redis-benchmark -h ${IP} ${POPULATE[$W]} >"${FILE_OUT_BASE}-populate.txt"
            stop_timeout
            echo "Populate done"


            # Operation
            install_timeout ${TIMEOUT}
            ./tools/redis-seq/src/redis-benchmark -h ${IP} ${OPERATION[$W]} >"${FILE_OUT_BASE}-operation.txt"
            stop_timeout
            echo "Operation done"

            cp /var/lib/lxc/${LXC_NAME}/rootfs/nohup.out ${FILE_OUT_BASE}-server-nohup.txt || true

            stop_lxc
        done
    done
done
