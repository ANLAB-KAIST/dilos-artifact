.PHONY: module test clean
module: ROOTFS usr.manifest

RAILS_VER=4.1.7
JSON_VER=1.8.2

sqlite-autoconf-3080500.tar.gz:
	wget "http://www.sqlite.org/2014/sqlite-autoconf-3080500.tar.gz"

sqlite-autoconf-3080500: sqlite-autoconf-3080500.tar.gz
	tar xvf sqlite-autoconf-3080500.tar.gz
sqlite-autoconf-3080500/.libs/libsqlite3.so.0.8.6: sqlite-autoconf-3080500
	cd sqlite-autoconf-3080500 && ./configure && make CFLAGS="-fPIC -DHAVE_MREMAP=0"
	
ROOTFS/libsqlite3.so.0: sqlite-autoconf-3080500/.libs/libsqlite3.so.0.8.6
	cp sqlite-autoconf-3080500/.libs/libsqlite3.so.0.8.6 ROOTFS/libsqlite3.so.0
tmp/bin/rails: 
	GEM_HOME=tmp PATH=tmp/bin:$$PATH gem install --no-ri --no-rdoc rails -v ${RAILS_VER}
	GEM_HOME=tmp PATH=tmp/bin:$$PATH gem install --no-ri --no-rdoc railties -v ${RAILS_VER}
ROOTFS/osv_test: tmp/bin/rails
	mkdir -p ROOTFS
	env GEM_HOME=tmp PATH=tmp/bin:$$PATH tmp/bin/rails new ROOTFS/osv_test
	mv ROOTFS/osv_test/Gemfile ROOTFS/osv_test/Gemfile.old
	grep -v spring ROOTFS/osv_test/Gemfile.old > ROOTFS/osv_test/Gemfile
	echo "gem 'therubyracer',  platforms: :ruby" >> ROOTFS/osv_test/Gemfile
	echo "gem 'tzinfo'" >> ROOTFS/osv_test/Gemfile
	echo "gem 'tzinfo-data'" >> ROOTFS/osv_test/Gemfile
	cd ROOTFS/osv_test && bundle install --standalone
	cd ROOTFS/osv_test && env GEM_HOME=bundle/ruby/2.2.0 gem install json -v ${JSON_VER}
	cd ROOTFS/osv_test && bundle exec rails g scaffold item name:string price:integer description:text
	cd ROOTFS/osv_test && bundle exec rake db:migrate

ROOTFS: ROOTFS/osv_test ROOTFS/libsqlite3.so.0

usr.manifest:
	echo '/osv_test/**: $${MODULE_DIR}/ROOTFS/osv_test/**' >> usr.manifest
	echo '/libsqlite3.so.0: $${MODULE_DIR}/sqlite-autoconf-3080500/.libs/libsqlite3.so.0.8.6' >> usr.manifest
	find ROOTFS/osv_test/bundle -name '*.so' -exec ldd {} \;|awk '{print $$1,":",$$3}'|grep "/lib64"|grep -v "ld-linux"|grep -v "libc.so"|grep -v "libpthread.so"|grep -v "libdl.so"|grep -v "libm.so"|grep -v "libstdc++"|grep -v "libgcc_s"|grep -v "librt"|grep -v "sqlite3"|grep -v "libssl"|sort|uniq|sed -e "s/ //" \
		>> usr.manifest

test:
	python test.py

clean:
	rm -rf usr.manifest sqlite-autoconf-3080500 *.tar.gz ROOTFS tmp
