CFLAGS+=-Wall -Werror -pedantic -std=gnu89 -O2 -flto -fsanitize=address -fno-omit-frame-pointer
LDLIBS+=-lm
OBJS = main

all: main
clean:
	rm -f -- $(OBJS)
