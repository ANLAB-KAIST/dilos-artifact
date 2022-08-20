#!/bin/bash
ROOT_PATH=$(dirname "$(realpath $0)")
cd $ROOT_PATH || exit

source config.sh

message() {
    printf "\033[0;32m$1\033[0m\n"
}

RDMA_CORE_PATH=build/rdma-core
QEMU_PATH=build/qemu
DPDK_PATH=build/dpdk
MIMALLOC_BUILD_PATH=build/mimalloc/${MIMALLOC}${MIMALLOC_SUFFIX}
MIMALLOC_LAST_PATH=build/mimalloc/last
REMOTE_PATH=build/remote
PREFETCHER_PATH=build/prefetcher

REMOTE_DRIVER_LIB="libremote_$REMOTE_DRIVER.a"
message "ROOT_PATH: $ROOT_PATH"
message "MIMALLOC_BUILD_PATH: $MIMALLOC_BUILD_PATH"
message "REMOTE_DRIVER_LIB: $REMOTE_DRIVER_LIB"
# RDMA_CORE_FORCE=1
# QEMU_FORCE=1
# DPDK_FORCE=1
# MIMALLOC_FORCE=1
# REMOTE_FORCE=1
# PREFETCHER_FORCE=1

message "Building RDMA-Core"
if [ ! -d "$RDMA_CORE_PATH" ] || [ ! -z $RDMA_CORE_FORCE ]; then
    mkdir -p $RDMA_CORE_PATH
    pushd $RDMA_CORE_PATH || exit
    cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DIN_PLACE=1 -DENABLE_STATIC=1 -DENABLE_RESOLVE_NEIGH=0 \
        -DENABLE_VALGRIND=0 -DIOCTL_MODE=write -DNO_PYVERBS=1 -GNinja "${ROOT_PATH}/rdma-core" || exit
    popd || exit
fi
pushd $RDMA_CORE_PATH || exit
ninja || exit
popd || exit

message "Building QEMU"
if [ ! -d "$QEMU_PATH" ] || [ ! -z $QEMU_FORCE ]; then
    mkdir -p $QEMU_PATH
    pushd $QEMU_PATH || exit
    "${ROOT_PATH}/qemu/configure" --prefix="${ROOT_PATH}/${QEMU_PATH}" --target-list=x86_64-softmmu --enable-linux-aio --disable-rdma
    popd || exit
fi
pushd $QEMU_PATH || exit
make -j >${ROOT_PATH}/build/qemu.log || exit
popd || exit

message "Building mimalloc"
if [ ! -d "$MIMALLOC_BUILD_PATH" ] || [ ! -z $MIMALLOC_FORCE ]; then
    mkdir -p $MIMALLOC_BUILD_PATH
    pushd $MIMALLOC_BUILD_PATH || exit
    cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DMI_DDC="${ROOT_PATH}/dilos/include" \
        -DMI_OVERRIDE=OFF -DMI_INTERPOSE=OFF -DMI_BUILD_OBJECT=OFF -DMI_BUILD_SHARED=OFF -DMI_BUILD_TESTS=OFF \
        -GNinja ${MIMALLOC_OPTION} "${ROOT_PATH}/${MIMALLOC}" || exit
    popd || exit
fi
pushd $MIMALLOC_BUILD_PATH || exit
ninja || exit
popd || exit

if [ ! "$(readlink $MIMALLOC_LAST_PATH)" = "$MIMALLOC${MIMALLOC_SUFFIX}" ]; then
    rm -f $MIMALLOC_LAST_PATH
    ln -s $MIMALLOC${MIMALLOC_SUFFIX} $MIMALLOC_LAST_PATH
    touch $MIMALLOC_LAST_PATH/libmimalloc.a
fi

message "Building Remote Drivers"
if [ ! -d "$REMOTE_PATH" ] || [ ! -z $REMOTE_FORCE ]; then
    mkdir -p $REMOTE_PATH
    pushd $REMOTE_PATH || exit
    cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -GNinja -DREMOTE_LOCAL_SIZE_GB=${REMOTE_LOCAL_SIZE_GB} "${ROOT_PATH}/remote" || exit
    popd || exit
fi
pushd $REMOTE_PATH || exit
ninja || exit
popd || exit

REMOTE_DRIVER_LIB_LINK_PATH=$REMOTE_PATH/driver/libremote.a
if [ ! "$(readlink $REMOTE_DRIVER_LIB_LINK_PATH)" = "$REMOTE_DRIVER_LIB" ]; then
    rm -f $REMOTE_DRIVER_LIB_LINK_PATH
    ln -s $REMOTE_DRIVER_LIB $REMOTE_DRIVER_LIB_LINK_PATH
    touch $REMOTE_DRIVER_LIB_LINK_PATH
fi

export QEMU_NBD=${ROOT_PATH}/${QEMU_PATH}/qemu-nbd
export QEMU_IMG=${ROOT_PATH}/${QEMU_PATH}/qemu-img

message "Building libosv.so"
pushd dilos || exit

if [[ "${NO_SG}" = "y" ]]; then
    ./scripts/build -j mode=release DILOS_ROOT="$ROOT_PATH" DILOS_NO_SG=y fs=rofs image=native-example || exit
else
    ./scripts/build -j mode=release DILOS_ROOT="$ROOT_PATH" fs=rofs image=native-example || exit
fi

popd || exit

message "Building Prefetchers"
if [ ! -d "$PREFETCHER_PATH" ] || [ ! -z $PREFETCHER_FORCE ]; then
    mkdir -p $PREFETCHER_PATH
    pushd $PREFETCHER_PATH || exit
    cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -GNinja "${ROOT_PATH}/prefetcher" || exit
    popd || exit
fi
pushd $PREFETCHER_PATH || exit
ninja || exit
popd || exit

export QEMU_NBD=${ROOT_PATH}/${QEMU_PATH}/qemu-nbd
export QEMU_IMG=${ROOT_PATH}/${QEMU_PATH}/qemu-img

message "Building DiLOS"
pushd dilos || exit
if [[ -n "$*" ]]; then
    IMAGE="$*"
else
    IMAGE=native-example
fi

if [[ "${NO_SG}" = "y" ]]; then
    ./scripts/build -j DILOS_ROOT="$ROOT_PATH" DILOS_NO_SG=y fs=rofs image=$IMAGE
else
    ./scripts/build -j DILOS_ROOT="$ROOT_PATH" fs=rofs image=$IMAGE
fi

popd || exit
