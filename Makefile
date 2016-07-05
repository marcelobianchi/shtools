
all:
	@make -C sh2sac
	@make -C sac2sh

install: all
	install bin/sac2sh /usr/local/bin
	install bin/sh2sac /usr/local/bin

clean:
	@make -C sh2sac clean
	@make -C sac2sh clean
	