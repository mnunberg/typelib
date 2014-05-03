all: libtypelib.so

CPPFLAGS=-Wall -Wextra -fno-strict-aliasing -Wmissing-declarations

libtypelib.so: src/dlist.c src/hashtable.c src/string.c src/nset.c
	$(CC) -Iinclude/typelib -fPIC -shared $(CPPFLAGS) $(CFLAGS) -o $@ $^
