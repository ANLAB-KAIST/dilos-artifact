import os
import sys
import subprocess

success = False

print("Started mysql test ...")
try:
   sysbench_script_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'sysbench.sh')
   output = subprocess.check_output([sysbench_script_path], stderr=subprocess.STDOUT).decode()
   print(output)

   if 'execution time' in output:
      success = True
except subprocess.CalledProcessError as err:
   print(err.output)
