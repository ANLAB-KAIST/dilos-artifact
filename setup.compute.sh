#!/bin/bash
cd "$(dirname $0)" || exit
source config.sh


echo "Setup RDMA"
ip addr add ${COMPUTE_IP}/${IP_MASK} dev ${COMPUTE_DEV}
ip link set ${COMPUTE_DEV} up