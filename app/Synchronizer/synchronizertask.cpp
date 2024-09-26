/*
    SPDX-FileCopyrightText: 2006 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "synchronizertask.h"

// QtCore
#include <QElapsedTimer>
#include <QFile>
#include <QTimer>

#include <KLocalizedString>
#include <KMessageBox>

#include "../FileSystem/filesystem.h"
#include "synchronizer.h"
#include "synchronizerdirlist.h"
#include "synchronizerfileitem.h"

CompareTask::CompareTask(SynchronizerFileItem *parentIn,
                         const QString &leftURL,
                         const QString &rightURL,
                         const QString &leftDir,
                         const QString &rightDir,
                         bool hidden)
    : m_parent(parentIn)
    , m_url(leftURL)
    , m_dir(leftDir)
    , m_otherUrl(rightURL)
    , m_otherDir(rightDir)
    , m_duplicate(true)
    , m_dirList(nullptr)
    , m_otherDirList(nullptr)
{
    ignoreHidden = hidden;
}

CompareTask::CompareTask(SynchronizerFileItem *parentIn, const QString &urlIn, const QString &dirIn, bool isLeftIn, bool hidden)
    : m_parent(parentIn)
    , m_url(urlIn)
    , m_dir(dirIn)
    , m_isLeft(isLeftIn)
    , m_duplicate(false)
    , m_dirList(nullptr)
    , m_otherDirList(nullptr)
{
    ignoreHidden = hidden;
}

CompareTask::~CompareTask()
{
    if (m_dirList) {
        delete m_dirList;
        m_dirList = nullptr;
    }
    if (m_otherDirList) {
        delete m_otherDirList;
        m_otherDirList = nullptr;
    }
}

void CompareTask::start()
{
    if (m_state == ST_STATE_NEW) {
        m_state = ST_STATE_PENDING;
        m_loadFinished = m_otherLoadFinished = false;

        m_dirList = new SynchronizerDirList(parentWidget, ignoreHidden);
        connect(m_dirList, &SynchronizerDirList::finished, this, &CompareTask::slotFinished);
        m_dirList->load(m_url, false);

        if (m_duplicate) {
            m_otherDirList = new SynchronizerDirList(parentWidget, ignoreHidden);
            connect(m_otherDirList, &SynchronizerDirList::finished, this, &CompareTask::slotOtherFinished);
            m_otherDirList->load(m_otherUrl, false);
        }
    }
}

void CompareTask::slotFinished(bool result)
{
    if (!result) {
        m_state = ST_STATE_ERROR;
        return;
    }
    m_loadFinished = true;

    if (m_otherLoadFinished || !m_duplicate)
        m_state = ST_STATE_READY;
}

void CompareTask::slotOtherFinished(bool result)
{
    if (!result) {
        m_state = ST_STATE_ERROR;
        return;
    }
    m_otherLoadFinished = true;

    if (m_loadFinished)
        m_state = ST_STATE_READY;
}

CompareContentTask::CompareContentTask(Synchronizer *syn, SynchronizerFileItem *itemIn, const QUrl &leftURLIn, const QUrl &rightURLIn, KIO::filesize_t sizeIn)
    : leftURL(leftURLIn)
    , rightURL(rightURLIn)
    , size(sizeIn)
    , errorPrinted(false)
    , leftReadJob(nullptr)
    , rightReadJob(nullptr)
    , compareArray()
    , owner(-1)
    , item(itemIn)
    , timer(nullptr)
    , leftFile(nullptr)
    , rightFile(nullptr)
    , received(0)
    , sync(syn)
{
}

CompareContentTask::~CompareContentTask()
{
    abortContentComparing();

    if (timer)
        delete timer;
    if (leftFile)
        delete leftFile;
    if (rightFile)
        delete rightFile;
}

void CompareContentTask::start()
{
    m_state = ST_STATE_PENDING;

    if (leftURL.isLocalFile() && rightURL.isLocalFile()) {
        leftFile = new QFile(leftURL.path());
        if (!leftFile->open(QIODevice::ReadOnly)) {
            KMessageBox::error(parentWidget, i18n("Error at opening %1.", leftURL.path()));
            m_state = ST_STATE_ERROR;
            return;
        }

        rightFile = new QFile(rightURL.path());
        if (!rightFile->open(QIODevice::ReadOnly)) {
            KMessageBox::error(parentWidget, i18n("Error at opening %1.", rightURL.path()));
            m_state = ST_STATE_ERROR;
            return;
        }

        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &CompareContentTask::sendStatusMessage);
        timer->setSingleShot(true);
        timer->start(1000);

        localFileCompareCycle();
    } else {
        leftReadJob = KIO::get(leftURL, KIO::NoReload, KIO::HideProgressInfo);
        rightReadJob = KIO::get(rightURL, KIO::NoReload, KIO::HideProgressInfo);

        connect(leftReadJob, &KIO::TransferJob::data, this, &CompareContentTask::slotDataReceived);
        connect(rightReadJob, &KIO::TransferJob::data, this, &CompareContentTask::slotDataReceived);
        connect(leftReadJob, &KIO::TransferJob::result, this, &CompareContentTask::slotFinished);
        connect(rightReadJob, &KIO::TransferJob::result, this, &CompareContentTask::slotFinished);

        rightReadJob->suspend();

        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &CompareContentTask::sendStatusMessage);
        timer->setSingleShot(true);
        timer->start(1000);
    }
}

void CompareContentTask::localFileCompareCycle()
{
    bool different = false;

    char leftBuffer[1440];
    char rightBuffer[1440];

    QElapsedTimer timer;
    timer.start();

    int cnt = 0;

    while (!leftFile->atEnd() && !rightFile->atEnd()) {
        int leftBytes = static_cast<int>(leftFile->read(leftBuffer, sizeof(leftBuffer)));
        int rightBytes = static_cast<int>(rightFile->read(rightBuffer, sizeof(rightBuffer)));

        if (leftBytes != rightBytes) {
            different = true;
            break;
        }

        if (leftBytes <= 0)
            break;

        received += leftBytes;

        if (memcmp(leftBuffer, rightBuffer, leftBytes)) {
            different = true;
            break;
        }

        if ((++cnt % 16) == 0 && timer.elapsed() >= 250)
            break;
    }

    if (different) {
        sync->compareContentResult(item, false);
        m_state = ST_STATE_READY;
        return;
    }

    if (leftFile->atEnd() && rightFile->atEnd()) {
        sync->compareContentResult(item, true);
        m_state = ST_STATE_READY;
        return;
    }

    QTimer::singleShot(0, this, &CompareContentTask::localFileCompareCycle);
}

void CompareContentTask::slotDataReceived(KIO::Job *job, const QByteArray &data)
{
    int jobowner = (job == leftReadJob) ? 1 : 0;
    qsizetype bufferLen = compareArray.size();
    qsizetype dataLen = data.size();

    if (job == leftReadJob)
        received += dataLen;

    if (jobowner == owner) {
        compareArray.append(data);
        return;
    }

    do {
        if (bufferLen == 0) {
            compareArray = QByteArray(data.data(), dataLen);
            owner = jobowner;
            break;
        }

        qsizetype minSize = (dataLen < bufferLen) ? dataLen : bufferLen;

        for (int i = 0; i != minSize; i++)
            if (data[i] != compareArray[i]) {
                abortContentComparing();
                return;
            }

        if (minSize == bufferLen) {
            compareArray = QByteArray(data.data() + bufferLen, dataLen - bufferLen);
            if (dataLen == bufferLen) {
                owner = -1;
                return;
            }
            owner = jobowner;
            break;
        } else {
            compareArray = QByteArray(compareArray.data() + dataLen, bufferLen - dataLen);
            return;
        }

    } while (false);

    KIO::TransferJob *otherJob = (job == leftReadJob) ? rightReadJob : leftReadJob;

    if (otherJob == nullptr) {
        if (compareArray.size())
            abortContentComparing();
    } else {
        if (!(qobject_cast<KIO::TransferJob *>(job))->isSuspended()) {
            (qobject_cast<KIO::TransferJob *>(job))->suspend();
            otherJob->resume();
        }
    }
}

void CompareContentTask::slotFinished(KJob *job)
{
    KIO::TransferJob *otherJob = (job == leftReadJob) ? rightReadJob : leftReadJob;

    if (job == leftReadJob)
        leftReadJob = nullptr;
    else
        rightReadJob = nullptr;

    if (otherJob)
        otherJob->resume();

    if (job->error()) {
        timer->stop();
        abortContentComparing();
    }

    if (job->error() && job->error() != KIO::ERR_USER_CANCELED && !errorPrinted) {
        errorPrinted = true;
        KMessageBox::error(parentWidget,
                           i18n("I/O error while comparing file %1 with %2.",
                                leftURL.toDisplayString(QUrl::PreferLocalFile),
                                rightURL.toDisplayString(QUrl::PreferLocalFile)));
    }

    if (leftReadJob == nullptr && rightReadJob == nullptr) {
        if (!compareArray.size())
            sync->compareContentResult(item, true);
        else
            sync->compareContentResult(item, false);

        m_state = ST_STATE_READY;
    }
}

void CompareContentTask::abortContentComparing()
{
    if (timer)
        timer->stop();

    if (leftReadJob)
        leftReadJob->kill(KJob::EmitResult);
    if (rightReadJob)
        rightReadJob->kill(KJob::EmitResult);

    if (item->task() >= TT_UNKNOWN)
        sync->compareContentResult(item, false);

    m_state = ST_STATE_READY;
}

void CompareContentTask::sendStatusMessage()
{
    double perc = (size == 0) ? 1. : (double)received / (double)size;
    auto percent = (int)(perc * 10000. + 0.5);
    QString statstr = QString("%1.%2%3").arg(percent / 100).arg((percent / 10) % 10).arg(percent % 10) + '%';
    setStatusMessage(i18n("Comparing file %1 (%2)...", leftURL.fileName(), statstr));
    timer->setSingleShot(true);
    timer->start(500);
}
