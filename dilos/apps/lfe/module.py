from osv.modules import api

api.require('erlang')
default = api.run(cmdline="/usr/lib64/erlang.so -env HOME / /etc/lfe/vm.args /etc/default/lfe/vm.args")
