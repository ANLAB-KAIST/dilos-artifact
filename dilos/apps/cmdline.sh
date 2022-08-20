#!/bin/bash

APP_DIR=$1
CMD=${2:-'default'}

THIS_DIR=$(readlink -f $(dirname $0))
export OSV_BASE=$THIS_DIR/..
export PYTHONPATH=$OSV_BASE/scripts

python -c "import runpy; m = runpy.run_path('$APP_DIR/module.py'); print(m['$CMD'].get_launcher_args())" | tail -1
