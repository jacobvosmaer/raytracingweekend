CFLAGS+=-Wall -Werror -pedantic -std=gnu89 -O2 -flto -march=native
LDLIBS+=-lm
OBJS = main vec3.o mt19937.o

all: main.txt
main: vec3.o mt19937.o scalar.o
main.txt: main
	objdump -d $< > $@
debug: CFLAGS += -fsanitize=address
debug: all

clean:
	rm -f -- $(OBJS)
