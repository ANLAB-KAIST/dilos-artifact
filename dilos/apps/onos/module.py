from osv.modules import api


default = api.run_java(
    args=['org.apache.karaf.main.Main'],
    classpath=[
        '/onos/apache-karaf-3.0.3/lib/karaf-jaas-boot.jar',
        '/onos/apache-karaf-3.0.3/lib/karaf.jar',
        '/onos/apache-karaf-3.0.3/lib/karaf-jmx-boot.jar',
        '/onos/apache-karaf-3.0.3/lib/karaf-org.osgi.core.jar'
    ],
    jvm_args=[
        '-Xms128M',
        '-Xmx512M',
        '-XX:+UnlockDiagnosticVMOptions',
        '-XX:+UnsyncloadClass',
        '-Dcom.sun.management.jmxremote',
        '-Djava.endorsed.dirs=/usr/lib/jvm/java/jre/lib/endorsed'
        ':/usr/lib/jvm/java/lib/endorsed'
        ':/onos/apache-karaf-3.0.3/lib/endorsed',
        '-Djava.ext.dirs=/usr/lib/jvm/java/jre/lib/ext:/usr/lib/jvm/java/lib/ext:/onos/apache-karaf-3.0.3/lib/ext',
        '-Dkaraf.instances=/onos/apache-karaf-3.0.3/instances',
        '-Dkaraf.home=/onos/apache-karaf-3.0.3',
        '-Dkaraf.base=/onos/apache-karaf-3.0.3',
        '-Dkaraf.data=/onos/apache-karaf-3.0.3/data',
        '-Dkaraf.etc=/onos/apache-karaf-3.0.3/etc',
        '-Djava.io.tmpdir=/onos/apache-karaf-3.0.3/data/tmp',
        '-Djava.util.logging.config.file=/onos/apache-karaf-3.0.3/etc/java.util.logging.properties',
        '-Dkaraf.startLocalConsole=true',
        '-Dkaraf.startRemoteShell=true',
    ],

)
