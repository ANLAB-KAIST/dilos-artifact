#!/bin/bash
cd "$(dirname $0)" || exit

BUILD_PATH="$(dirname $0)/build/linux"

mkdir -p "${BUILD_PATH}"

echo "Building Kernel..."

pushd fastswap || exit
rm *.deb
rm *.tar.gz
rm *.dsc
rm *.changes
popd || exit

pushd fastswap/linux || exit
make -j "$(nproc)" deb-pkg LOCALVERSION=-fastswap
popd || exit


echo "Installing Kernel..."

pushd fastswap || exit
dpkg -i *.deb
popd || exit

