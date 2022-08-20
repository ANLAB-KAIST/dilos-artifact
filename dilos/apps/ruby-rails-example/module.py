from osv.modules import api

api.require('ruby')
default = api.run(cmdline="--cwd=/osv_test/ /ruby.so ./bin/rails server")
