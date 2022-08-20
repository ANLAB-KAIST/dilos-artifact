.PHONY: module test browser_test clean
module: ROOTFS usr.manifest

PUBLIFY_VER=v8.1.1
JSON_VER=1.8.2

sqlite-autoconf-3080500.tar.gz:
	wget "http://www.sqlite.org/2014/sqlite-autoconf-3080500.tar.gz"

sqlite-autoconf-3080500: sqlite-autoconf-3080500.tar.gz
	tar xvf sqlite-autoconf-3080500.tar.gz
sqlite-autoconf-3080500/.libs/libsqlite3.so.0.8.6: sqlite-autoconf-3080500
	cd sqlite-autoconf-3080500 && ./configure && make CFLAGS="-fPIC -DHAVE_MREMAP=0"
	
ROOTFS/libsqlite3.so.0: sqlite-autoconf-3080500/.libs/libsqlite3.so.0.8.6
	cp sqlite-autoconf-3080500/.libs/libsqlite3.so.0.8.6 ROOTFS/libsqlite3.so.0
publify:
	git clone -b ${PUBLIFY_VER} --depth 1 https://github.com/publify/publify.git
	rm publify/config/environments/production.rb
ROOTFS/publify: publify
	mkdir -p ROOTFS/publify
	cd publify && git archive ${PUBLIFY_VER} | tar -x -C ../ROOTFS/publify

ROOTFS/publify/bin/rails:
	mkdir -p ROOTFS/publify/bin
	cp rails ROOTFS/publify/bin

ROOTFS/publify/vendor/bundle: ROOTFS/publify  ROOTFS/publify/bin/rails
	cp database.yml ROOTFS/publify/config/database.yml
	cp development.rb ROOTFS/publify/config/environments/development.rb
	cd ROOTFS/publify && echo "gem 'tzinfo'" >> Gemfile
	cd ROOTFS/publify && echo "gem 'tzinfo-data'" >> Gemfile
	cd ROOTFS/publify && bundle install --standalone
	cd ROOTFS/publify && env GEM_HOME=bundle/ruby/2.2.0 gem install json -v ${JSON_VER}
	cd ROOTFS/publify && bundle exec rake db:create
	cd ROOTFS/publify && bundle exec rake db:migrate
	cd ROOTFS/publify && bundle exec rake db:seed
	cd ROOTFS/publify && bundle exec rake assets:precompile
ROOTFS: ROOTFS/publify/vendor/bundle ROOTFS/libsqlite3.so.0

usr.manifest:
	echo '/publify/**: $${MODULE_DIR}/ROOTFS/publify/**' >> usr.manifest
	echo '/libsqlite3.so.0: $${MODULE_DIR}/sqlite-autoconf-3080500/.libs/libsqlite3.so.0.8.6' >> usr.manifest
	find ROOTFS/publify/bundle -name '*.so' -exec ldd {} \;|awk '{print $$1,":",$$3}'|grep "/lib64"|grep -v "ld-linux"|grep -v "libc.so"|grep -v "libpthread.so"|grep -v "libdl.so"|grep -v "libm.so"|grep -v "libstdc++"|grep -v "libgcc_s"|grep -v "librt"|grep -v "sqlite3"|grep -v "libssl.so"|sort|uniq|sed -e "s/ //" \
		>> usr.manifest

test:
	python test.py

browser_test:
	Xvfb :1 -screen 0 1024x768x24 &
	env DISPLAY=:1 python browser_test.py
	killall Xvfb

clean:
	rm -rf usr.manifest sqlite-autoconf-3080500 *.tar.gz ROOTFS tmp publify
