from osv.modules import api

api.require('ruby')
default = api.run(cmdline="--env=RUBYLIB=/usr/share/ruby:/usr/lib64/ruby /ruby.so /ruby-example/hello.rb")
