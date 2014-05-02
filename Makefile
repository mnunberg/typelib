all: libtypelib.so

CPPFLAGS=-Wall -Wextra -fno-strict-aliasing -Wmissing-declarations

libtypelib.so: src/list.c src/genhash.c src/simplestring.c src/hashset.c
	$(CC) -Iinclude/typelib -fPIC -shared $(CPPFLAGS) $(CFLAGS) -o $@ $^
