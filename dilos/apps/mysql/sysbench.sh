#!/bin/bash
sysbench /usr/share/sysbench/oltp_read_write.lua --mysql-host=$OSV_HOSTNAME --mysql-port=3306 --mysql-user=admin --mysql-password=osv --mysql-db=test --db-driver=mysql --tables=3 --table-size=100000 prepare

sleep 1

sysbench /usr/share/sysbench/oltp_read_write.lua --mysql-host=$OSV_HOSTNAME --mysql-port=3306 --mysql-user=admin --mysql-password='osv' --mysql-db=test --db-driver=mysql --tables=3 --table-size=100000 --report-interval=10 --threads=12 --time=60 run 2>&1
