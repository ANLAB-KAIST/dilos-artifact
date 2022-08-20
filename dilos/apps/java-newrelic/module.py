from osv.modules import api

default = api.run_java(jvm_args=['-javaagent:/usr/newrelic.jar'])
