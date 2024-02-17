CFLAGS+=-Wall -Werror -pedantic -std=gnu89 -O2 -flto -fsanitize=address -fno-omit-frame-pointer
LDLIBS+=-lm

all: main
main: shared.o camera.o
camera.o: camera.h
