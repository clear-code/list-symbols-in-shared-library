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
#include <mach-o/loader.h>
#include <mach-o/nlist.h>

int
main (int argc, char **argv)
{
    gchar *content;
    gsize length;
    gsize offset;
    GPtrArray *all_symbols;
    uint32_t i, n_commands;

    if (argc < 2) {
        g_print("Usage: %s XXX.dylib\n", argv[0]);
        return 0;
    }

    g_file_get_contents(argv[1], &content, &length, NULL);

    {
        struct mach_header *header;

        header = (struct mach_header *)content;
        if (header->magic != MH_MAGIC) {
            g_print("not Mach-O 32\n");
            return -1;
        }

        offset = sizeof(*header);
        n_commands = header->ncmds;
    }

    all_symbols = g_ptr_array_sized_new(n_commands);
    for (i = 0; i < n_commands; i++) {
        struct load_command *load;

        load = (struct load_command *)(content + offset);
        switch (load->cmd) {
        case LC_SYMTAB:
        {
            struct symtab_command *table;
            struct nlist *symbol_info = NULL;
            gchar *string_table;
            gint j;

            table = (struct symtab_command *)(content + offset);
            symbol_info = (struct nlist *)(content + table->symoff);
            string_table = content + table->stroff;
            for (j = 0; j < table->nsyms; j++) {
                uint8_t type;
                int32_t string_offset;
                gchar *name;

                type = symbol_info[j].n_type;
                string_offset = symbol_info[j].n_un.n_strx;

                name = string_table + string_offset;
                if ((string_offset == 0) ||
                    (name[0] == '\0') ||
                    (name[0] != '_')) {
                    g_ptr_array_add(all_symbols, NULL);
                } else {
                    g_ptr_array_add(all_symbols, name + 1);
                }
            }
            break;
        }
        case LC_DYSYMTAB:
        {
            struct dysymtab_command *table;
            gint j;

            table = (struct dysymtab_command *)(content + offset);
            for (j = 0; j < table->nextdefsym; j++) {
                const gchar *name;

                name = g_ptr_array_index(all_symbols, table->iextdefsym + j);
                g_print("found: %s\n", name);
            }
            break;
        }
        default:
            break;
        }
        offset += load->cmdsize;
    }
    g_ptr_array_free(all_symbols, TRUE);

    g_free(content);

    return 0;
}
