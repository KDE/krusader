/*****************************************************************************
 * Copyright (C) 2003 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2004-2019 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#include "crc32.h"

#define MASK1       0x00FFFFFF
#define MASK2       0xFFFFFFFF
#define POLYNOMIAL  0xEDB88320

bool            CRC32::crc_initialized = false;
unsigned long   CRC32::crc_table[ 256 ];

CRC32::CRC32(unsigned long initialValue)
{
    crc_accum = initialValue;

    if (!crc_initialized) {
        for (int byte = 0; byte != 256; byte++) {
            unsigned long data = byte;

            for (int i = 8; i > 0 ; --i)
                data = data & 1 ? (data >> 1) ^ POLYNOMIAL : data >> 1;

            crc_table[ byte ] = data;
        }

        crc_initialized = true;
    }
}

void CRC32::update(unsigned char *buffer, int bufferLen)
{
    while (bufferLen-- > 0)
        crc_accum = ((crc_accum >> 8) & MASK1) ^ crc_table[(crc_accum & 0xff) ^ *buffer++ ];
}

unsigned long CRC32::result()
{
    return (~crc_accum) & MASK2;
}
