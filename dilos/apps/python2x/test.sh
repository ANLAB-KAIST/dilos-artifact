#!/bin/bash

THIS_DIR=$(readlink -f $(dirname $0))

$THIS_DIR/../../scripts/tests/test_app.py -e \
  "--env=TERM=unknown /python -c \"print('Hello from Python 2 on OSv')\"" --line 'Hello from Python 2 on OSv'
