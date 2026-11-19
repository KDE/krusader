/*
    SPDX-FileCopyrightText: 2026 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LOCALDELETEJOB_H
#define LOCALDELETEJOB_H

// QtCore
#include <QList>
#include <QString>
#include <QUrl>

#include <KJob>

/**
 * Delete or trash local files without KIO/kioslave.
 *
 * Using KIO::del / KIO::trash keeps a file/trash ioslave open on the mount and
 * blocks unmounting removable devices until Krusader exits (bugs.kde.org/445549).
 */
class LocalDeleteJob : public KJob
{
    Q_OBJECT
public:
    LocalDeleteJob(const QList<QUrl> &urls, bool moveToTrash, QObject *parent = nullptr);
    ~LocalDeleteJob() override;

    void start() override;

    QList<QUrl> urls() const
    {
        return m_urls;
    }
    bool moveToTrash() const
    {
        return m_moveToTrash;
    }

    /** True if any job URL is on/under the given mount point. */
    bool touchesMountPoint(const QString &mntPoint) const;

    /** Request abort of all active jobs that touch @p mntPoint. */
    static void abortJobsUnderMount(const QString &mntPoint);

protected:
    bool doKill() override;

private:
    void doWork();
    bool deletePath(const QString &path, QString *errorMessage);
    bool trashPath(const QString &path, QString *errorMessage);
    bool copyPathRecursively(const QString &src, const QString &dst, QString *errorMessage);
    bool shouldAbort() const;
    static QString uniqueTrashName(const QString &filesDir, const QString &baseName);
    static void registerJob(LocalDeleteJob *job);
    static void unregisterJob(LocalDeleteJob *job);

    const QList<QUrl> m_urls;
    const bool m_moveToTrash;
    bool m_killed = false;

    static QList<LocalDeleteJob *> s_activeJobs;
};

#endif
