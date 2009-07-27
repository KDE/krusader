/***************************************************************************
                          crc32.cpp  -  description
                             -------------------
    copyright            : (C) 2003 by Csaba Karai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
