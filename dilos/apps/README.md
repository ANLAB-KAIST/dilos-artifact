OSv Applications
========

Introduction
------------

OSv has been designed to implement subset of Linux ABI to allow running single Linux application. 
However in its earlier days OSv could not run Linux binaries "as is" due to some limitations in its
dynamic linker, kernel memory placement and other bugs. Fairly recently most of these problems 
have been addressed and now it is possible to run many **completely unmodified Linux binaries** like 
Java, Node.JS, Python 2 and 3, iperf 3, etc. 

This repository contains the necessary glue to build many applications for OSv.
Most of the C apps get built as a shared library, but given what is written in the paragraph above,
these days it should be possible in most cases to run the same Linux application
from host **as is** unless some patch is necessary.

Each of the subdirectories here does NOT contain the original application
code. This makes this repository very small, and free of license issues.
Rather for each application this repository contains a script "GET" to
fetch this code from the Internet, patch it (if required) to run on OSv,
a Makefile to compile it for OSv (if compilation is required), and a
manifest of which files from the application should be copied into the
OSv image.

Please note that some of the apps contain ```Capstanfile``` which means that the app
can alse be built using [capstan](https://github.com/cloudius-systems/capstan#capstan).

How to use
----------

Each one of the subdirectories in this repository is an application or a "runtime" (JRE, Python or Node runtime).
In order to build any of the apps, one would clone full OSv repo that links this repo automatically as
its child under ```/apps``` subfolder. Please see the [main OSv page](https://github.com/cloudius-systems/osv#building) for details on how to setup OSv development environment. 

The simplest app is the ```native-example``` and it can be built and run on OSv like so:
```bash
# In the main OSv directory, run this to build the image:
./scripts/build image=native-example # This compiles OSv kernel and native-example app 
# and fuses all into an image located at ./buid/release/usr.img
#
# Run this to run the image
./scripts/run.py # This always runs the latest built image
```

If you want to build and run any JVM app (Java, Scala, Kotlin, Clojure), you would do something similar:
```bash
./scripts/build image=openjdk8-zulu-full,java-example # Please run ./scripts/build --help for more information
./scripts/run.py
```
Please note that ```openjdk8-zulu-full``` can be replaced with any other "Java runtime" app from this repo (see other openjdk* apps). It may seem that we are building two apps to run on OSv, but in reality the files from both ```openjdk8-zulu-full``` (JRE binaries) and ```java-example``` (Java ```*.class``` files) are overlaid on top of each other to fuse single image just like with Docker images.

Linux applications from host
----------
Please note the apps whose name ends with ```-from-host```, are the latest addition to demo creating images out of native Linux images from host.
