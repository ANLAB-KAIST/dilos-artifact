.PHONY: module
module: usr.manifest

sample_codes/twitter/Gemfile.lock:
	cd sample_codes/twitter && bundle install --path vendor/bundle

usr.manifest:
	echo '/ruby-example/**: $${MODULE_DIR}/sample_codes/**' >> usr.manifest
	find sample_codes/twitter -name '*.so' -exec ldd {} \;|awk '{print $$1,":",$$3}'|grep "/lib"|grep -v "ld-linux"|grep -v "libc.so"|grep -v "libpthread.so"|grep -v "libdl.so"|grep -v "libm.so"|grep -v "libruby"|grep -v "libstdc++"|grep -v "libgcc_s"|grep -v "librt"|sort|uniq|sed -e "s/ //" \
		>> usr.manifest

clean:
	rm -f usr.manifest
