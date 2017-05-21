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

#ifndef SYNCHRONIZERTASK_H
#define SYNCHRONIZERTASK_H

// QtCore
#include <QFile>
#include <QObject>
#include <QTimer>

#include <KIO/Job>

#include "../FileSystem/krquery.h"

class Synchronizer;
class FileSearcher;
class FileItem;
class SynchronizerFileItem;

#define ST_STATE_NEW 0
#define ST_STATE_PENDING 1
#define ST_STATE_STATUS 2
#define ST_STATE_READY 3
#define ST_STATE_ERROR 4

class SynchronizerTask : public QObject
{
    Q_OBJECT

public:
    SynchronizerTask() : QObject(), m_state(ST_STATE_NEW), m_statusMessage(QString()) {}
    virtual ~SynchronizerTask() {}

    inline int start(QWidget *parentWidget)
    {
        this->parentWidget = parentWidget;
        start();
        return state();
    }

    inline int state() { return m_state; }

    void setStatusMessage(const QString &statMsg)
    {
        if (m_state == ST_STATE_PENDING || m_state == ST_STATE_STATUS)
            m_state = ST_STATE_STATUS;
        m_statusMessage = statMsg;
    }

    QString status()
    {
        if (m_state == ST_STATE_STATUS) {
            m_state = ST_STATE_PENDING;
            return m_statusMessage;
        }
        return QString();
    }

protected:
    virtual void start() {}
    int m_state;
    QString m_statusMessage;
    QWidget *parentWidget;
};

class CompareTask : public SynchronizerTask
{
    Q_OBJECT

public:
    CompareTask(SynchronizerFileItem *parentIn, const KRQuery &query,
                const QUrl &url, const QString &dir, bool isLeft);

    CompareTask(SynchronizerFileItem *parentIn, const KRQuery &query, const QUrl &left,
                const QString &leftDir, const QUrl &right, const QString &rightDir);

    virtual ~CompareTask();

    inline bool hasBothSides() const { return m_bothSides; }
    inline bool hasOnlyLeftSide() const { return !m_bothSides && m_isLeft; }
    inline const QUrl &firstUrl() const { return m_url; }
    inline const QUrl &secondUrl() const { return m_otherUrl; }
    inline const QList<FileItem *> &firstFiles() const { return m_fileList; }
    inline const QList<FileItem *> &secondFiles() const { return m_otherFileList; }
    inline const QString &firstDir() const { return m_dir; }
    inline const QString &secondDir() const { return m_otherDir; }
    inline SynchronizerFileItem *parent() const { return m_parent; }

protected slots:
    virtual void start() Q_DECL_OVERRIDE;
    void slotError(const QUrl &url);
    void slotFinished();
    void slotOtherFinished();

private:
    SynchronizerFileItem *m_parent;
    const KRQuery m_query;
    const QUrl m_url;
    const QUrl m_otherUrl;
    const QString m_dir;
    const QString m_otherDir;
    const bool m_isLeft;
    const bool m_bothSides;

    FileSearcher *m_fileSearcher;
    FileSearcher *m_otherFileSearcher;
    QList<FileItem *> m_fileList;
    QList<FileItem *> m_otherFileList;

    bool m_loadFinished;
    bool m_otherLoadFinished;
};

class CompareContentTask : public SynchronizerTask
{
    Q_OBJECT

public:
    CompareContentTask(Synchronizer *, SynchronizerFileItem *, const QUrl &, const QUrl &,
                       KIO::filesize_t);
    virtual ~CompareContentTask();

public slots:
    void slotDataReceived(KIO::Job *job, const QByteArray &data);
    void slotFinished(KJob *job);
    void sendStatusMessage();

protected:
    virtual void start();

protected slots:
    void localFileCompareCycle();

private:
    void abortContentComparing();

    const QUrl             left;           // the currently processed URL (left)
    const QUrl             right;          // the currently processed URL (right)
    KIO::filesize_t        size;           // the size of the compared files

    bool                   errorPrinted;   // flag indicates error
    KIO::TransferJob      *leftReadJob;    // compare left read job
    KIO::TransferJob      *rightReadJob;   // compare right read job
    QByteArray             compareArray;   // the array for comparing
    int                    owner;          // the owner of the compare array
    SynchronizerFileItem  *item;           // the item for content compare
    QTimer                *timer;          // timer to show the process dialog at compare by content

    QFile                 *leftFile;       // the left side local file
    QFile                 *rightFile;      // the right side local file

    KIO::filesize_t        received;       // the received size
    Synchronizer          *sync;
};

#endif /* __SYNCHRONIZER_TASK_H__ */
