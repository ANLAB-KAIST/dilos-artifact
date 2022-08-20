#!/bin/bash

THIS_DIR=$(readlink -f $(dirname $0))
CMDLINE=$($THIS_DIR/../cmdline.sh $THIS_DIR)

$THIS_DIR/../../scripts/tests/test_app_with_test_script.py \
  -e "$CMDLINE" \
  --guest_port 5201 \
  --host_port 5201 \
  --start_line 'Server listening' \
  --end_line 'Server listening' \
  --script_path $THIS_DIR/tester.py
