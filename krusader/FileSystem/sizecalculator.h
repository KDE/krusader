/*****************************************************************************
 * Copyright (C) 2017 Krusader Krew                                          *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#ifndef SIZECALCULATOR_H
#define SIZECALCULATOR_H

// QtCore
#include <QPointer>
#include <QUrl>

#include <KIO/DirectorySizeJob>

/**
 * Calculate the size of files and directories (recursive).
 *
 * Krusader's virtual filesystem and all KIO protocols are supported.
 *
 * This calculator will delete itself when its finished (like KJob).
 */
class SizeCalculator : public QObject
{
    Q_OBJECT
public:
    /**
     * Calculate the size of files and directories defined by URLs.
     *
     * The calculation is automatically started (like KJob).
     */
    explicit SizeCalculator(const QList<QUrl> &urls);
    ~SizeCalculator();

    /** Return the URLs this calculator was created with. */
    QList<QUrl> urls() const { return m_urls; }
    /** Return the current total size calculation result. */
    KIO::filesize_t totalSize() const;
    /** Return the current total number of files counted. */
    unsigned long totalFiles() const;
    /** Return the current number of directories counted. */
    unsigned long totalDirs() const;

public slots:
    /** Cancel the calculation and mark for deletion. finished() will be emitted.*/
    void cancel();

signals:
    /** Emitted when an intermediate size for one of the URLs was calculated. */
    void calculated(const QUrl &url, KIO::filesize_t size);
    /** Emitted when calculation is finished or was canceled. Calculator will be deleted afterwards. */
    void finished(bool canceled);

private:
    void nextSubUrl();
    void done();

private slots:
    void nextUrl();
    void slotStatResult(KJob *job);
    void slotDirectorySizeResult(KJob *job);

private:
    const QList<QUrl> m_urls;

    QList<QUrl> m_nextUrls;
    QUrl m_currentUrl;
    QList<QUrl> m_nextSubUrls;

    KIO::filesize_t m_currentUrlSize;
    KIO::filesize_t m_totalSize;
    unsigned long m_totalFiles;
    unsigned long m_totalDirs;

    bool m_canceled;

    QPointer<KIO::DirectorySizeJob> m_directorySizeJob;
};

#endif // SIZECALCULATOR_H
