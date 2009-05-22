all: list-elf64-public-symbols

list-elf64-public-symbols: list-elf64-public-symbols.c
	$(CC) `pkg-config --cflags --libs glib-2.0` -o $@ $<
