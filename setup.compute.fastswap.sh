#!/bin/bash
cd "$(dirname $0)" || exit
source setup.compute.sh

echo "Disable THP"
echo never > /sys/kernel/mm/transparent_hugepage/enabled
echo never > /sys/kernel/mm/transparent_hugepage/defrag



echo "Building Kernel Module"
pushd fastswap/drivers || exit
make BACKEND=RDMA
popd || exit

echo "Enable Swap"

swapoff -a
swapon $SWAP_DEV



echo "Loading Kernel Module"
pushd fastswap/drivers || exit
modprobe rdma_cm
insmod fastswap_rdma.ko sport=${REMOTE_PORT} sip="$REMOTE_IP" cip="$COMPUTE_IP" nq=${COMPUTE_CPUS}
insmod fastswap.ko
popd || exit


