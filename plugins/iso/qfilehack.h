/*
    SPDX-FileCopyrightText: Krusader Krew <https://krusader.org>
    SPDX-FileCopyrightText: Szombathelyi György <gyurco@users.sourceforge.net>

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
