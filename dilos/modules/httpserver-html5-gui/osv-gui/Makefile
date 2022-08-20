TEMPLATE_SRCS=$(shell find osv/templates/ -type f -name '*')
JAVASCRIPT_LIBS=$(shell find lib/ -type f -name '*.js' | sort)
JAVASCRIPT_SRCS=$(shell find osv/ -type f -name '*.js')

LEGACY_MAIN_INDEX=public/dashboard/index.html
MAIN_INDEX=public/index.html
MAIN_JS=public/dashboard_static/out.js
LIB_JS=public/dashboard_static/lib.js

RHINO=bin/js.jar
CLOSURE_COMPILER=bin/compiler.js
RJS=bin/r.js
BUILDJS=build.js

ALL=$(MAIN_INDEX) $(LEGACY_MAIN_INDEX) $(MAIN_JS) $(LIB_JS)

all: $(ALL)

$(LEGACY_MAIN_INDEX): $(TEMPLATE_SRCS)
	mkdir -p public/dashboard;
	scripts/build_index.sh > $@

$(MAIN_INDEX): $(TEMPLATE_SRCS)
	mkdir -p public/dashboard;
	scripts/build_index.sh > $@

$(MAIN_JS): $(JAVASCRIPT_SRCS)
	java -classpath $(RHINO):$(CLOSURE_COMPILER) org.mozilla.javascript.tools.shell.Main $(RJS) -o $(BUILDJS)

$(LIB_JS): $(JAVASCRIPT_LIBS)
	echo "" > $@;
	for LIB in $(JAVASCRIPT_LIBS) ; do cat $$LIB >> $@; done;

clean:
	@rm -f $(ALL)

.PHONY: all clean
