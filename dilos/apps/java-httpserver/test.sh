#!/bin/bash

THIS_DIR=$(readlink -f $(dirname $0))
CMDLINE=$($THIS_DIR/../java_cmdline.sh $THIS_DIR)

$THIS_DIR/../../scripts/tests/test_http_app.py \
  -e "/usr/bin/java $CMDLINE" \
  --guest_port 8000 \
  --host_port 8000 \
  --line 'Listening on port' \
  --http_line 'This is the response'
