module: httpserverapp

include ../graalvm-example/common.gmk

HttpServerApp.class: ../java-httpserver/HttpServerApp.java ../graalvm-example/upstream/graalvm-ce-java11-$(GRAAL_VERSION)
	../graalvm-example/upstream/graalvm-ce-java11-$(GRAAL_VERSION)/bin/javac -d . ../java-httpserver/HttpServerApp.java

httpserverapp: HttpServerApp.class ../graalvm-example/upstream/graalvm-ce-java11-$(GRAAL_VERSION)/bin/native-image
	../graalvm-example/upstream/graalvm-ce-java11-$(GRAAL_VERSION)/bin/native-image --no-server -Djava.net.preferIPv4Stack=true HttpServerApp

clean:
	rm -rf *.class httpserverapp upstream
