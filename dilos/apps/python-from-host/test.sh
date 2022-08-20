#!/bin/bash

THIS_DIR=$(readlink -f $(dirname $0))

$THIS_DIR/../../scripts/tests/test_app.py -e \
  "--env=TERM=unknown /python -c \"import sys; print('Hello from Python %d.%d.%d on OSv' % (sys.version_info.major,sys.version_info.minor,sys.version_info.micro))\"" --line 'Hello from Python'
