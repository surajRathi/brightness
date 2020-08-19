CC = g++
OPTIONS =
#INSTALL_DIR = "/usr/bin/"
INSTALL_DIR = ~/.local/bin/# trailing backslash req and NO trailing space

all: default

default: compile
	sudo chown root:root brightness
	sudo chmod 6511 brightness

compile:
	$(CC) $(OPTIONS) brightness.cpp -o brightness -lstdc++fs -std=c++17

clean:
	rm -f brightness

install: default
	mv -f brightness $(INSTALL_DIR)brightness # cp requires o+r perm or sudo

uninstall:
	rm -f $(INSTALL_DIR)brightness
