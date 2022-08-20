from osv.modules import api

api.require('java')

default = api.run(cmdline='java.so -cp /java-example Hello')
