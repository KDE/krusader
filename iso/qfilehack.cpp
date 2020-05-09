/*****************************************************************************
 * Copyright (C) 2002 Szombathelyi György <gyurco@users.sourceforge.net>     *
 * Copyright (C) 2004-2020 Krusader Krew [https://krusader.org]              *
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

#include "qfilehack.h"

QFileHack::QFileHack()
= default;

QFileHack::QFileHack(const QString & name) : QFile(name)
{
}

QFileHack::~QFileHack()
= default;

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
