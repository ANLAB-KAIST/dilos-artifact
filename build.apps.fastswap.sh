#!/bin/bash
cd "$(dirname $0)" || exit 0
source ./config.sh

lxc-stop -n ${LXC_NAME}

echo "Copying apps"
rm -rf ${LXC_ROOTFS}/apps
cp -r apps ${LXC_ROOTFS}/apps
cp -r aifm/aifm/DataFrame/original ${LXC_ROOTFS}/apps/dataframe


lxc-start -n ${LXC_NAME} -d
sleep 5

echo "Install deps"
lxc-attach -n ${LXC_NAME} -- apt-get update
lxc-attach -n ${LXC_NAME} -- apt-get install -y python3 python3-decorator python3-numpy python3-joblib python3-sklearn python3-pip build-essential libnuma-dev software-properties-common systemtap-sdt-dev pkg-config autoconf
lxc-attach -n ${LXC_NAME} -- apt-get purge -y cmake
lxc-attach -n ${LXC_NAME} -- pip3 install -U pip wheel
lxc-attach -n ${LXC_NAME} -- pip3 install cmake ninja
lxc-attach -n ${LXC_NAME} -- add-apt-repository -y ppa:ubuntu-toolchain-r/test
lxc-attach -n ${LXC_NAME} -- apt-get install -y gcc-9 g++-9

echo "Building apps"

lxc-attach -n ${LXC_NAME} -- make -C /apps/microbench module
lxc-attach -n ${LXC_NAME} -- make -C /apps/snappy module
lxc-attach -n ${LXC_NAME} -- make -j -C /apps/gapbs bc pr
lxc-attach -n ${LXC_NAME} -- bash -c "cd /apps/redis/deps/jemalloc/ && ./autogen.sh"
lxc-attach -n ${LXC_NAME} -- make -j -C /apps/redis
lxc-attach -n ${LXC_NAME} -- /apps/build.dataframe.sh

lxc-stop -n ${LXC_NAME}

echo "Building Redis Tools"

make -j -C tools/redis-seq MALLOC=libc redis-benchmark redis-cli