/*
    SPDX-FileCopyrightText: 2017-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "sizecalculator.h"

// QtCore
#include <QDebug>
#include <QTimer>

#include <KIO/StatJob>

#include "fileitem.h"
#include "virtualfilesystem.h"

SizeCalculator::SizeCalculator(const QList<QUrl> &urls)
    : QObject(nullptr)
    , m_urls(urls)
    , m_nextUrls(urls)
    , m_totalSize(0)
    , m_totalFiles(0)
    , m_totalDirs(0)
    , m_canceled(false)
    , m_directorySizeJob(nullptr)
{
    QTimer::singleShot(0, this, &SizeCalculator::start);
}

SizeCalculator::~SizeCalculator()
{
    if (m_directorySizeJob)
        m_directorySizeJob->kill();
}

void SizeCalculator::start()
{
    emit started();
    nextUrl();
}

void SizeCalculator::add(const QUrl &url)
{
    m_urls.append(url);
    m_nextUrls.append(url);
    emitProgress();
}

KIO::filesize_t SizeCalculator::totalSize() const
{
    return m_totalSize + (m_directorySizeJob ? m_directorySizeJob->totalSize() : 0);
}

unsigned long SizeCalculator::totalFiles() const
{
    return m_totalFiles + (m_directorySizeJob ? m_directorySizeJob->totalFiles() : 0);
}

unsigned long SizeCalculator::totalDirs() const
{
    return m_totalDirs + (m_directorySizeJob ? m_directorySizeJob->totalSubdirs() : 0);
}

void SizeCalculator::cancel()
{
    m_canceled = true;
    if (m_directorySizeJob)
        m_directorySizeJob->kill();

    done();
}

void SizeCalculator::nextUrl()
{
    if (m_canceled)
        return;

    emitProgress();

    if (!m_currentUrl.isEmpty())
        emit calculated(m_currentUrl, m_currentUrlSize);
    m_currentUrlSize = 0;

    if (m_nextUrls.isEmpty()) {
        done();
        return;
    }
    m_currentUrl = m_nextUrls.takeFirst();

    if (m_currentUrl.scheme() == "virt") {
        // calculate size of all files/directories in this virtual directory
        auto *fs = new VirtualFileSystem();
        if (!fs->scanDir(m_currentUrl)) {
            qWarning() << "cannot scan virtual FS, URL=" << m_currentUrl.toDisplayString();
            nextUrl();
            return;
        }
        const QList<FileItem *> fileItems = fs->fileItems();
        for (FileItem *file : fileItems)
            m_nextSubUrls << file->getUrl();
        delete fs;
    } else {
        m_nextSubUrls << m_currentUrl;
    }

    nextSubUrl();
}

void SizeCalculator::nextSubUrl()
{
    if (m_canceled)
        return;

    if (m_nextSubUrls.isEmpty()) {
        nextUrl();
        return;
    }

    const QUrl url = m_nextSubUrls.takeFirst();
    KIO::StatJob *statJob = KIO::stat(url, KIO::HideProgressInfo);
    connect(statJob, &KIO::Job::result, this, &SizeCalculator::slotStatResult);
}

void SizeCalculator::slotStatResult(KJob *job)
{
    if (m_canceled)
        return;

    if (job->error()) {
        qWarning() << "stat job failed, error=" << job->error() << "; error string=" << job->errorString();
        nextSubUrl();
        return;
    }

    const KIO::StatJob *statJob = dynamic_cast<KIO::StatJob *>(job);
    const QUrl &url = statJob->url();

    const KFileItem kfi(statJob->statResult(), url, true);
    if (kfi.isFile() || kfi.isLink()) {
        m_totalFiles++;
        m_totalSize += kfi.size();
        m_currentUrlSize += kfi.size();
        nextSubUrl();
        return;
    }

    // URL should be a directory, we are always counting the directory itself
    m_totalDirs++;

    m_directorySizeJob = KIO::directorySize(url);
    connect(m_directorySizeJob.data(), &KIO::Job::result, this, &SizeCalculator::slotDirectorySizeResult);
}

void SizeCalculator::slotDirectorySizeResult(KJob *)
{
    if (!m_directorySizeJob->error()) {
        m_totalSize += m_directorySizeJob->totalSize();
        // do not count filesystem size of empty directories for this current directory
        m_currentUrlSize += m_directorySizeJob->totalFiles() == 0 ? 0 : m_directorySizeJob->totalSize();
        m_totalFiles += m_directorySizeJob->totalFiles();
        m_totalDirs += m_directorySizeJob->totalSubdirs();
    }
    m_directorySizeJob = nullptr;
    nextSubUrl();
}

void SizeCalculator::done()
{
    emit finished(m_canceled);
    deleteLater();
}

void SizeCalculator::emitProgress()
{
    emit progressChanged(static_cast<int>(((m_urls.length() - m_nextUrls.length()) * 100) / m_urls.length()));
}
