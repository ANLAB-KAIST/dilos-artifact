#!/bin/bash

THIS_DIR=$(readlink -f $(dirname $0))
ZOOKEEPER_CMDLINE=$($THIS_DIR/../cmdline.sh $THIS_DIR 'zookeeper')

#TODO: For now verify we can both launch zookeeper and kafka independently
# but eventually we want to run 2 OSv instances at the some time and have
# Kafka connect to Zookeeper instance
$THIS_DIR/../../scripts/tests/test_app.py -e "$ZOOKEEPER_CMDLINE" \
  --host_port 2181 \
  --guest_port 2181 \
  --kill \
  --line 'binding to port' \
#ZOOKEEPER_PID=$!
echo 'Zookeeper started'

KAFKA_CMDLINE=$($THIS_DIR/../cmdline.sh $THIS_DIR 'native')
$THIS_DIR/../../scripts/tests/test_app.py -e "$KAFKA_CMDLINE" \
  --host_port 8080 \
  --guest_port 8080 \
  --kill \
  --line 'Opening socket connection to server'
echo 'Kafka started'
