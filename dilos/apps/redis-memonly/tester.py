import sys
import os
import subprocess

success = False

print("Started redis test ...")
try:
   server_host = os.getenv('OSV_HOSTNAME')
   output = subprocess.check_output(["redis-benchmark", "-h", server_host, "-t", "set,lpush", "-n", "100000", "-q"],
       stderr=subprocess.STDOUT).decode()
   print(output)

   if 'requests per second' in output:
      success = True
except subprocess.CalledProcessError as err:
   print(err.output)
