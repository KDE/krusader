/*****************************************************************************
 * Copyright (C) 2006 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2006-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#include "synchronizertask.h"

// QtCore
#include <QFile>
#include <QTimer>

#include <KI18n/KLocalizedString>
#include <KWidgetsAddons/KMessageBox>

#include "synchronizer.h"
#include "synchronizerdirlist.h"
#include "synchronizerfileitem.h"
#include "../FileSystem/filesearcher.h"
#include "../FileSystem/filesystem.h"

CompareTask::CompareTask(SynchronizerFileItem *parentIn, const KRQuery &query, const QUrl &url,
                         const QString &dir, bool isLeft)
    : SynchronizerTask(), m_parent(parentIn), m_query(query), m_url(url), m_dir(dir),
      m_isLeft(isLeft), m_bothSides(false), m_fileSearcher(0), m_otherFileSearcher(0)
{
}

CompareTask::CompareTask(SynchronizerFileItem *parentIn, const KRQuery &query, const QUrl &url,
                         const QString &dir, const QUrl &otherUrl, const QString &otherDir)
    : SynchronizerTask(), m_parent(parentIn), m_query(query), m_url(url), m_otherUrl(otherUrl),
      m_dir(dir), m_otherDir(otherDir), m_isLeft(false), m_bothSides(true), m_fileSearcher(0),
      m_otherFileSearcher(0)
{
}

CompareTask::~CompareTask()
{
    if (m_fileSearcher) {
        delete m_fileSearcher;
        m_fileSearcher = 0;
    }
    if (m_otherFileSearcher) {
        delete m_otherFileSearcher;
        m_otherFileSearcher = 0;
    }
}

void CompareTask::start()
{
    if (m_state != ST_STATE_NEW)
        return;

    m_state = ST_STATE_PENDING;
    m_loadFinished = m_otherLoadFinished = false;

    m_fileSearcher = new FileSearcher(m_query);
    connect(m_fileSearcher, &FileSearcher::finished, this, &CompareTask::slotFinished);
    connect(m_fileSearcher, &FileSearcher::error, this, &CompareTask::slotError);
    m_fileSearcher->start(m_url);

    if (m_bothSides) {
        m_otherFileSearcher = new FileSearcher(m_query);
        connect(m_otherFileSearcher, &FileSearcher::finished, this, &CompareTask::slotOtherFinished);
        connect(m_otherFileSearcher, &FileSearcher::error, this, &CompareTask::slotError);
        m_otherFileSearcher->start(m_otherUrl);
    }
}

void CompareTask::slotError(const QUrl &url)
{
    m_state = ST_STATE_ERROR;
    KMessageBox::error(parentWidget,
                       i18n("Error opening %1.", url.toDisplayString(QUrl::PreferLocalFile)));
}

void CompareTask::slotFinished()
{
    if (m_state == ST_STATE_ERROR)
        return;

    m_loadFinished = true;
    m_fileList = m_fileSearcher->files();

    if (m_otherLoadFinished || !m_bothSides)
        m_state = ST_STATE_READY;
}

void CompareTask::slotOtherFinished()
{
    if (m_state == ST_STATE_ERROR)
        return;

    m_otherLoadFinished = true;
    m_otherFileList = m_otherFileSearcher->files();

    if (m_loadFinished)
        m_state = ST_STATE_READY;
}

CompareContentTask::CompareContentTask(Synchronizer *syn, SynchronizerFileItem *itemIn,
                                       const QUrl &left, const QUrl &right, KIO::filesize_t sizeIn)
    : SynchronizerTask(), left(left), right(right), size(sizeIn),
      errorPrinted(false), leftReadJob(0), rightReadJob(0), compareArray(), owner(-1), item(itemIn),
      timer(0), leftFile(0), rightFile(0), received(0), sync(syn)
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

    if (left.isLocalFile() && right.isLocalFile()) {
        leftFile = new QFile(left.path());
        if (!leftFile->open(QIODevice::ReadOnly)) {
            KMessageBox::error(parentWidget, i18n("Error at opening %1.", left.path()));
            m_state = ST_STATE_ERROR;
            return;
        }

        rightFile = new QFile(right.path());
        if (!rightFile->open(QIODevice::ReadOnly)) {
            KMessageBox::error(parentWidget, i18n("Error at opening %1.", right.path()));
            m_state = ST_STATE_ERROR;
            return;
        }

        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(sendStatusMessage()));
        timer->setSingleShot(true);
        timer->start(1000);

        localFileCompareCycle();
    } else {
        leftReadJob = KIO::get(left, KIO::NoReload, KIO::HideProgressInfo);
        rightReadJob = KIO::get(right, KIO::NoReload, KIO::HideProgressInfo);

        connect(leftReadJob, SIGNAL(data(KIO::Job *, QByteArray)), this,
                SLOT(slotDataReceived(KIO::Job *, QByteArray)));
        connect(rightReadJob, SIGNAL(data(KIO::Job *, QByteArray)), this,
                SLOT(slotDataReceived(KIO::Job *, QByteArray)));
        connect(leftReadJob, SIGNAL(result(KJob *)), this, SLOT(slotFinished(KJob *)));
        connect(rightReadJob, SIGNAL(result(KJob *)), this, SLOT(slotFinished(KJob *)));

        rightReadJob->suspend();

        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(sendStatusMessage()));
        timer->setSingleShot(true);
        timer->start(1000);
    }
}

void CompareContentTask::localFileCompareCycle()
{

    bool different = false;

    char leftBuffer[1440];
    char rightBuffer[1440];

    QTime timer;
    timer.start();

    int cnt = 0;

    while (!leftFile->atEnd() && !rightFile->atEnd()) {
        int leftBytes = leftFile->read(leftBuffer, sizeof(leftBuffer));
        int rightBytes = rightFile->read(rightBuffer, sizeof(rightBuffer));

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

    QTimer::singleShot(0, this, SLOT(localFileCompareCycle()));
}

void CompareContentTask::slotDataReceived(KIO::Job *job, const QByteArray &data)
{
    int jobowner = (job == leftReadJob) ? 1 : 0;
    int bufferLen = compareArray.size();
    int dataLen = data.size();

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

        int minSize = (dataLen < bufferLen) ? dataLen : bufferLen;

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

    if (otherJob == 0) {
        if (compareArray.size())
            abortContentComparing();
    } else {
        if (!((KIO::TransferJob *)job)->isSuspended()) {
            ((KIO::TransferJob *)job)->suspend();
            otherJob->resume();
        }
    }
}

void CompareContentTask::slotFinished(KJob *job)
{
    KIO::TransferJob *otherJob = (job == leftReadJob) ? rightReadJob : leftReadJob;

    if (job == leftReadJob)
        leftReadJob = 0;
    else
        rightReadJob = 0;

    if (otherJob)
        otherJob->resume();

    if (job->error()) {
        timer->stop();
        abortContentComparing();
    }

    if (job->error() && job->error() != KIO::ERR_USER_CANCELED && !errorPrinted) {
        errorPrinted = true;
        KMessageBox::detailedError(parentWidget, i18n("I/O error while comparing file %1 with %2.",
                                                      left.toDisplayString(QUrl::PreferLocalFile),
                                                      right.toDisplayString(QUrl::PreferLocalFile)),
                                   job->errorString());
    }

    if (leftReadJob == 0 && rightReadJob == 0) {
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
    int percent = (int)(perc * 10000. + 0.5);
    QString statstr =
        QString("%1.%2%3").arg(percent / 100).arg((percent / 10) % 10).arg(percent % 10) + '%';
    setStatusMessage(i18n("Comparing file %1 (%2)...", left.fileName(), statstr));
    timer->setSingleShot(true);
    timer->start(500);
}
