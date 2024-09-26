/*
    SPDX-FileCopyrightText: 2003 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CRC32_H
#define CRC32_H

#include <QtGlobal>

class CRC32
{
private:
    unsigned long crc_accum;
    static unsigned long crc_table[256];
    static bool crc_initialized;

public:
    explicit CRC32(unsigned long initialValue = (unsigned long)-1);

    void update(unsigned char *buffer, qsizetype bufferLen);
    unsigned long result();
};

#endif /* __CRC32_H__ */
