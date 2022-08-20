#!/bin/bash
source config.sh

pushd tools/redis-seq || exit
make MALLOC=libc redis-benchmark redis-cli
popd || exit

mkdir -p build

cp tools/redis-seq/src/redis-benchmark build/redis-benchmark
cp tools/redis-seq/src/redis-cli build/redis-cli
