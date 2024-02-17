CFLAGS+=-Wall -Werror -pedantic -std=gnu89 -O2 -fsanitize=address -fno-omit-frame-pointer
LDLIBS+=-lm

all: main
