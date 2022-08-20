#!/bin/bash

THIS_DIR=$(readlink -f $(dirname $0))
CMDLINE=$($THIS_DIR/../cmdline.sh $THIS_DIR)

if [ "$OSV_HYPERVISOR" == "firecracker" ]; then
$THIS_DIR/../../scripts/tests/test_app_with_test_script.py \
  -e "$CMDLINE" \
  --start_line 'ready for connections' \
  --script_path $THIS_DIR/tester.py
else
sudo $THIS_DIR/../../scripts/tests/test_app_with_test_script.py \
  -e "$CMDLINE" \
  --vhost \
  --start_line 'ready for connections' \
  --script_path $THIS_DIR/tester.py
fi
