from osv.modules import api

api.require('java')

default = api.run_java(args=['-jar', '/usr/oracle/kv-3.0.5/lib/kvstore.jar', 'kvlite', '-host', '192.168.122.89'])
