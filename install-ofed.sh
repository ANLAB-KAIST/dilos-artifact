#!/bin/bash
ROOT_PATH=$(dirname "$(realpath $0)")
cd $ROOT_PATH || exit

source config.sh

set -e
set -x

wget -O ofed.tar.gz https://content.mellanox.com/ofed/MLNX_OFED-5.2-2.2.3.0/MLNX_OFED_LINUX-5.2-2.2.3.0-ubuntu18.04-x86_64.tgz
tar -xvzf ofed.tar.gz
cd MLNX_OFED_LINUX-5.2-2.2.3.0-ubuntu18.04-x86_64
./mlnxofedinstall --all --force