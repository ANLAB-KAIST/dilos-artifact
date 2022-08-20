from osv.modules import api
import glob
import os

#For a proper interactive python terminal
api.require('unknown-term')

#For sqlite3 and help()
api.require('sqlite')

module_path=os.path.dirname(os.path.realpath(__file__))

default = api.run(cmdline="--env=TERM=unknown /python3 /run.py")
