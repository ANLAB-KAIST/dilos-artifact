from osv.modules import api

api.require('node')

default = api.run(cmdline="--cwd=/express/ /libnode.so ./examples/hello-world")
