#!/bin/bash

THIS_DIR=$(readlink -f $(dirname $0))
CMDLINE=$($THIS_DIR/../cmdline.sh $THIS_DIR)
TEST_TYPE=${1:-'benchmark'}

case $TEST_TYPE in
	benchmark)
		TESTER=tester.py;;
	ycsb)
		TESTER=tester_with_ycsb.py;;
esac

$THIS_DIR/../../scripts/tests/test_app_with_test_script.py \
  -e "$CMDLINE" \
  --guest_port 6379 \
  --host_port 6379 \
  --start_line 'Not listening to IPv6' \
  --script_path $THIS_DIR/$TESTER
