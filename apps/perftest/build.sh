#!/bin/bash

mkdir -p build
pushd build || exit

../perftest/configure
make -j

popd || exit
