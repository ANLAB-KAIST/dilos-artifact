# Vert.x on OSv

Vertx.io is a polyglot (Java, JavaScript, Groovy, Ruby, Ceylon, Scala and Kotlin)
toolkit based on netty and intended to build minimalistic reactive apps on JVM. Its design
is based on the non-blocking event loop model and in this way is similar to node.js.
You can read more here http://vertx.io/.

## Building
Vert.x requires at least Java 8. It can be run on compact 1 JRE.

Here is how you can build it
```
./scripts/build image=openjdk8-zulu-compact3-with-java-beans,vertx
```
