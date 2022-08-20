from osv.modules import api

api.require('java')

default = api.run("--cwd=/spark /java.so -Xms512m -Xmx512m -cp /spark/conf:/spark/lib/spark-assembly-1.4.0-hadoop2.6.0.jar:/spark/lib/datanucleus-core-3.2.10.jar:/spark/lib/datanucleus-rdbms-3.2.9.jar:/spark/lib/datanucleus-api-jdo-3.2.6.jar -Dscala.usejavacp=true org.apache.spark.deploy.SparkSubmit --class org.apache.spark.repl.Main spark-shell")
