from osv.modules import api

api.require('erlang')
default = api.run(cmdline="/usr/lib64/erlang.so -env HOME / /etc/yaws/vm.args /etc/default/yaws/vm.args")
