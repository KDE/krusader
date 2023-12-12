/*
    SPDX-FileCopyrightText: 2017-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    ~SizeCalculator() override;

    /** Return all URLs (queued and progressed). */
    QList<QUrl> urls() const
    {
        return m_urls;
    }
    /** Add a URL to the running calculation. */
    void add(const QUrl &url);
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
    /** Emitted when the calculation starts. */
    void started();
    /** Emitted when an intermediate size for one of the URLs was calculated. */
    void calculated(const QUrl &url, KIO::filesize_t size);
    /** Emitted when calculation is finished or was canceled. Calculator will be deleted afterwards. */
    void finished(bool canceled);
    /** Emitted when the progress changes. */
    void progressChanged(int percentage);

private:
    void nextSubUrl();
    void done();
    void emitProgress();

private slots:
    void start();
    void nextUrl();
    void slotStatResult(KJob *job);
    void slotDirectorySizeResult(KJob *job);

private:
    QList<QUrl> m_urls; // all URLs

    QList<QUrl> m_nextUrls; // URLs not calculated yet
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
