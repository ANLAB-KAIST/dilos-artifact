#!/bin/bash

THIS_DIR=$(readlink -f $(dirname $0))
CMDLINE=$($THIS_DIR/../cmdline.sh $THIS_DIR)

$THIS_DIR/../../scripts/tests/test_http_app.py \
  -e "$CMDLINE" \
  --guest_port 80 \
  --host_port 8000 \
  --line 'initgroups' \
  --http_line 'Welcome to nginx'
