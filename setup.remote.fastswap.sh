#!/bin/bash
cd "$(dirname $0)" || exit
source setup.remote.sh


echo "Setup RDMA"
ip addr add ${REMOTE_IP}/${IP_MASK} dev ${REMOTE_DEV}
ip link set ${REMOTE_DEV} up

echo "Setup HugeTLB"
hugeadm --pool-pages-min 2M:420G
hugeadm --create-mounts

echo "Building Remote Server"
pushd fastswap/farmemserver || exit
make
rm -f nohup.out
pkill -f rmserver
nohup numactl -N $REMOTE_NODE -m $REMOTE_NODE  ./rmserver ${REMOTE_PORT} &
popd || exit



