#!/bin/bash
IP_MASK=24
COMPUTE_IP=192.168.124.2
REMOTE_IP=192.168.124.3

SWAP_DEV=/dev/sdb1

COMPUTE_CPUS=48

COMPUTE_DEV=ens2f0
REMOTE_DEV=ens2f0


COMPUTE_NODE=0
REMOTE_NODE=0

REMOTE_PORT=5000

OUT_PATH=$HOME/benchmark-out/
DATE=$(date +%Y-%m-%d_%H-%M-%S)

LXC_ROOTFS=/var/lib/lxc/fastswap-container/rootfs

LXC_NAME=fastswap-container

function start_lxc(){
    lxc-start -n ${LXC_NAME} -s lxc.cgroup.memory.limit_in_bytes=$1
    sleep 5
}

function stop_lxc(){
    lxc-stop -n ${LXC_NAME} -k
}


function install_timeout() {
    rm -f build/bench-sleep
    ln -s /bin/sleep build/bench-sleep
    build/bench-sleep $1 && echo "TIMEOUT!!" && (
        stop_lxc
    ) &
}

function stop_timeout() {
    pkill -f "bench-sleep"
}

TRIES=1
TRIES=$(seq 1 ${TRIES})

FULL_MB=30720
