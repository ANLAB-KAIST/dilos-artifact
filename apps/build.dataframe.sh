#!/bin/bash
cd "$(dirname $0)" || exit 0

cd dataframe || exit 0

rm -rf build
mkdir build
cd build || exit 0
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++-9 ..
make -j