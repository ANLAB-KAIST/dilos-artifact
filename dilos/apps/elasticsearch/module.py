from osv.modules import api

api.require('libz')

default = api.run( '--env=HOSTNAME=localhost /elasticsearch/java/bin/java'
'  -Xms1g -Xmx1g -XX:+UseConcMarkSweepGC -XX:CMSInitiatingOccupancyFraction=75 -XX:+UseCMSInitiatingOccupancyOnly '
'  -Des.networkaddress.cache.ttl=60 -Des.networkaddress.cache.negative.ttl=10 -XX:+AlwaysPreTouch -Xss1m -Djava.awt.headless=true '
'  -Dfile.encoding=UTF-8 -Djna.nosys=true -XX:-OmitStackTraceInFastThrow -Dio.netty.noUnsafe=true -Dio.netty.noKeySetOptimization=true '
'  -Dio.netty.recycler.maxCapacityPerThread=0 -Dlog4j.shutdownHookEnabled=false -Dlog4j2.disable.jmx=true -Djava.io.tmpdir=/tmp '
'  -XX:HeapDumpPath=data '
'  -Djava.locale.providers=COMPAT -Dio.netty.allocator.type=unpooled -XX:MaxDirectMemorySize=536870912 '
'  -Des.path.home=/elasticsearch -Des.path.conf=/elasticsearch/config -Des.distribution.flavor=default -Des.distribution.type=tar -Des.bundled_jdk=true '
'  -cp /elasticsearch/lib/* org.elasticsearch.bootstrap.Elasticsearch !')
