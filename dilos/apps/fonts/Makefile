.PHONY: module clean

module: usr.manifest

usr.manifest:
	cp usr.manifest.skel usr.manifest
	find /usr/share/fonts -type d -name liberation\* | awk '{ printf "%s/**: %s/**\n", $$0, $$0 }' >> usr.manifest
	grep -il liberation /etc/fonts/conf.d/* | awk '{ printf "%s: %s\n", $$0, $$0 }' >> usr.manifest

clean:
	rm usr.manifest
