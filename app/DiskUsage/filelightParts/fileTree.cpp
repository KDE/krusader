/*
    SPDX-FileCopyrightText: 2004 Max Howell <max.howell@methylblue.com>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fileTree.h"

// QtCore
#include <QLocale>
#include <QString>

// static definitions
const FileSize File::DENOMINATOR[4] = {1ull, 1ull << 10, 1ull << 20, 1ull << 30};
const char File::PREFIX[5][2] = {"", "K", "M", "G", "T"};

QString File::fullPath(const Directory *root /*= 0*/) const
{
    QString path;

    if (root == this)
        root = nullptr; // prevent returning empty string when there is something we could return

    const File *d;

    for (d = this; d != root && d && d->parent() != nullptr; d = d->parent()) {
        if (!path.isEmpty())
            path = '/' + path;

        path = d->name() + path;
    }

    if (d) {
        while (d->parent())
            d = d->parent();

        if (d->directory().endsWith('/'))
            return d->directory() + path;
        else
            return d->directory() + '/' + path;
    } else
        return path;
}

QString File::humanReadableSize(UnitPrefix key /*= mega*/) const // FIXME inline
{
    return humanReadableSize(m_size, key);
}

QString File::humanReadableSize(FileSize size, UnitPrefix key /*= mega*/) // static
{
    QString s;
    double prettySize = (double)size / (double)DENOMINATOR[key];
    const QLocale locale;

    if (prettySize >= 0.01) {
        if (prettySize < 1)
            s = locale.toString(prettySize, 'f', 2);
        else if (prettySize < 100)
            s = locale.toString(prettySize, 'f', 1);
        else
            s = locale.toString(prettySize, 'f', 0);

        s += ' ';
        s += PREFIX[key];
        s += 'B';
    }

    if (prettySize < 0.1) {
        s += " (";
        s += locale.toString(size / DENOMINATOR[key ? key - 1 : 0]);
        s += ' ';
        s += PREFIX[key];
        s += "B)";
    }

    return s;
}
