from osv.modules import api

api.require('erlang')
default = api.run(cmdline="/usr/lib64/erlang.so -env HOME / /etc/elixir/vm.args /etc/default/elixir/vm.args")
