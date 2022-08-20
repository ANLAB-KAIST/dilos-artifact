from osv.modules import api

default = api.run(cmdline="/haproxy.so -f /haproxy.cfg -p haproxy.pid -d -V")
