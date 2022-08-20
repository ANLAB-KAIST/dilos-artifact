#!/bin/bash
source config.sh

TRIES=10
TRIES=$(seq 1 ${TRIES})
OUT_PATH="$OUT_PATH/bench-sg-nonosv/${DATE}"

BIN=./apps/microbench/build/sg-nonosv
BIN_SEQ=./apps/microbench/build/sg-nonosv-seq

MASKS=(
    0x00000000FFFFFFFF
    0x0000FFFF0000FFFF
    0x00FF00FF0000FFFF
    0x00FF00FF00FF00FF
    0x0F0F0F0F00FF00FF
    0x0F0F0F0F0F0F0F0F
    0x333333330F0F0F0F
    0x3333333333333333
    0x000000000000FFFF
    0x000000FF000000FF
    0x000F000F000F000F
    0x03030303000F000F
    0x0303030303030303
    0x0303030311111111
    0x1111111111111111
)

SIZES=(
    0x1000
    0x800
    0x400
    0x200
    0x100
    0x80
)

./remote.sh down
./remote.sh clean
./remote.sh build

mkdir -p $OUT_PATH

for TRY in ${TRIES[@]}; do
    for mask in ${MASKS[@]}; do
        echo $mask
        ./remote.sh down
        ./remote.sh up || exit
        numactl -N 0 -m 0 $BIN $mask >$OUT_PATH/mask-$mask-$TRY.txt
    done
done

for TRY in ${TRIES[@]}; do
    for size in ${SIZES[@]}; do
        echo $size
        ./remote.sh down
        ./remote.sh up || exit
        numactl -N 0 -m 0 $BIN_SEQ $size >$OUT_PATH/size-$size-$TRY.txt

    done
done
