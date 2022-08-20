from osv.modules import api

default = api.run_java(
    jvm_args=[
#       '-ea',
        '-javaagent:/usr/cassandra/lib/jamm-0.2.6.jar',
        '-XX:+CMSClassUnloadingEnabled',
        '-XX:+UseThreadPriorities',
        '-XX:ThreadPriorityPolicy=42',
        '-Xms1918M',
        '-Xmx1918M',
        '-Xmn479M',
        '-XX:+HeapDumpOnOutOfMemoryError',
        '-Xss256k',
        '-XX:StringTableSize=1000003',
        '-XX:+UseParNewGC',
        '-XX:+UseConcMarkSweepGC',
        '-XX:+CMSParallelRemarkEnabled',
        '-XX:SurvivorRatio=8',
        '-XX:MaxTenuringThreshold=1',
        '-XX:CMSInitiatingOccupancyFraction=75',
        '-XX:+UseCMSInitiatingOccupancyOnly',
        '-XX:+UseTLAB',
#       '-XX:+CMSParallelInitialMarkEnabled',
#       '-XX:+CMSEdenChunksRecordAlways',
        '-XX:+UseCondCardMark',
        '-Djava.rmi.server.hostname=$OSV_IP',
        '-Djava.net.preferIPv4Stack=true',
        '-Dcom.sun.management.jmxremote.port=7199',
        '-Dcom.sun.management.jmxremote.rmi.port=7199',
        '-Dcom.sun.management.jmxremote.ssl=false',
        '-Dcom.sun.management.jmxremote.authenticate=false',
        '-Dlogback.configurationFile=logback.xml',
        '-Dcassandra.logdir=/usr/cassandra/logs',
        '-Dcassandra.storagedir=/usr/cassandra/data',
        '-Dcassandra-foreground=yes'
    ],
    classpath=[
        '/usr/cassandra/conf/',
        '/usr/cassandra/lib/*'
    ],
    args=['org.apache.cassandra.service.CassandraDaemon'])
