/*
    SPDX-FileCopyrightText: 2009 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2009-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "abstractthreadedjob.h"

// QtCore
#include <QDir>
#include <QEventLoop>
#include <QPointer>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTimer>
// QtWidgets
#include <QApplication>

#include <KIO/JobUiDelegate>
#include <KIO/JobUiDelegateFactory>
#include <KLocalizedString>
#include <KMessageBox>

#include "../FileSystem/filesystemprovider.h"
#include "../krglobal.h"
#include "../krservices.h"
#include "krarchandler.h"

AbstractThreadedJob::AbstractThreadedJob()
    : _maxProgressValue(0)
    , _currentProgress(0)
    , _exiting(false)
    , _jobThread(nullptr)
{
}

void AbstractThreadedJob::startAbstractJobThread(AbstractJobThread *jobThread)
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
        auto *event = dynamic_cast<UserEvent *>(e);
        switch (event->command()) {
        case CMD_SUCCESS: {
            emitResult();
        } break;
        case CMD_ERROR: {
            auto error = event->args()[0].value<int>();
            QString errorText = event->args()[1].value<QString>();

            setError(error);
            setErrorText(errorText);
            emitResult();
        } break;
        case CMD_INFO: {
            QString info = event->args()[0].value<QString>();
            QString arg1 = event->args()[1].value<QString>();
            QString arg2 = event->args()[2].value<QString>();
            QString arg3 = event->args()[3].value<QString>();
            QString arg4 = event->args()[4].value<QString>();

            _title = info;
            emit description(this, info, qMakePair(arg1, arg2), qMakePair(arg3, arg4));

        } break;
        case CMD_RESET: {
            QString info = event->args()[0].value<QString>();
            QString arg1 = event->args()[1].value<QString>();
            QString arg2 = event->args()[2].value<QString>();
            QString arg3 = event->args()[3].value<QString>();
            QString arg4 = event->args()[4].value<QString>();

            _title = info;
            setProcessedAmount(KJob::Bytes, 0);
            setTotalAmount(KJob::Bytes, 0);
            emitSpeed(0);

            emit description(this, info, qMakePair(arg1, arg2), qMakePair(arg3, arg4));

        } break;
        case CMD_UPLOAD_FILES:
        case CMD_DOWNLOAD_FILES: {
            QList<QUrl> sources = KrServices::toUrlList(event->args()[0].value<QStringList>());
            QUrl dest = event->args()[1].value<QUrl>();
            KIO::Job *job = KIO::copy(sources, dest, KIO::HideProgressInfo);
            addSubjob(job);
            job->setUiDelegate(KIO::createDefaultJobUiDelegate());

            connect(job, &KIO::Job::result, this, &AbstractThreadedJob::slotDownloadResult);
            connect(job, SIGNAL(processedAmount(KJob *, KJob::Unit, qulonglong)), this, SLOT(slotProcessedAmount(KJob *, KJob::Unit, qulonglong)));
            connect(job, SIGNAL(totalAmount(KJob *, KJob::Unit, qulonglong)), this, SLOT(slotTotalAmount(KJob *, KJob::Unit, qulonglong)));
            connect(job, SIGNAL(speed(KJob *, ulong)), this, SLOT(slotSpeed(KJob *, ulong)));
            connect(job,
                    SIGNAL(description(KJob *, QString, QPair<QString, QString>, QPair<QString, QString>)),
                    this,
                    SLOT(slotDescription(KJob *, QString, QPair<QString, QString>, QPair<QString, QString>)));
        } break;
        case CMD_MAXPROGRESSVALUE: {
            auto maxValue = event->args()[0].value<qulonglong>();
            _maxProgressValue = maxValue;
            _currentProgress = 0;
        } break;
        case CMD_ADD_PROGRESS: {
            auto progress = event->args()[0].value<qulonglong>();
            _currentProgress += progress;
            if (_maxProgressValue != 0) {
                setPercent(100 * _currentProgress / _maxProgressValue);

                int elapsed = _time.isNull() ? 1 : _time.secsTo(QTime::currentTime());

                if (elapsed != 0 && event->args().count() > 1) {
                    _time = QTime::currentTime();
                    QString progressString = (event->args()[1].value<QString>());
                    emit description(this,
                                     _title,
                                     qMakePair(progressString, QString("%1/%2").arg(_currentProgress).arg(_maxProgressValue)),
                                     qMakePair(QString(), QString()));
                }
            }
        } break;
        case CMD_GET_PASSWORD: {
            QString path = event->args()[0].value<QString>();
            QString password = KrArcHandler::getPassword(path);

            auto *resultResp = new QList<QVariant>();
            (*resultResp) << password;
            addEventResponse(resultResp);
        } break;
        case CMD_MESSAGE: {
            QString message = event->args()[0].value<QString>();
            auto *ui = dynamic_cast<KIO::JobUiDelegate *>(uiDelegate());
            KMessageBox::information(ui ? ui->window() : nullptr, message);
            auto *resultResp = new QList<QVariant>();
            addEventResponse(resultResp);
        } break;
        }
        return true;
    } else {
        return KIO::Job::event(e);
    }
}

void AbstractThreadedJob::addEventResponse(QList<QVariant> *obj)
{
    _locker.lock();
    _stack.push(obj);
    _waiter.wakeOne();
    _locker.unlock();
}

QList<QVariant> *AbstractThreadedJob::getEventResponse(UserEvent *event)
{
    _locker.lock();
    QApplication::postEvent(this, event);
    _waiter.wait(&_locker);
    if (_exiting)
        return nullptr;
    QList<QVariant> *resp = _stack.pop();
    _locker.unlock();
    return resp;
}

void AbstractThreadedJob::sendEvent(UserEvent *event)
{
    QApplication::postEvent(this, event);
}

void AbstractThreadedJob::slotDownloadResult(KJob *job)
{
    auto *resultResp = new QList<QVariant>();
    if (job) {
        (*resultResp) << QVariant(job->error());
        (*resultResp) << QVariant(job->errorText());
    } else {
        (*resultResp) << QVariant(KJob::UserDefinedError);
        (*resultResp) << QVariant(QString(i18n("Internal error, undefined <job> in result signal")));
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

void AbstractThreadedJob::slotDescription(KJob *, const QString &title, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2)
{
    QString mytitle = title;
    if (!_title.isNull())
        mytitle = _title;
    emit description(this, mytitle, field1, field2);
}

class AbstractJobObserver : public KrArcObserver
{
protected:
    AbstractJobThread *_jobThread;

public:
    explicit AbstractJobObserver(AbstractJobThread *thread)
        : _jobThread(thread)
    {
    }
    ~AbstractJobObserver() override = default;

    void processEvents() override
    {
        usleep(1000);
        qApp->processEvents();
    }

    void subJobStarted(const QString &jobTitle, qulonglong count) override
    {
        _jobThread->sendReset(jobTitle);
        _jobThread->sendMaxProgressValue(count);
    }

    void subJobStopped() override
    {
    }

    bool wasCancelled() override
    {
        return _jobThread->_exited;
    }

    void error(const QString &error) override
    {
        _jobThread->sendError(KIO::ERR_NO_CONTENT, error);
    }

    void detailedError(const QString &error, const QString &details) override
    {
        _jobThread->sendError(KIO::ERR_NO_CONTENT, error + '\n' + details);
    }

    void incrementProgress(qsizetype c) override
    {
        _jobThread->sendAddProgress(c, _jobThread->_progressTitle);
    }
};

AbstractJobThread::AbstractJobThread()
    : _job(nullptr)
    , _downloadTempDir(nullptr)
    , _observer(nullptr)
    , _tempFile(nullptr)
    , _tempDir(nullptr)
    , _exited(false)
{
}

AbstractJobThread::~AbstractJobThread()
{
    if (_downloadTempDir) {
        delete _downloadTempDir;
        _downloadTempDir = nullptr;
    }
    if (_observer) {
        delete _observer;
        _observer = nullptr;
    }
    if (_tempFile) {
        delete _tempFile;
        _tempFile = nullptr;
    }
}

void AbstractJobThread::run()
{
    QTimer::singleShot(0, this, &AbstractJobThread::slotStart);

    QPointer<QEventLoop> threadLoop = new QEventLoop(this);
    _loop = threadLoop;
    threadLoop->exec();

    _loop = nullptr;
    delete threadLoop;
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

QList<QUrl> AbstractJobThread::remoteUrls(const QUrl &baseUrl, const QStringList &files)
{
    QList<QUrl> urlList;
    for (const QString &name : files) {
        QUrl url = baseUrl;
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + '/' + (name));
        urlList << url;
    }
    return urlList;
}

QUrl AbstractJobThread::downloadIfRemote(const QUrl &baseUrl, const QStringList &files)
{
    // download remote URL-s if necessary

    if (!baseUrl.isLocalFile()) {
        sendInfo(i18n("Downloading remote files"));

        _downloadTempDir = new QTemporaryDir();
        QList<QUrl> urlList = remoteUrls(baseUrl, files);

        const QUrl dest = QUrl::fromLocalFile(_downloadTempDir->path());

        QList<QVariant> args;
        args << KrServices::toStringList(urlList);
        args << dest;

        auto *downloadEvent = new UserEvent(CMD_DOWNLOAD_FILES, args);
        QList<QVariant> *result = _job->getEventResponse(downloadEvent);
        if (result == nullptr)
            return QUrl();

        auto errorCode = (*result)[0].value<int>();
        QString errorText = (*result)[1].value<QString>();

        delete result;

        if (errorCode) {
            sendError(errorCode, errorText);
            return QUrl();
        } else {
            return dest;
        }
    } else {
        return baseUrl;
    }
}

QString AbstractJobThread::tempFileIfRemote(const QUrl &kurl, const QString &type)
{
    if (kurl.isLocalFile()) {
        return kurl.path();
    }

    _tempFile = new QTemporaryFile(QDir::tempPath() + QLatin1String("/krusader_XXXXXX.") + type);
    _tempFile->open();
    _tempFileName = _tempFile->fileName();
    _tempFile->close(); // necessary to create the filename
    QFile::remove(_tempFileName);

    _tempFileTarget = kurl;
    return _tempFileName;
}

QString AbstractJobThread::tempDirIfRemote(const QUrl &kurl)
{
    if (kurl.isLocalFile()) {
        return kurl.adjusted(QUrl::StripTrailingSlash).path();
    }

    _tempDir = new QTemporaryDir();
    _tempDirTarget = kurl;
    return _tempDirName = _tempDir->path();
}

void AbstractJobThread::sendSuccess()
{
    terminate();

    QList<QVariant> args;

    auto *errorEvent = new UserEvent(CMD_SUCCESS, args);
    _job->sendEvent(errorEvent);
}

void AbstractJobThread::sendError(int errorCode, const QString &message)
{
    terminate();

    QList<QVariant> args;
    args << errorCode;
    args << message;

    auto *errorEvent = new UserEvent(CMD_ERROR, args);
    _job->sendEvent(errorEvent);
}

void AbstractJobThread::sendInfo(const QString &message, const QString &a1, const QString &a2, const QString &a3, const QString &a4)
{
    QList<QVariant> args;
    args << message;
    args << a1;
    args << a2;
    args << a3;
    args << a4;

    auto *infoEvent = new UserEvent(CMD_INFO, args);
    _job->sendEvent(infoEvent);
}

void AbstractJobThread::sendReset(const QString &message, const QString &a1, const QString &a2, const QString &a3, const QString &a4)
{
    QList<QVariant> args;
    args << message;
    args << a1;
    args << a2;
    args << a3;
    args << a4;

    auto *infoEvent = new UserEvent(CMD_RESET, args);
    _job->sendEvent(infoEvent);
}

void AbstractJobThread::sendMaxProgressValue(qulonglong value)
{
    QList<QVariant> args;
    args << value;

    auto *infoEvent = new UserEvent(CMD_MAXPROGRESSVALUE, args);
    _job->sendEvent(infoEvent);
}

void AbstractJobThread::sendAddProgress(qulonglong value, const QString &progress)
{
    QList<QVariant> args;
    args << value;

    if (!progress.isNull())
        args << progress;

    auto *infoEvent = new UserEvent(CMD_ADD_PROGRESS, args);
    _job->sendEvent(infoEvent);
}

void countFiles(const QString &path, unsigned long &totalFiles, bool &stop)
{
    const QDir dir(path);
    if (!dir.exists()) {
        totalFiles++; // assume it's a file
        return;
    }

    for (const QString &name : dir.entryList(QDir::NoDotAndDotDot)) {
        if (stop)
            return;

        countFiles(dir.absoluteFilePath(name), totalFiles, stop);
    }
}

void AbstractJobThread::countLocalFiles(const QUrl &baseUrl, const QStringList &names, unsigned long &totalFiles)
{
    sendReset(i18n("Counting files"));

    FileSystem *calcSpaceFileSystem = FileSystemProvider::instance().getFilesystem(baseUrl);
    calcSpaceFileSystem->scanDir(baseUrl);
    for (const QString &name : names) {
        if (_exited)
            return;

        const QString path = calcSpaceFileSystem->getUrl(name).toLocalFile();
        if (!QFileInfo::exists(path))
            return;

        countFiles(path, totalFiles, _exited);
    }

    delete calcSpaceFileSystem;
}

KrArcObserver *AbstractJobThread::observer()
{
    if (_observer)
        return _observer;
    _observer = new AbstractJobObserver(this);
    return _observer;
}

bool AbstractJobThread::uploadTempFiles()
{
    if (_tempFile != nullptr || _tempDir != nullptr) {
        sendInfo(i18n("Uploading to remote destination"));

        if (_tempFile) {
            QList<QUrl> urlList;
            urlList << QUrl::fromLocalFile(_tempFileName);

            QList<QVariant> args;
            args << KrServices::toStringList(urlList);
            args << _tempFileTarget;

            auto *uploadEvent = new UserEvent(CMD_UPLOAD_FILES, args);
            QList<QVariant> *result = _job->getEventResponse(uploadEvent);
            if (result == nullptr)
                return false;

            auto errorCode = (*result)[0].value<int>();
            QString errorText = (*result)[1].value<QString>();

            delete result;

            if (errorCode) {
                sendError(errorCode, errorText);
                return false;
            }
        }

        if (_tempDir) {
            QList<QUrl> urlList;
            QDir tempDir(_tempDirName);
            QStringList list = tempDir.entryList();
            for (const QString &name : std::as_const(list)) {
                if (name == "." || name == "..")
                    continue;
                QUrl url = QUrl::fromLocalFile(_tempDirName).adjusted(QUrl::StripTrailingSlash);
                url.setPath(url.path() + '/' + (name));
                urlList << url;
            }

            QList<QVariant> args;
            args << KrServices::toStringList(urlList);
            args << _tempDirTarget;

            auto *uploadEvent = new UserEvent(CMD_UPLOAD_FILES, args);
            QList<QVariant> *result = _job->getEventResponse(uploadEvent);
            if (result == nullptr)
                return false;

            auto errorCode = (*result)[0].value<int>();
            QString errorText = (*result)[1].value<QString>();

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

    auto *getPasswdEvent = new UserEvent(CMD_GET_PASSWORD, args);
    QList<QVariant> *result = _job->getEventResponse(getPasswdEvent);
    if (result == nullptr)
        return QString();

    QString password = (*result)[0].value<QString>();
    if (password.isNull())
        password = QString("");

    delete result;
    return password;
}

void AbstractJobThread::sendMessage(const QString &message)
{
    QList<QVariant> args;
    args << message;

    auto *getPasswdEvent = new UserEvent(CMD_MESSAGE, args);
    QList<QVariant> *result = _job->getEventResponse(getPasswdEvent);
    if (result == nullptr)
        return;
    delete result;
}

//! Gets some archive information that is needed in several cases.
/*!
    \param path A path to the archive.
    \param type The type of the archive.
    \param password The password of the archive.
    \param arcName The name of the archive.
    \param sourceFolder A QUrl, which may be remote, of the folder where the archive is.
    \return If the archive information has been obtained.
*/
bool AbstractJobThread::getArchiveInformation(QString &path, QString &type, QString &password, QString &arcName, const QUrl &sourceFolder)
{
    // Safety checks (though the user shouldn't have been able to select something named "" or "..")
    if (arcName.isEmpty())
        return false;
    if (arcName == "..")
        return false;

    QUrl url = sourceFolder.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + '/' + arcName);

    path = url.adjusted(QUrl::StripTrailingSlash).path();

    QMimeDatabase db;
    QMimeType mt = db.mimeTypeForUrl(url);
    QString mime = mt.isValid() ? mt.name() : QString();
    bool encrypted = false;
    type = krArcMan.getType(encrypted, path, mime);

    // Check that the archive is supported
    if (!KrArcHandler::arcSupported(type)) {
        sendError(KIO::ERR_NO_CONTENT, i18nc("%1=archive filename", "%1, unsupported archive type.", arcName));
        return false;
    }

    password = encrypted ? getPassword(path) : QString();

    return true;
}
