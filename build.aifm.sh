#!/bin/bash
cd "$(dirname $0)" || exit

pushd aifm || exit
./build_all.sh
popd || exit
