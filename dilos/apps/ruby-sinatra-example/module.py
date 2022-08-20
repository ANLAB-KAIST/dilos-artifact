from osv.modules import api

api.require('ruby')
default = api.run(cmdline="--cwd=/sinatra/ --env=GEM_HOME=/sinatra/gems /ruby.so ./example.rb")
