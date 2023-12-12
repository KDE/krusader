/*
    SPDX-FileCopyrightText: 2002 Szombathelyi György <gyurco@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qfilehack.h"

QFileHack::QFileHack() = default;

QFileHack::QFileHack(const QString &name)
    : QFile(name)
{
}

QFileHack::~QFileHack() = default;

bool QFileHack::open(QFile::OpenMode m)
{
    bool ret;

#ifdef Q_OS_UNIX
//    m |= IO_Async; // On linux, set O_NONBLOCK, opens CD-ROMs faster
#endif
    ret = QFile::open(m);
    //    if (ret && isSequential() ) {
    //        setOpenMode(m | (QFile::OpenMode)IO_Direct);
    //    }
    return ret;
}
