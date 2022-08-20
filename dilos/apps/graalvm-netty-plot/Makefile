module: netty-plot

GRAAL_VERSION=1.0.0-rc15

netty-plot: upstream/graalvm-demos/native-netty-plot/target/netty-plot-0.1-jar-with-dependencies.jar upstream/graalvm-ce-$(GRAAL_VERSION)
	upstream/graalvm-ce-$(GRAAL_VERSION)/bin/native-image --no-server -Djava.net.preferIPv4Stack=true -jar upstream/graalvm-demos/native-netty-plot/target/netty-plot-0.1-jar-with-dependencies.jar

upstream/graalvm-demos/native-netty-plot/target/netty-plot-0.1-jar-with-dependencies.jar: upstream/graalvm-ce-$(GRAAL_VERSION) upstream/graalvm-demos/.git
	cd upstream/graalvm-demos/native-netty-plot && mvn package

upstream/graalvm-demos/.git:
	mkdir -p upstream
	cd upstream && git clone --depth 1 https://github.com/graalvm/graalvm-demos.git

upstream/graalvm-ce-$(GRAAL_VERSION):
	mkdir -p upstream
	wget -c -O upstream//graalvm-ce-$(GRAAL_VERSION)-linux-amd64.tar.gz https://github.com/oracle/graal/releases/download/vm-$(GRAAL_VERSION)/graalvm-ce-$(GRAAL_VERSION)-linux-amd64.tar.gz
	cd upstream && tar xf graalvm-ce-$(GRAAL_VERSION)-linux-amd64.tar.gz

clean:
	rm -rf netty-plot upstream
