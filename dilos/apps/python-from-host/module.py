from osv.modules import api

#For a proper interactive python terminal
api.require('unknown-term')

default = api.run(cmdline="--env=TERM=unknown /python")
