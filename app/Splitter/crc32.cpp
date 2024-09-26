/*
    SPDX-FileCopyrightText: 2003 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "crc32.h"

#define MASK1 0x00FFFFFF
#define MASK2 0xFFFFFFFF
#define POLYNOMIAL 0xEDB88320

bool CRC32::crc_initialized = false;
unsigned long CRC32::crc_table[256];

CRC32::CRC32(unsigned long initialValue)
{
    crc_accum = initialValue;

    if (!crc_initialized) {
        for (int byte = 0; byte != 256; byte++) {
            unsigned long data = byte;

            for (int i = 8; i > 0; --i)
                data = data & 1 ? (data >> 1) ^ POLYNOMIAL : data >> 1;

            crc_table[byte] = data;
        }

        crc_initialized = true;
    }
}

void CRC32::update(unsigned char *buffer, qsizetype bufferLen)
{
    while (bufferLen-- > 0)
        crc_accum = ((crc_accum >> 8) & MASK1) ^ crc_table[(crc_accum & 0xff) ^ *buffer++];
}

unsigned long CRC32::result()
{
    return (~crc_accum) & MASK2;
}
