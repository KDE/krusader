/***************************************************************************
                         packjob.cpp  -  description
                             -------------------
    copyright            : (C) 2009 + by Csaba Karai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "abstractthreadedjob.h"

#include <QtCore/QTimer>
#include <QEventLoop>
#include <QtGui/QApplication>
#include <QtCore/QDir>

#include <klocale.h>
#include <ktempdir.h>
#include <ktemporaryfile.h>
#include <kio/jobuidelegate.h>
#include <kmessagebox.h>

#include "krvfshandler.h"
#include "krarchandler.h"
#include "vfs.h"
#include "preservingcopyjob.h"
#include "../krusader.h"

AbstractThreadedJob::AbstractThreadedJob() : KIO::Job(), _locker(), _waiter(), _stack(), _maxProgressValue(0),
        _currentProgress(0), _exiting(false), _jobThread(0)
{
}

void AbstractThreadedJob::start(AbstractJobThread * jobThread)
{
    _jobThread = jobThread;
    _jobThread->setJob(this);
    _jobThread->moveToThread(_jobThread);
    _jobThread->start();
}

AbstractThreadedJob::~AbstractThreadedJob()
{
    _exiting = true;
    if (_jobThread) {
        _jobThread->abort();

        _locker.lock();
        _waiter.wakeAll();
        _locker.unlock();

        _jobThread->wait();
        delete _jobThread;
    }
}

bool AbstractThreadedJob::event(QEvent *e)
{
    if (e->type() == QEvent::User) {
        UserEvent *event = (UserEvent*) e;
        switch (event->command()) {
        case CMD_SUCCESS: {
            emitResult();
        }
        break;
        case CMD_ERROR: {
            int error = event->args()[ 0 ].value<int>();
            QString errorText = event->args()[ 1 ].value<QString>();

            setError(error);
            setErrorText(errorText);
            emitResult();
        }
        break;
        case CMD_INFO: {
            QString info = event->args()[ 0 ].value<QString>();
            QString arg1 = event->args()[ 1 ].value<QString>();
            QString arg2 = event->args()[ 2 ].value<QString>();
            QString arg3 = event->args()[ 3 ].value<QString>();
            QString arg4 = event->args()[ 4 ].value<QString>();

            _title = info;
            emit description(this, info,
                             qMakePair(arg1, arg2),
                             qMakePair(arg3, arg4));

        }
        break;
        case CMD_RESET: {
            QString info = event->args()[ 0 ].value<QString>();
            QString arg1 = event->args()[ 1 ].value<QString>();
            QString arg2 = event->args()[ 2 ].value<QString>();
            QString arg3 = event->args()[ 3 ].value<QString>();
            QString arg4 = event->args()[ 4 ].value<QString>();

            _title = info;
            setProcessedAmount(KJob::Bytes, 0);
            setTotalAmount(KJob::Bytes, 0);
            emitSpeed(0);

            emit description(this, info,
                             qMakePair(arg1, arg2),
                             qMakePair(arg3, arg4));

        }
        break;
        case CMD_UPLOAD_FILES:
        case CMD_DOWNLOAD_FILES: {
            KUrl::List sources = event->args()[ 0 ].value<KUrl::List>();
            KUrl dest = event->args()[ 1 ].value<KUrl>();
            KIO::Job *job = PreservingCopyJob::createCopyJob(PM_PRESERVE_ATTR, sources, dest, KIO::CopyJob::Copy, false, false);
            addSubjob(job);
            job->setUiDelegate(new KIO::JobUiDelegate());

            connect(job, SIGNAL(result(KJob*)), this, SLOT(slotDownloadResult(KJob*)));
            connect(job, SIGNAL(processedAmount(KJob *, KJob::Unit, qulonglong)),
                    this, SLOT(slotProcessedAmount(KJob *, KJob::Unit, qulonglong)));
            connect(job, SIGNAL(totalAmount(KJob *, KJob::Unit, qulonglong)),
                    this, SLOT(slotTotalAmount(KJob *, KJob::Unit, qulonglong)));
            connect(job, SIGNAL(speed(KJob *, unsigned long)),
                    this, SLOT(slotSpeed(KJob *, unsigned long)));
            connect(job, SIGNAL(description(KJob *, const QString &, const QPair<QString, QString> &, const QPair<QString, QString> &)),
                    this, SLOT(slotDescription(KJob *, const QString &, const QPair<QString, QString> &, const QPair<QString, QString> &)));
        }
        break;
        case CMD_MAXPROGRESSVALUE: {
            qulonglong maxValue = event->args()[ 0 ].value<qulonglong>();
            _maxProgressValue = maxValue;
            _currentProgress = 0;
        }
        break;
        case CMD_ADD_PROGRESS: {
            qulonglong progress = event->args()[ 0 ].value<qulonglong>();
            _currentProgress += progress;
            if (_maxProgressValue != 0) {
                setPercent(100 * _currentProgress / _maxProgressValue);

                int elapsed = _time.isNull() ? 1 : _time.secsTo(QTime::currentTime());

                if (elapsed != 0 && event->args().count() > 1) {
                    _time = QTime::currentTime();
                    QString progressString = (event->args()[ 1 ].value<QString>());
                    emit description(this, _title,
                                     qMakePair(progressString, QString("%1/%2").arg(_currentProgress).arg(_maxProgressValue)),
                                     qMakePair(QString(), QString())
                                    );
                }
            }
        }
        break;
        case CMD_GET_PASSWORD: {
            QString path = event->args()[ 0 ].value<QString>();
            QString password = KRarcHandler::getPassword(path);

            QList<QVariant> *resultResp = new QList<QVariant> ();
            (*resultResp) << password;
            addEventResponse(resultResp);
        }
        break;
        case CMD_MESSAGE: {
            QString message = event->args()[ 0 ].value<QString>();
            KMessageBox::information(krApp, message);
            QList<QVariant> *resultResp = new QList<QVariant> ();
            addEventResponse(resultResp);
        }
        break;
        }
        return true;
    } else {
        return KIO::Job::event(e);
    }
}

void AbstractThreadedJob::addEventResponse(QList<QVariant> * obj)
{
    _locker.lock();
    _stack.push(obj);
    _waiter.wakeOne();
    _locker.unlock();
}

QList<QVariant> * AbstractThreadedJob::getEventResponse(UserEvent * event)
{
    _locker.lock();
    QApplication::postEvent(this, event);
    _waiter.wait(&_locker);
    if (_exiting)
        return 0;
    QList<QVariant> *resp = _stack.pop();
    _locker.unlock();
    return resp;
}

void AbstractThreadedJob::sendEvent(UserEvent * event)
{
    QApplication::postEvent(this, event);
}

void AbstractThreadedJob::slotDownloadResult(KJob* job)
{
    QList<QVariant> *resultResp = new QList<QVariant> ();
    if (job) {
        (*resultResp) << QVariant(job->error());
        (*resultResp) << QVariant(job->errorText());
    } else {
        (*resultResp) << QVariant(KJob::UserDefinedError);
        (*resultResp) << QVariant(QString(i18n("Internal error, undefined <job> in result signal!")));
    }

    addEventResponse(resultResp);
}

void AbstractThreadedJob::slotProcessedAmount(KJob *, KJob::Unit unit, qulonglong xu)
{
    setProcessedAmount(unit, xu);
}

void AbstractThreadedJob::slotTotalAmount(KJob *, KJob::Unit unit, qulonglong xu)
{
    setTotalAmount(unit, xu);
}

void AbstractThreadedJob::slotSpeed(KJob *, unsigned long spd)
{
    emitSpeed(spd);
}

void AbstractThreadedJob::slotDescription(KJob *, const QString &title, const QPair<QString, QString> &field1,
        const QPair<QString, QString> &field2)
{
    QString mytitle = title;
    if (!_title.isNull())
        mytitle = _title;
    emit description(this, mytitle, field1, field2);
}

class AbstractJobObserver : public KRarcObserver
{
protected:
    AbstractJobThread * _jobThread;
public:
    AbstractJobObserver(AbstractJobThread * thread): _jobThread(thread) {}
    virtual ~AbstractJobObserver() {}

    virtual void processEvents() {
        usleep(1000);
        qApp->processEvents();
    }

    virtual void subJobStarted(const QString & jobTitle, int count) {
        _jobThread->sendReset(jobTitle);
        _jobThread->sendMaxProgressValue(count);
    }

    virtual void subJobStopped() {
    }

    virtual bool wasCancelled() {
        return _jobThread->_exited;
    }

    virtual void error(const QString & error) {
        _jobThread->sendError(KIO::ERR_NO_CONTENT, error);
    }

    virtual void detailedError(const QString & error, const QString & details) {
        _jobThread->sendError(KIO::ERR_NO_CONTENT, error + '\n' + details);
    }

    virtual void incrementProgress(int c) {
        _jobThread->sendAddProgress(c, _jobThread->_progressTitle);
    }
};


AbstractJobThread::AbstractJobThread() : _job(0), _downloadTempDir(0), _observer(0), _tempFile(0),
        _tempDir(0), _exited(false)
{
}

AbstractJobThread::~AbstractJobThread()
{
    if (_downloadTempDir) {
        delete _downloadTempDir;
        _downloadTempDir = 0;
    }
    if (_observer) {
        delete _observer;
        _observer = 0;
    }
    if (_tempFile) {
        delete _tempFile;
        _tempFile = 0;
    }
}

void AbstractJobThread::run()
{
    QTimer::singleShot(0, this, SLOT(slotStart()));

    QEventLoop threadLoop(this);
    _loop = &threadLoop;
    threadLoop.exec();

    _loop = 0;
}

void AbstractJobThread::terminate()
{
    if (_loop && !_exited) {
        _loop->quit();
        _exited = true;
    }
}

void AbstractJobThread::abort()
{
    terminate();
}

KUrl::List AbstractJobThread::remoteUrls(const KUrl &baseUrl, const QStringList & files)
{
    KUrl::List urlList;
    foreach(const QString &name, files) {
        KUrl url = baseUrl;
        url.addPath(name);
        urlList << url;
    }
    return urlList;
}

KUrl AbstractJobThread::downloadIfRemote(const KUrl &baseUrl, const QStringList & files)
{
    // download remote URL-s if necessary

    if (!baseUrl.isLocalFile()) {
        sendInfo(i18n("Downloading remote files"));

        _downloadTempDir = new KTempDir();
        KUrl::List urlList = remoteUrls(baseUrl, files);

        KUrl dest(_downloadTempDir->name());

        QList<QVariant> args;
        args << urlList;
        args << dest;

        UserEvent * downloadEvent = new UserEvent(CMD_DOWNLOAD_FILES, args);
        QList<QVariant> * result = _job->getEventResponse(downloadEvent);
        if (result == 0)
            return KUrl();

        int errorCode = (*result)[ 0 ].value<int>();
        QString errorText = (*result)[ 1 ].value<QString>();

        delete result;

        if (errorCode) {
            sendError(errorCode, errorText);
            return KUrl();
        } else {
            return dest;
        }
    } else {
        return baseUrl;
    }
}


QString AbstractJobThread::tempFileIfRemote(const KUrl &kurl, const QString &type)
{
    if (kurl.isLocalFile()) {
        return kurl.path();
    }

    _tempFile = new KTemporaryFile();
    _tempFile->setSuffix(QString(".") + type);
    _tempFile->open();
    _tempFileName = _tempFile->fileName();
    _tempFile->close(); // necessary to create the filename
    QFile::remove(_tempFileName);

    _tempFileTarget = kurl;
    return _tempFileName;
}

QString AbstractJobThread::tempDirIfRemote(const KUrl &kurl)
{
    if (kurl.isLocalFile()) {
        return kurl.path(KUrl::RemoveTrailingSlash);
    }

    _tempDir = new KTempDir();
    _tempDirTarget = kurl;
    return _tempDirName = _tempDir->name();
}

void AbstractJobThread::sendSuccess()
{
    terminate();

    QList<QVariant> args;

    UserEvent * errorEvent = new UserEvent(CMD_SUCCESS, args);
    _job->sendEvent(errorEvent);
}

void AbstractJobThread::sendError(int errorCode, QString message)
{
    terminate();

    QList<QVariant> args;
    args << errorCode;
    args << message;

    UserEvent * errorEvent = new UserEvent(CMD_ERROR, args);
    _job->sendEvent(errorEvent);
}

void AbstractJobThread::sendInfo(QString message, QString a1, QString a2, QString a3, QString a4)
{
    QList<QVariant> args;
    args << message;
    args << a1;
    args << a2;
    args << a3;
    args << a4;

    UserEvent * infoEvent = new UserEvent(CMD_INFO, args);
    _job->sendEvent(infoEvent);
}

void AbstractJobThread::sendReset(QString message, QString a1, QString a2, QString a3, QString a4)
{
    QList<QVariant> args;
    args << message;
    args << a1;
    args << a2;
    args << a3;
    args << a4;

    UserEvent * infoEvent = new UserEvent(CMD_RESET, args);
    _job->sendEvent(infoEvent);
}

void AbstractJobThread::sendMaxProgressValue(qulonglong value)
{
    QList<QVariant> args;
    args << value;

    UserEvent * infoEvent = new UserEvent(CMD_MAXPROGRESSVALUE, args);
    _job->sendEvent(infoEvent);
}

void AbstractJobThread::sendAddProgress(qulonglong value, const QString &progress)
{
    QList<QVariant> args;
    args << value;

    if (!progress.isNull())
        args << progress;

    UserEvent * infoEvent = new UserEvent(CMD_ADD_PROGRESS, args);
    _job->sendEvent(infoEvent);
}

void AbstractJobThread::calcSpaceLocal(const KUrl &baseUrl, const QStringList & files, KIO::filesize_t &totalSize,
                                       unsigned long &totalDirs, unsigned long &totalFiles)
{
    sendReset(i18n("Calculating space"));

    vfs *calcSpaceVfs = KrVfsHandler::getVfs(baseUrl);
    calcSpaceVfs->vfs_refresh(baseUrl);

    for (int i = 0; i != files.count(); i++) {
        calcSpaceVfs->vfs_calcSpaceLocal(files[ i ], &totalSize, &totalFiles, &totalDirs, &_exited);
    }
    delete calcSpaceVfs;
}

KRarcObserver * AbstractJobThread::observer()
{
    if (_observer)
        return _observer;
    _observer = new AbstractJobObserver(this);
    return _observer;
}

bool AbstractJobThread::uploadTempFiles()
{
    if (_tempFile != 0 || _tempDir != 0) {
        sendInfo(i18n("Uploading to remote destination"));

        if (_tempFile) {
            KUrl::List urlList;
            urlList << KUrl(_tempFileName);

            QList<QVariant> args;
            args << urlList;
            args << _tempFileTarget;

            UserEvent * uploadEvent = new UserEvent(CMD_UPLOAD_FILES, args);
            QList<QVariant> * result = _job->getEventResponse(uploadEvent);
            if (result == 0)
                return false;

            int errorCode = (*result)[ 0 ].value<int>();
            QString errorText = (*result)[ 1 ].value<QString>();

            delete result;

            if (errorCode) {
                sendError(errorCode, errorText);
                return false;
            }
        }

        if (_tempDir) {
            KUrl::List urlList;
            QDir tempDir(_tempDirName);
            QStringList list = tempDir.entryList();
            foreach(const QString &name, list) {
                if (name == "." || name == "..")
                    continue;
                KUrl url = _tempDirName;
                url.addPath(name);
                urlList << url;
            }

            QList<QVariant> args;
            args << urlList;
            args << _tempDirTarget;

            UserEvent * uploadEvent = new UserEvent(CMD_UPLOAD_FILES, args);
            QList<QVariant> * result = _job->getEventResponse(uploadEvent);
            if (result == 0)
                return false;

            int errorCode = (*result)[ 0 ].value<int>();
            QString errorText = (*result)[ 1 ].value<QString>();

            delete result;

            if (errorCode) {
                sendError(errorCode, errorText);
                return false;
            }
        }
    }
    return true;
}

QString AbstractJobThread::getPassword(const QString &path)
{
    QList<QVariant> args;
    args << path;

    UserEvent * getPasswdEvent = new UserEvent(CMD_GET_PASSWORD, args);
    QList<QVariant> * result = _job->getEventResponse(getPasswdEvent);
    if (result == 0)
        return QString();

    QString password = (*result)[ 0 ].value<QString>();
    if (password.isNull())
        password = QString("");

    delete result;
    return password;
}

void AbstractJobThread::sendMessage(const QString &message)
{
    QList<QVariant> args;
    args << message;

    UserEvent * getPasswdEvent = new UserEvent(CMD_MESSAGE, args);
    QList<QVariant> * result = _job->getEventResponse(getPasswdEvent);
    if (result == 0)
        return;
    delete result;
}

