
.PHONY: all tools

all: tools

tools:
	@make -C tools/bin2hex -f Makefile TARGET=linux
	@make -C tools/fwcrc -f Makefile TARGET=linux
	@make -C tools/apphdr -f Makefile TARGET=linux

clean:
	@make -C tools/bin2hex -f Makefile TARGET=linux clean
	@make -C tools/fwcrc -f Makefile TARGET=linux clean
	@make -C tools/apphdr -f Makefile TARGET=linux clean

