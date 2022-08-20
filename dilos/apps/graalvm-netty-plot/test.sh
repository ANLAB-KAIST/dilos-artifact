#!/bin/bash

THIS_DIR=$(readlink -f $(dirname $0))
CMDLINE=$($THIS_DIR/../cmdline.sh $THIS_DIR)

$THIS_DIR/../../scripts/tests/test_http_app.py \
  -e "$CMDLINE" \
  --guest_port 8080 \
  --host_port 8080 \
  --line 'Open your web browser' \
  --http_path '/?function=abs((x-31.4)sin(x-pi/2))&xmin=0&xmax=31.4' \
  --concurrency 10 \
  --count 100
