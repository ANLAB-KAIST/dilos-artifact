#!/bin/bash

THIS_DIR=$(readlink -f $(dirname $0))
CMDLINE=$($THIS_DIR/../cmdline.sh $THIS_DIR 'native')

$THIS_DIR/../../scripts/tests/test_app_with_test_script.py \
  -e "$CMDLINE" \
  --guest_port 1527 \
  --host_port 1527 \
  --start_line "Apache Derby Network Server" \
  --script_path $THIS_DIR/tester.py
