This example is a demo of Akka actor framework in Scala (https://akka.io/)
that implements simple TCP server serving list of directories and allowing
to fetch individual files. Each TCP connection is served by an actor.
The app requires JDK compact1 profile or above.
