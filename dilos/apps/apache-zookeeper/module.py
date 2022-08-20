from osv.modules import api

api.require('java')

default = api.run(
    '--cwd=/zookeeper '
    '/java.so '
    '-Dzookeeper.log.dir="." '
    '-Dzookeeper.root.logger="INFO,CONSOLE" '
    '-Dcom.sun.management.jmxremote '
    '-Dcom.sun.management.jmxremote.local.only=false '
    '-cp /zookeeper/zookeeper-3.4.6.jar:/zookeeper/lib/slf4j-log4j12-1.6.1.jar:/zookeeper/lib/slf4j-api-1.6.1.jar:/zookeeper/lib/netty-3.7.0.Final.jar:/zookeeper/lib/log4j-1.2.16.jar:/zookeeper/lib/jline-0.9.94.jar:/zookeeper/conf/log4j.properties '
    'org.apache.zookeeper.server.quorum.QuorumPeerMain "/zookeeper/conf/zoo.cfg"'
)
