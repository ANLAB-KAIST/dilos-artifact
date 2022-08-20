from osv.modules import api

api.require('node')

default = api.run(cmdline="--cwd=/chat-example/ /libnode.so ./index.js")
