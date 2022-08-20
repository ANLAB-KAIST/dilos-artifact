from osv.modules import api

api.require('java')

java_cmd = '-Xms64m -Xmx64m -Dvertx.disableDnsResolver=true -jar HelloWorld.jar'
default = api.run('/java.so ' + java_cmd)
native = api.run('/usr/bin/java ' + java_cmd)
