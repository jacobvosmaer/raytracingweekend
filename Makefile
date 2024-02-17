CFLAGS+=-Wall -Werror -pedantic -std=gnu89 -O2 -flto -fno-omit-frame-pointer
LDLIBS+=-lm
OBJS = main

all: main

debug: CFLAGS += -fsanitize=address
debug: all

clean:
	rm -f -- $(OBJS)
