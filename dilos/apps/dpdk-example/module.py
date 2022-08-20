from osv.modules import api

default = api.run("--maxnic=0 /test -c f -n 1 --no-shconf")
