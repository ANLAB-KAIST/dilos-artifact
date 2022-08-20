from osv.modules import api

api.require('unknown-term')
default = api.run(cmdline="--env=TERM=unknown /python")
