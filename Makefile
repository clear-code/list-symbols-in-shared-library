all: list-elf64-public-function-names \
	list-mach-o-32-public-function-names \
	list-pe-public-function-names.exe

list-elf64-public-function-names: list-elf64-public-function-names.c
	$(CC) -Wall `pkg-config --cflags --libs glib-2.0` -o $@ $<

list-mach-o-32-public-function-names: list-mach-o-32-public-function-names.c
	$(CC) -Wall `pkg-config --cflags --libs glib-2.0` -o $@ $<

list-pe-public-function-names.exe: list-pe-public-function-names.c
	i586-mingw32msvc-$(CC) -Wall -o $@ $<
