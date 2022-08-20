from osv.modules import api

#For a proper interactive python terminal
api.require('unknown-term')

#For sqlite3 and help()
api.require('sqlite')

default = api.run(cmdline="--env=TERM=unknown /python3")
