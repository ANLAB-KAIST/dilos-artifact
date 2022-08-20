#!/bin/bash
function install_timeout() {
    rm -f build/bench-sleep
    ln -s /bin/sleep build/bench-sleep
    build/bench-sleep $1 && echo "TIMEOUT!!" && (
        pkill -f $2
    ) &
}

function stop_timeout() {
    pkill -f "bench-sleep"
}

TRIES=1
TRIES=$(seq 1 ${TRIES})

export AUTO_POWEROFF=y

FULL_CLEAN=y

FULL_MB=51200
