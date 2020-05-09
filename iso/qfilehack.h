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

#ifndef QFILEHACK_H
#define QFILEHACK_H

// QtCore
#include <QFile>
#include <QString>

/**
 * Qt thinks if a file is not S_IFREG, you cannot seek in it.
 * It's false (what about block devices for example ?)
 */
class QFileHack : public QFile
{
public:
    QFileHack();
    explicit QFileHack(const QString & name);
    ~QFileHack();
    virtual bool open(QFile::OpenMode m) override;
};

#endif
