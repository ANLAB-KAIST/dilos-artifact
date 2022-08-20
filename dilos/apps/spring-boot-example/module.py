from osv.modules import api

api.require('java')

java_cmd = "-Xms128m -Xmx128m -jar spring-boot-2-rest-service-basic.jar"
default = api.run('/java.so ' + java_cmd)
native = api.run('/usr/bin/java ' + java_cmd)
