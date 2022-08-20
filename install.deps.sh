#!/bin/bash
cd "$(dirname $0)" || exit
apt update
apt install -y build-essential kernel-package fakeroot libncurses5-dev libssl-dev ccache bison flex lxc hugepages numactl python3-pip

pip3 install -U pip wheel
pip3 install cmake ninja

