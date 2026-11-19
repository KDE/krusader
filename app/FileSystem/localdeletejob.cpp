/*
    SPDX-FileCopyrightText: 2026 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "localdeletejob.h"
#include "mountpath.h"

// QtCore
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QTextStream>
#include <QTimer>
#include <QUrl>
#include <QUuid>

#include <KDirNotify>
#include <KLocalizedString>

QList<LocalDeleteJob *> LocalDeleteJob::s_activeJobs;

LocalDeleteJob::LocalDeleteJob(const QList<QUrl> &urls, bool moveToTrash, QObject *parent)
    : KJob(parent)
    , m_urls(urls)
    , m_moveToTrash(moveToTrash)
{
    setCapabilities(Killable);
    registerJob(this);
}

LocalDeleteJob::~LocalDeleteJob()
{
    unregisterJob(this);
}

void LocalDeleteJob::registerJob(LocalDeleteJob *job)
{
    if (!s_activeJobs.contains(job))
        s_activeJobs.append(job);
}

void LocalDeleteJob::unregisterJob(LocalDeleteJob *job)
{
    s_activeJobs.removeAll(job);
}

bool LocalDeleteJob::touchesMountPoint(const QString &mntPoint) const
{
    for (const QUrl &url : m_urls) {
        if (url.isLocalFile() && MountPath::isSameOrChild(mntPoint, url.toLocalFile()))
            return true;
    }
    return false;
}

void LocalDeleteJob::abortJobsUnderMount(const QString &mntPoint)
{
    // Copy the list: kill() may delete a job and mutate s_activeJobs.
    const QList<LocalDeleteJob *> jobs = s_activeJobs;
    for (LocalDeleteJob *job : jobs) {
        if (job && job->touchesMountPoint(mntPoint))
            job->kill(KJob::Quietly);
    }
}

void LocalDeleteJob::start()
{
    QTimer::singleShot(0, this, &LocalDeleteJob::doWork);
}

bool LocalDeleteJob::doKill()
{
    m_killed = true;
    return true;
}

bool LocalDeleteJob::shouldAbort() const
{
    return m_killed;
}

void LocalDeleteJob::doWork()
{
    setTotalAmount(Unit::Files, static_cast<qulonglong>(m_urls.size()));
    qulonglong processed = 0;
    QStringList errors;

    for (const QUrl &url : m_urls) {
        // Allow Solid teardown / leaveMountPoint to run and call kill() while we work.
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        // kill() may already have emitted result; do not finish twice.
        if (isFinished())
            return;
        if (shouldAbort()) {
            setError(KilledJobError);
            emitResult();
            return;
        }

        if (!url.isLocalFile()) {
            errors.append(i18n("Not a local file: %1", url.toDisplayString()));
            continue;
        }

        const QString path = url.toLocalFile();
        QString errorMessage;
        const bool ok = m_moveToTrash ? trashPath(path, &errorMessage) : deletePath(path, &errorMessage);
        if (isFinished())
            return;
        if (!ok && !shouldAbort())
            errors.append(errorMessage);

        ++processed;
        setProcessedAmount(Unit::Files, processed);
        emitPercent(processed, static_cast<qulonglong>(m_urls.size()));
    }

    if (isFinished())
        return;

    if (shouldAbort()) {
        setError(KilledJobError);
    } else if (!errors.isEmpty()) {
        setError(KJob::UserDefinedError);
        setErrorText(errors.join(QLatin1Char('\n')));
    } else {
        org::kde::KDirNotify::emitFilesRemoved(m_urls);
    }

    emitResult();
}

bool LocalDeleteJob::deletePath(const QString &path, QString *errorMessage)
{
    if (shouldAbort()) {
        *errorMessage = i18n("Operation cancelled");
        return false;
    }

    const QFileInfo info(path);
    // Broken symlinks: exists() is false, but they must still be removable.
    if (!info.exists() && !info.isSymLink())
        return true;

    // Never follow directory symlinks when deleting.
    if (info.isDir() && !info.isSymLink()) {
        if (!QDir(path).removeRecursively()) {
            *errorMessage = i18n("Cannot delete folder %1", path);
            return false;
        }
        return true;
    }

    if (!QFile::remove(path)) {
        *errorMessage = i18n("Cannot delete %1", path);
        return false;
    }
    return true;
}

QString LocalDeleteJob::uniqueTrashName(const QString &filesDir, const QString &baseName)
{
    // Avoid TOCTOU on existence checks: include a UUID segment up front.
    const QFileInfo fi(baseName);
    const QString stem = fi.completeBaseName().isEmpty() ? QStringLiteral("file") : fi.completeBaseName();
    const QString suffix = fi.suffix().isEmpty() ? QString() : (QLatin1Char('.') + fi.suffix());
    const QString unique = stem + QLatin1Char('-') + QUuid::createUuid().toString(QUuid::WithoutBraces).left(8) + suffix;

    if (!QFileInfo::exists(filesDir + QLatin1Char('/') + unique))
        return unique;

    return stem + QLatin1Char('-') + QString::number(QDateTime::currentMSecsSinceEpoch()) + suffix;
}

bool LocalDeleteJob::copyPathRecursively(const QString &src, const QString &dst, QString *errorMessage)
{
    if (shouldAbort()) {
        *errorMessage = i18n("Operation cancelled");
        return false;
    }
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    const QFileInfo srcInfo(src);

    // Symlinks (including symlink-to-directory): recreate the link, never follow.
    if (srcInfo.isSymLink()) {
        if (!QFile::link(srcInfo.symLinkTarget(), dst)) {
            *errorMessage = i18n("Cannot copy %1", src);
            return false;
        }
        return true;
    }

    if (srcInfo.isDir()) {
        if (!QDir().mkpath(dst)) {
            *errorMessage = i18n("Cannot create folder %1", dst);
            return false;
        }
        const QStringList entries = QDir(src).entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        for (const QString &entry : entries) {
            if (!copyPathRecursively(src + QLatin1Char('/') + entry, dst + QLatin1Char('/') + entry, errorMessage))
                return false;
        }
        return true;
    }

    // Refuse to copy through a path whose final component became a symlink mid-flight.
    if (!QFile::copy(src, dst)) {
        *errorMessage = i18n("Cannot copy %1", src);
        return false;
    }
    return true;
}

bool LocalDeleteJob::trashPath(const QString &path, QString *errorMessage)
{
    if (shouldAbort()) {
        *errorMessage = i18n("Operation cancelled");
        return false;
    }

    // Freedesktop trash in the home trash directory — never create .Trash-* on the
    // source volume. That avoids leaving a kioslave/cwd on removable mounts.
    const QString trashRoot = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/Trash");
    const QString filesDir = trashRoot + QLatin1String("/files");
    const QString infoDir = trashRoot + QLatin1String("/info");

    if (!QDir().mkpath(filesDir) || !QDir().mkpath(infoDir)) {
        *errorMessage = i18n("Cannot create trash directory");
        return false;
    }

    const QFileInfo srcInfo(path);
    if (!srcInfo.exists() && !srcInfo.isSymLink()) {
        *errorMessage = i18n("File %1 does not exist", path);
        return false;
    }

    const QString trashName = uniqueTrashName(filesDir, srcInfo.fileName());
    const QString destPath = filesDir + QLatin1Char('/') + trashName;
    const QString infoPath = infoDir + QLatin1Char('/') + trashName + QLatin1String(".trashinfo");

    auto cleanupTrashArtifacts = [&]() {
        if (QFileInfo(destPath).isDir() && !QFileInfo(destPath).isSymLink())
            QDir(destPath).removeRecursively();
        else
            QFile::remove(destPath);
        QFile::remove(infoPath);
    };

    // FreeDesktop Trash spec: create the .trashinfo before moving the file.
    QFile infoFile(infoPath);
    if (!infoFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::NewOnly | QIODevice::Text)) {
        *errorMessage = i18n("Cannot write trash info for %1", path);
        return false;
    }

    const QString originalPath = srcInfo.absoluteFilePath();
    const QByteArray encodedPath = QUrl::toPercentEncoding(originalPath, "/");
    const QString deletionDate = QDateTime::currentDateTime().toString(Qt::ISODate);

    QTextStream stream(&infoFile);
    stream << "[Trash Info]\n";
    stream << "Path=" << QString::fromUtf8(encodedPath) << '\n';
    stream << "DeletionDate=" << deletionDate << '\n';
    infoFile.close();

    // Re-stat after writing info (narrow TOCTOU window for device check vs move).
    const QFileInfo srcInfoAgain(path);
    if (!srcInfoAgain.exists() && !srcInfoAgain.isSymLink()) {
        cleanupTrashArtifacts();
        *errorMessage = i18n("File %1 does not exist", path);
        return false;
    }

    const QStorageInfo srcStorage(srcInfoAgain.absolutePath());
    const QStorageInfo trashStorage(filesDir);
    const bool sameDevice = srcStorage.isValid() && trashStorage.isValid() && srcStorage.device() == trashStorage.device();

    if (sameDevice && QFile::rename(path, destPath))
        return true;

    // Cross-device (typical for USB): copy into home trash, then remove source.
    QString copyError;
    if (!copyPathRecursively(path, destPath, &copyError)) {
        cleanupTrashArtifacts();
        *errorMessage = copyError.isEmpty() ? i18n("Cannot move %1 to trash", path) : copyError;
        return false;
    }

    if (!deletePath(path, errorMessage)) {
        cleanupTrashArtifacts();
        return false;
    }

    return true;
}
