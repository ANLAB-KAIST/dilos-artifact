#!/bin/bash
source config.sh

cd dilos
gdb -batch -iex "set auto-load safe-path ." \
    -ex "connect" \
    -ex "osv syms" \
    -ex "set pagination off" \
    -ex "osv trace" \
    build/release/loader.elf >"${ROOT_PATH}/build/trace.log"
