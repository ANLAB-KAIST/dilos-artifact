#!/bin/sh
#
#  Copyright (C) 2013 Cloudius Systems, Ltd.
#
#  This work is open source software, licensed under the terms of the
#  BSD license as described in the LICENSE file in the top-level directory.
#

SRC_ROOT=$1

SCRIPTS_ROOT="`pwd`/scripts"
. $SCRIPTS_ROOT/ec2-utils.sh

TEST_INSTANCE_ID=`cat /tmp/test_instance_id`

post_test_cleanup() {
 if test x"$TEST_INSTANCE_ID" != x""; then
    stop_instance_forcibly $TEST_INSTANCE_ID
    wait_for_instance_shutdown $TEST_INSTANCE_ID
    delete_instance $TEST_INSTANCE_ID
 fi
}

echo "=== Cleaning up ==="
post_test_cleanup
exit 0
