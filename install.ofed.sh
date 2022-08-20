#!/bin/bash
cd "$(dirname $0)" || exit
apt update
apt install -y libnl-3-dev libnl-route-3-dev libnl-route-3-200 chrpath libltdl-dev dpatch graphviz autotools-dev swig pkg-config autoconf debhelper automake ethtool 

pushd mlnx-ofed-4.6 || exit
./mlnxofedinstall --add-kernel-support --dpdk --upstream-libs --without-dkms
popd || exit

