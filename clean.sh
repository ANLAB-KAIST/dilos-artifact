#!/bin/bash
source config.sh

pushd dilos
make clean
popd

rm -r build
