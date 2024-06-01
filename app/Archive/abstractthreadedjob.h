/*
    SPDX-FileCopyrightText: 2009 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2009-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ABSTRACTTHREADEDJOB_H
#define ABSTRACTTHREADEDJOB_H

// QtCore
#include <QEvent>
#include <QEventLoop>
#include <QList>
#include <QMimeDatabase>
#include <QMutex>
#include <QStack>
#include <QThread>
#include <QTime>
#include <QUrl>
#include <QVariant>
#include <QWaitCondition>

#include <KIO/Job>
#include <KIO/Global>

class AbstractJobThread;
class QTemporaryDir;
class UserEvent;
class KrArcObserver;
class QTemporaryFile;

class AbstractThreadedJob : public KIO::Job
{
    friend class AbstractJobThread;

    Q_OBJECT

protected:
    AbstractThreadedJob();

    void addEventResponse(QList<QVariant> *obj);
    QList<QVariant> *getEventResponse(UserEvent *event);
    void sendEvent(UserEvent *event);

    virtual ~AbstractThreadedJob();
    virtual bool event(QEvent *) override;
    virtual void startAbstractJobThread(AbstractJobThread *);
    virtual bool doSuspend() override
    {
        return false;
    }

protected slots:
    void slotDownloadResult(KJob *);
    void slotProcessedAmount(KJob *, KJob::Unit, qulonglong);
    void slotTotalAmount(KJob *, KJob::Unit, qulonglong);
    void slotSpeed(KJob *, unsigned long);
    void slotDescription(KJob *job, const QString &title, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2);

public:
    QMutex _locker;
    QWaitCondition _waiter;
    QStack<QList<QVariant> *> _stack;
    QString _title;
    qulonglong _maxProgressValue;
    qulonglong _currentProgress;
    QTime _time;
    bool _exiting;

private:
    AbstractJobThread *_jobThread;
};

class AbstractJobThread : public QThread
{
    friend class AbstractThreadedJob;
    friend class AbstractJobObserver;
    Q_OBJECT

public:
    AbstractJobThread();
    virtual ~AbstractJobThread();

    void abort();
    KrArcObserver *observer();

protected slots:
    virtual void slotStart() = 0;

protected:
    virtual void run() override;
    void setJob(AbstractThreadedJob *job)
    {
        _job = job;
    }

    QList<QUrl> remoteUrls(const QUrl &baseUrl, const QStringList &files);
    QUrl downloadIfRemote(const QUrl &baseUrl, const QStringList &files);
    void countLocalFiles(const QUrl &baseUrl, const QStringList &names, unsigned long &totalFiles);

    void sendError(int errorCode, const QString &message);
    void sendInfo(const QString &message,
                  const QString &a1 = QString(),
                  const QString &a2 = QString(),
                  const QString &a3 = QString(),
                  const QString &a4 = QString());
    void sendReset(const QString &message,
                   const QString &a1 = QString(""),
                   const QString &a2 = QString(""),
                   const QString &a3 = QString(""),
                   const QString &a4 = QString(""));
    void sendSuccess();
    void sendMessage(const QString &message);
    void sendMaxProgressValue(qulonglong value);
    void sendAddProgress(qulonglong value, const QString &progress = QString());

    void setProgressTitle(const QString &title)
    {
        _progressTitle = title;
    }

    QString tempFileIfRemote(const QUrl &kurl, const QString &type);
    QString tempDirIfRemote(const QUrl &kurl);
    bool uploadTempFiles();

    bool isExited()
    {
        return _exited;
    }
    void terminate();

    QString getPassword(const QString &path);
    bool getArchiveInformation(QString &, QString &, QString &, QString &, const QUrl &);

    AbstractThreadedJob *_job;
    QEventLoop *_loop;

    QTemporaryDir *_downloadTempDir;
    KrArcObserver *_observer;

    QTemporaryFile *_tempFile;
    QString _tempFileName;
    QUrl _tempFileTarget;
    QTemporaryDir *_tempDir;
    QString _tempDirName;
    QUrl _tempDirTarget;

    bool _exited;

    QString _progressTitle;
};

enum PossibleCommands {
    CMD_ERROR = 1,
    CMD_INFO = 2,
    CMD_RESET = 3,
    CMD_DOWNLOAD_FILES = 4,
    CMD_UPLOAD_FILES = 5,
    CMD_SUCCESS = 6,
    CMD_MAXPROGRESSVALUE = 7,
    CMD_ADD_PROGRESS = 8,
    CMD_GET_PASSWORD = 9,
    CMD_MESSAGE = 10
};

class UserEvent : public QEvent
{
public:
    UserEvent(int command, const QList<QVariant> &args)
        : QEvent(QEvent::User)
        , _command(command)
        , _args(args)
    {
    }

    inline int command()
    {
        return _command;
    }
    inline const QList<QVariant> &args()
    {
        return _args;
    }

protected:
    int _command;
    QList<QVariant> _args;
};

#endif // __ABSTRACTTHREADED_JOB_H__
