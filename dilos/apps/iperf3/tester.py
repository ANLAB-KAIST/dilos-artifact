import sys
import subprocess
import os

success = False

print("Started iperf3 test ...")
try:
   server_host = os.getenv('OSV_HOSTNAME')
   output = subprocess.check_output(["iperf3", "-t", "5", "-c", server_host], stderr=subprocess.STDOUT).decode()
   print(output)

   if 'iperf Done' in output:
      success = True
except subprocess.CalledProcessError as err:
   print(err.output)
