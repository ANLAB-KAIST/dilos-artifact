import sys
import os
import subprocess

success = False

print("Started YCSB redis test ...")

curr_dir = os.getcwd()
server_host = os.getenv('OSV_HOSTNAME')
ycsb_home = os.getenv('YCSB_HOME') #Home of the cloned http://github.com/brianfrankcooper/YCSB.git

if not ycsb_home or not os.path.exists(ycsb_home):
   print("Please set YCSB_HOME env variable that points to the directory of the http://github.com/brianfrankcooper/YCSB.git project")
else:
   try:
      os.chdir(ycsb_home)
      output = subprocess.check_output(['./bin/ycsb', 'load', 'redis', '-s', '-P', 'workloads/workloada',
                                        '-p', 'redis.host=%s' % server_host, '-p', 'redis.port=6379'], stderr=subprocess.STDOUT).decode()
      print(output)
      output = subprocess.check_output(['./bin/ycsb', 'run', 'redis', '-s', '-P', 'workloads/workloada',
                                        '-p', 'redis.host=%s' % server_host, '-p', 'redis.port=6379'], stderr=subprocess.STDOUT).decode()
      print(output)

      if '[UPDATE], Return=OK' in output:
         success = True
   except subprocess.CalledProcessError as err:
      print(err.output)
   finally:
      os.chdir(curr_dir)
