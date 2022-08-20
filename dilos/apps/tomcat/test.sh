#!/bin/bash

THIS_DIR=$(readlink -f $(dirname $0))
CMDLINE=$($THIS_DIR/../java_cmdline.sh $THIS_DIR)

$THIS_DIR/../../scripts/tests/test_http_app.py \
  -e "/usr/bin/java $CMDLINE" \
  --guest_port 8081 \
  --host_port 8081 \
  --line 'Server startup' \
  --http_line 'Apache Tomcat'
