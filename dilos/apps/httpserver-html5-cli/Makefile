src = $(shell readlink -f ../..)
app-dir = $(src)/apps/httpserver-html5-cli

all: module
module: html5-cli

html5-cli: osv-html5-terminal/.git
	cd $(app-dir) && \
	(command -v node > /dev/null 2>&1 || (echo "Node not installed. Install node (https://nodejs.org/en/download/package-manager/) and try again" && exit 1)) && \
	cd osv-html5-terminal && \
	npm install && \
	node_modules/grunt-cli/bin/grunt
	
.PHONY: html5-cli

osv-html5-terminal/.git:
	cd $(app-dir) && \
	git clone --depth 1 https://github.com/wkozaczuk/osv-html5-terminal.git

clean:
	cd $(app-dir) && rm -rf osv-html5-terminal

.SECONDARY:
