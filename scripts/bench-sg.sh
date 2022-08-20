#!/bin/bash
source config.sh

OUT_PATH="${OUT_PATH}/bench-sg/${DATE}"

MASKS=(
    0x00000000FFFFFFFF
    0x0000FFFF0000FFFF
    0x00FF00FF0000FFFF
    0x00FF00FF00FF00FF
    0x0F0F0F0F00FF00FF
    0x0F0F0F0F0F0F0F0F
    0x333333330F0F0F0F
    0x3333333333333333
)

./remote.sh down
./remote.sh clean
./remote.sh build

mkdir -p $OUT_PATH

for mask in ${MASKS[@]}; do
    echo $mask
    ./remote.sh down
    ./remote.sh up || exit
    ./clean-app.sh || exit
    ./build.sh microbench || exit
    ./run.sh --no_eviction /sg $mask >$OUT_PATH/$mask.txt
done
