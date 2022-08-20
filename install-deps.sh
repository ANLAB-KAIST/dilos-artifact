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
    zlib1g-dev libaio-dev libsysfs-dev hugepages net-tools python3-numpy

pip3 install -U pip wheel
pip3 install ninja meson cmake
