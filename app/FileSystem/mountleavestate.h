/*
    SPDX-FileCopyrightText: 2026 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MOUNTLEAVESTATE_H
#define MOUNTLEAVESTATE_H

// QtCore
#include <QDir>
#include <QSet>
#include <QString>

#include "mountpath.h"

/**
 * Process-wide guard while a mount is being released for unmount.
 * Lives in FileSystem so both panels and MountMan can use it without a link cycle.
 */
class MountLeaveState
{
public:
    static MountLeaveState &instance()
    {
        static MountLeaveState state;
        return state;
    }

    void begin(const QString &mntPoint)
    {
        const QString canonical = MountPath::canonical(mntPoint);
        if (!canonical.isEmpty())
            m_leaving.insert(canonical);
    }

    void end(const QString &mntPoint)
    {
        const QString canonical = MountPath::canonical(mntPoint);
        if (!canonical.isEmpty())
            m_leaving.remove(canonical);
    }

    void clear()
    {
        m_leaving.clear();
    }

    bool contains(const QString &canonicalMnt) const
    {
        return m_leaving.contains(canonicalMnt);
    }

    bool isLeavingPath(const QString &path) const
    {
        if (path.isEmpty() || m_leaving.isEmpty())
            return false;
        for (const QString &mnt : m_leaving) {
            if (MountPath::isSameOrChild(mnt, path))
                return true;
        }
        return false;
    }

    void setCwdIfNotLeaving(const QString &path)
    {
        if (path.isEmpty() || isLeavingPath(path))
            QDir::setCurrent(QDir::homePath());
        else
            QDir::setCurrent(path);
    }

private:
    MountLeaveState() = default;
    QSet<QString> m_leaving;
};

#endif
