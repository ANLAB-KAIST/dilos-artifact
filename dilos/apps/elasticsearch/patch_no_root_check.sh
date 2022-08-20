#!/bin/bash
VERSION=$1

# Download original source file and patch it return false for the check
wget https://raw.githubusercontent.com/elastic/elasticsearch/v${VERSION}/server/src/main/java/org/elasticsearch/bootstrap/Natives.java -O /tmp/Natives.java
sed -i 's/return JNANatives.definitelyRunningAsRoot()/return false/g' /tmp/Natives.java 

ES_PATH=ROOTFS/elasticsearch
ES_JARS=$(ls $ES_PATH/lib/*jar)
ES_CLASSPATH=$(echo $ES_JARS | sed 's/ /:/g')
$ES_PATH/jdk/bin/javac -d /tmp -cp $ES_CLASSPATH /tmp/Natives.java
$ES_PATH/jdk/bin/jar -uf $ES_PATH/lib/elasticsearch-${VERSION}.jar -C /tmp org/elasticsearch/bootstrap/Natives.class
