from osv.modules import api

default = api.run_java(
    jvm_args=[
        '-Xms1G',
        '-Xmx1G',
        '-Djava.util.logging.config.file=/activemq/conf/logging.properties',
        '-Djava.security.auth.login.config=/activemq/conf/login.config',
        '-Dcom.sun.management.jmxremote',
        '-Djava.awt.headless=true',
        '-Djava.io.tmpdir=/activemq/tmp',
        '-Dactivemq.classpath=/activemq/conf',
        '-Dactivemq.home=/activemq',
        '-Dactivemq.base=/activemq',
        '-Dactivemq.conf=/activemq/conf',
        '-Dactivemq.data=/activemq/data',
    ],
    args=['-jar /activemq/bin/activemq.jar start']
)
