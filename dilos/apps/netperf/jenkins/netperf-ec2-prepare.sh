#!/bin/sh
#
#  Copyright (C) 2013 Cloudius Systems, Ltd.
#
#  This work is open source software, licensed under the terms of the
#  BSD license as described in the LICENSE file in the top-level directory.
#

SRC_ROOT=$1
AWS_REGION=$2
AWS_ZONE=$3
AWS_PLACEMENT_GROUP=$4
INSTANCE_TYPE=$5

SCRIPTS_ROOT="`pwd`/scripts"

. $SCRIPTS_ROOT/ec2-utils.sh

post_test_cleanup() {
 if test x"$TEST_INSTANCE_ID" != x""; then
    stop_instance_forcibly $TEST_INSTANCE_ID
    wait_for_instance_shutdown $TEST_INSTANCE_ID
    delete_instance $TEST_INSTANCE_ID
 fi
}

handle_test_error() {
 echo "=== Error occured. Cleaning up. ==="
 post_test_cleanup
 exit 1
}

prepare_instance_for_test() {
 local TEST_OSV_VER=`$SCRIPTS_ROOT/osv-version.sh`-jenkins-netperf-ec2-`timestamp`
 local TEST_INSTANCE_NAME=OSv-$TEST_OSV_VER

 if test x"$AWS_PLACEMENT_GROUP" != x""; then
  PLACEMENT_GROUP_PARAM="--placement-group $AWS_PLACEMENT_GROUP"
 fi

 echo "=== Create OSv instance ==="
 $SCRIPTS_ROOT/release-ec2.sh --override-image build/release.x64/osv.raw \
                              --instance-only \
                              --override-version $TEST_OSV_VER \
                              --region $AWS_REGION \
                              --zone $AWS_ZONE \
                              $PLACEMENT_GROUP_PARAM || handle_test_error

 TEST_INSTANCE_ID=`get_instance_id_by_name $TEST_INSTANCE_NAME`

 if test x"$TEST_INSTANCE_ID" = x""; then
  handle_test_error
 fi

 change_instance_type $TEST_INSTANCE_ID $INSTANCE_TYPE || handle_test_error
 start_instances $TEST_INSTANCE_ID || handle_test_error
 wait_for_instance_startup $TEST_INSTANCE_ID 300 || handle_test_error

 TEST_INSTANCE_IP=`get_instance_private_ip $TEST_INSTANCE_ID`

 if test x"$TEST_INSTANCE_IP" = x""; then
  handle_test_error
 fi

}

echo "=== Build everything ==="
( cd $SRC_ROOT && \
  rm -rf netperf-2.6.0.tar.bz2 netperf-2.6.0 && \
  wget ftp://ftp.netperf.org/netperf/netperf-2.6.0.tar.bz2 && \
  tar jxf netperf-2.6.0.tar.bz2 && \
  cd netperf-2.6.0 && ./configure && make -j `nproc` && sudo make install ) \
  || handle_test_error

make -j `nproc` image=netperf img_format=raw || handle_test_error
scripts/convert raw

prepare_instance_for_test
echo $TEST_INSTANCE_ID > /tmp/test_instance_id
echo $TEST_INSTANCE_IP > /tmp/test_instance_ip
cp -v apps/netperf/jenkins/netperf-xml.py /tmp
