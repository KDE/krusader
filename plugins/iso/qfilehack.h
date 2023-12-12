/*
    SPDX-FileCopyrightText: 2002 Szombathelyi György <gyurco@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    explicit QFileHack(const QString &name);
    ~QFileHack();
    virtual bool open(QFile::OpenMode m) override;
};

#endif
