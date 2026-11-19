/*
    SPDX-FileCopyrightText: 2026 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MOUNTPATH_H
#define MOUNTPATH_H

// QtCore
#include <QDir>
#include <QString>
#include <QUrl>

/**
 * Path helpers for mount-point comparisons (canonical, same-or-child).
 * Used when leaving a mount so watches/cwd/jobs do not keep the device busy.
 */
namespace MountPath
{
inline QString canonical(const QString &path)
{
    if (path.isEmpty())
        return {};
    const QString result = QDir(path).canonicalPath();
    return result.isEmpty() ? QDir::cleanPath(path) : result;
}

/** True if @p childPath is the same as @p parentPath or a path under it. */
inline bool isSameOrChild(const QString &parentPath, const QString &childPath)
{
    const QString parent = canonical(parentPath);
    const QString child = canonical(childPath);
    if (parent.isEmpty() || child.isEmpty())
        return false;

    const QUrl parentUrl = QUrl::fromLocalFile(parent);
    const QUrl childUrl = QUrl::fromLocalFile(child);
    return parentUrl == childUrl || parentUrl.isParentOf(childUrl);
}
}

#endif
