from osv.modules import api

api.require('ruby')
default = api.run(cmdline="--cwd=/publify/ /ruby.so ./bin/rails server")
