from osv.modules import api

api.require('java8')

kafka_cmd = (
    '-Xmx1G '
    '-Xms1G '
    '-XX:+UseParNewGC '
    '-XX:+UseConcMarkSweepGC '
    '-XX:+CMSClassUnloadingEnabled '
    '-XX:+CMSScavengeBeforeRemark '
    '-XX:+DisableExplicitGC '
    '-Djava.awt.headless=true '
    '-Xloggc:/kafka/logs/kafkaServer-gc.log '
    '-Dcom.sun.management.jmxremote '
    '-Dcom.sun.management.jmxremote.authenticate=false '
    '-Dcom.sun.management.jmxremote.ssl=false '
    '-Dkafka.logs.dir=/kafka/logs '
    '-Dlog4j.configuration=file:/kafka/config/log4j.properties '
    '-cp :/kafka/libs/* '
    'kafka.Kafka /kafka/config/server.properties')

zookeeper_cmd = (
    '-Xmx512M -Xms512M -server -XX:+UseG1GC -XX:MaxGCPauseMillis=20 '
    '-XX:InitiatingHeapOccupancyPercent=35 -XX:+ExplicitGCInvokesConcurrent '
    '-Djava.awt.headless=true -Xloggc:/kafka/logs/zookeeper-gc.log '
    '-Dcom.sun.management.jmxremote '
    '-Dcom.sun.management.jmxremote.authenticate=false -Dcom.sun.management.jmxremote.ssl=false '
    '-Dkafka.logs.dir=/kafka/logs -Dlog4j.configuration=file:./kafka/config/log4j.properties '
    '-cp :/kafka/libs/* '
    'org.apache.zookeeper.server.quorum.QuorumPeerMain /kafka/config/zookeeper.properties')

default = api.run('/java.so ' + kafka_cmd)
native = api.run('/usr/bin/java ' + kafka_cmd)

zookeeper = api.run('/usr/bin/java ' + zookeeper_cmd)
