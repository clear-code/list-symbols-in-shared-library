/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <elf.h>

int
main (int argc, char **argv)
{
    gchar *content;
    gsize length;
    Elf64_Shdr *dynstr = NULL;
    Elf64_Shdr *dynsym = NULL;
    uint16_t text_section_header_index = 0;

    if (argc < 2) {
        g_print("Usage: %s XXX.so\n", argv[0]);
        return 0;
    }

    g_file_get_contents(argv[1], &content, &length, NULL);

    {
        Elf64_Ehdr *header;
        gsize section_offset;
        uint16_t section_header_size;
        uint16_t i, n_headers;
        Elf64_Shdr *section_name_header;
        gsize section_name_header_offset;
        const gchar *section_names;

        header = (Elf64_Ehdr *)content;
        if (memcmp(header->e_ident, ELFMAG, SELFMAG) != 0) {
            g_print("not ELF\n");
            return -1;
        }

        header = (Elf64_Ehdr *)content;
        if (header->e_type != ET_DYN) {
            g_print("not shared library\n");
            return -1;
        }

        section_offset = header->e_shoff;
        section_header_size = header->e_shentsize;
        n_headers = header->e_shnum;

        section_name_header_offset =
            header->e_shoff +
            (header->e_shstrndx * header->e_shentsize);
        section_name_header =
            (Elf64_Shdr *)(content + section_name_header_offset);
        section_names = content + section_name_header->sh_offset;

        for (i = 0; i < n_headers; i++) {
            Elf64_Shdr *section_header = NULL;
            gsize offset;
            const gchar *section_name;

            offset = section_offset + (section_header_size * i);
            section_header = (Elf64_Shdr *)(content + offset);
            section_name = section_names + section_header->sh_name;

            if (g_str_equal(section_name, ".dynstr")) {
                dynstr = section_header;
            } else if (g_str_equal(section_name, ".dynsym")) {
                dynsym = section_header;
            } else if (g_str_equal(section_name, ".text")) {
                text_section_header_index = i;
            }
        }

        if (!dynsym) {
            g_print(".dynsym section is not found\n");
            return -1;
        }

        if (!dynstr) {
            g_print(".dynstr section is not found\n");
            return -1;
        }

        if (text_section_header_index == 0) {
            g_print(".text section is not found\n");
            return -1;
        }
    }

    {
        guint i, n_entries;
        gsize symbol_section_offset;
        gsize symbol_entry_size;
        gsize name_section_offset;

        symbol_section_offset = dynsym->sh_offset;
        symbol_entry_size = dynsym->sh_entsize;
        name_section_offset = dynstr->sh_offset;
        if (symbol_entry_size > 0)
            n_entries = dynsym->sh_size / symbol_entry_size;
        else
            n_entries = 0;

        for (i = 0; i < n_entries; i++) {
            Elf64_Sym *symbol;
            uint64_t name_index;
            unsigned char info;
            uint16_t section_header_index;
            gsize offset;

            offset = symbol_section_offset + (i * symbol_entry_size);
            symbol = (Elf64_Sym *)(content + offset);
            name_index = symbol->st_name;
            info = symbol->st_info;
            section_header_index = symbol->st_shndx;

            if ((info & STT_FUNC) &&
                (ELF64_ST_BIND(info) & STB_GLOBAL) &&
                (section_header_index == text_section_header_index)) {
                const gchar *name;

                name = content + name_section_offset + name_index;
                g_print("found: %s\n", name);
            }
        }
    }

    g_free(content);

    return 0;
}
