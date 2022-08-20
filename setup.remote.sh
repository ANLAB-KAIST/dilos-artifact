#!/bin/bash
cd "$(dirname $0)" || exit
source config.sh


echo "Setup RDMA"
ip addr add ${REMOTE_IP}/${IP_MASK} dev ${REMOTE_DEV}
ip link set ${REMOTE_DEV} up
