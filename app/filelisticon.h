/*
    SPDX-FileCopyrightText: 2018-2022 Nikita Melnichenko <nikita+kde@melnichenko.name>
    SPDX-FileCopyrightText: 2018-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILELISTICON_H
#define FILELISTICON_H

#include "icon.h"

// QtGui
#include <QPixmap>

class FileListIcon : public Icon
{
public:
    explicit FileListIcon(QString name)
        : Icon(name)
    {
    }

    /// Load pixmap of standard file list icon size
    QPixmap pixmap() const;

    /// Get icon size as configured by user
    QSize size() const;
};

#endif // FILELISTICON_H
