#!/bin/bash

APP_DIR=$1
CMD=${2:-'default'}

THIS_DIR=$(readlink -f $(dirname $0))
export OSV_BASE=$THIS_DIR/..
export PYTHONPATH=$OSV_BASE/scripts

python -c "import runpy; m = runpy.run_path('$APP_DIR/module.py'); print(' '.join(m['$CMD'].get_multimain_lines()))" | tail -1
