#!/bin/bash
source config.sh

for d in apps/*/; do
    pushd $d || exit
    make clean >/dev/null
    popd || exit
done

#rm -rf build/prefetcher
