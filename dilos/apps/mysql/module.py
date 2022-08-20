from osv.modules import api

default = api.run("/usr/bin/mysqld --datadir=/usr/data --user=root --innodb-use-native-aio=OFF --explicit_defaults_for_timestamp --innodb-strict-mode=OFF")
