/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 * From the linux kernel                                                     *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#ifndef EL_TORITO_H
#define EL_TORITO_H

#include "iso_fs.h"

#define EL_TORITO_ID "EL TORITO SPECIFICATION\0\0\0\0\0\0\0\0\0"

struct el_torito_boot_descriptor {
    char type [ISODCL(1, 1)];   /* 711 */
    char id    [ISODCL(2,   6)];
    char version   [ISODCL(7,   7)];    /* 711 */
    char system_id   [ISODCL(8,  39)];    /* achars */
    char unused    [ISODCL(40,  71)];
    char boot_catalog  [ISODCL(72,  75)];   /* 731 */
};

struct validation_entry {
    char type  [ISODCL(1,  1)];    /* 1 */
    char platform  [ISODCL(2,  2)];
    char unused  [ISODCL(3,  4)];
    char id   [ISODCL(5, 28)];
    char cheksum [ISODCL(29, 30)];
    char key  [ISODCL(31, 31)];   /* 0x55 */
    char key2  [ISODCL(32, 32)];   /* 0xaa */
};

struct default_entry {
    char bootid  [ISODCL(1,  1)];
    char media  [ISODCL(2,  2)];
    char loadseg [ISODCL(3,  4)];
    char systype [ISODCL(5,  5)];
    char unused  [ISODCL(6,  6)];
    char seccount [ISODCL(7,  8)];
    char start  [ISODCL(9, 12)];
    char unused2 [ISODCL(13, 32)];
};

struct section_header {
    char headerid [ISODCL(1,  1)];
    char platform [ISODCL(2,  2)];
    char entries [ISODCL(3,  4)];
    char id   [ISODCL(5, 32)];
};

struct section_entry {
    char bootid  [ISODCL(1,  1)];
    char media  [ISODCL(2,  2)];
    char loadseg [ISODCL(3,  4)];
    char systype [ISODCL(5,  5)];
    char unused  [ISODCL(6,  6)];
    char seccount [ISODCL(7,  8)];
    char start  [ISODCL(9, 12)];
    char selcrit [ISODCL(13, 13)];
    char vendor_selcrit [ISODCL(14, 32)];
};

struct section_entry_ext {
    char extid  [ISODCL(1,  1)];
    char extrec  [ISODCL(2,  2)];
    char vendor_selcrit [ISODCL(3,  32)];
};

#endif
