.PHONY: module
module:
	infocmp xterm | sed 's/cud1=\(^J\|\\n\),/cud1=\\E[B,/; s/ind=\(^J\|\\n\),/ind=\\E[S,/; s/xterm/unknown/' | tic -o terminfo -
clean:
	rm -rf terminfo
