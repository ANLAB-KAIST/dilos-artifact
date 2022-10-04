#!/bin/bash
ROOT_PATH=$(dirname "$(realpath $0)")
cd $ROOT_PATH || exit

source config.sh

set -e
set -x

apt update
apt install -y python3-pip build-essential gdb bridge-utils numactl \
    gfortran bison pkg-config autoconf automake swig libnl-route-3-200 \
    m4 libltdl-dev ethtool libnl-3-dev debhelper graphviz libnl-3-200 \
    autotools-dev linux-headers-`uname -r` libnuma1 dkms quilt tk \
    libnl-route-3-dev chrpath dpatch libgfortran4 flex tcl 

apt install -y libboost-all-dev
apt install -y unzip liblua5.3-dev lua5.3 libssl-dev pax-utils libyaml-cpp-dev \
    openjdk-11-jdk-headless libedit-dev libglib2.0-dev libfdt-dev libpixman-1-dev \
    zlib1g-dev libaio-dev libsysfs-dev hugepages net-tools python3-decorator python3-numpy python3-joblib python3-sklearn

apt install -y software-properties-common
add-apt-repository -y ppa:ubuntu-toolchain-r/test
apt-get install -y gcc-9 g++-9

pip3 install -U pip wheel
pip3 install ninja meson cmake
