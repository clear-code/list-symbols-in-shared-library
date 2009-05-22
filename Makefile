all: list-elf64-public-function-names

list-elf64-public-function-names: list-elf64-public-function-names.c
	$(CC) `pkg-config --cflags --libs glib-2.0` -o $@ $<
