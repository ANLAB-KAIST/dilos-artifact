#!/bin/bash

THIS_DIR=$(readlink -f $(dirname $0))
CMDLINE=$($THIS_DIR/../cmdline.sh $THIS_DIR)

$THIS_DIR/../../scripts/tests/test_http_app.py \
  -e "$CMDLINE" \
  --guest_port 9200 \
  --host_port 9200 \
  --line 'LicenseService' \
  --http_line 'Frank Herbert' \
  --pre_script "$THIS_DIR/tests/load_data.sh" \
  --http_path '/library/_search?q=author:frank' \
  --concurrency 50 \
  --count 1000 \
  --no_keep_alive \
  --error_line_to_ignore_on_kill "mainWithoutErrorHandling"
