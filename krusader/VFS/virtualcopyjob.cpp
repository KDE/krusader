/***************************************************************************
                      virtualcopyjob.cpp  -  description
                             -------------------
    copyright            : (C) 2006 + by Csaba Karai
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

#include "virtualcopyjob.h"
#include "vfs.h"
#include "vfile.h"

#include <QtCore/QDir>

#include <KI18n/KLocalizedString>
#include <KIO/Global>
#include <KIO/JobClasses>
#include <KIO/DirectorySizeJob>
#include <KIO/JobUiDelegate>
#include <KJobWidgets/KUiServerJobTracker>

#define REPORT_TIMEOUT 200

class MyUiDelegate : public KIO::JobUiDelegate
{
public:
    MyUiDelegate(VirtualCopyJob * ref) : KIO::JobUiDelegate(), copyJobRef(ref) {}

    virtual KIO::RenameDialog_Result askFileRename(KJob * job, const QString & caption, const QUrl& src,
            const QUrl & dest, KIO::RenameDialog_Options mode,
            QString& newDest, KIO::filesize_t sizeSrc = (KIO::filesize_t) - 1,
            KIO::filesize_t sizeDest = (KIO::filesize_t) - 1,
            const QDateTime &ctimeSrc = QDateTime(), const QDateTime &ctimeDest = QDateTime(),
            const QDateTime &mtimeSrc = QDateTime(), const QDateTime &mtimeDest = QDateTime()) Q_DECL_OVERRIDE {
        if (copyJobRef->isSkipAll()) {
            if (mode & KIO::M_MULTI)
                return KIO::R_AUTO_SKIP;
            else
                return KIO::R_SKIP;
        }
        if (copyJobRef->isOverwriteAll()) {
            if (mode & KIO::M_MULTI)
                return KIO::R_OVERWRITE_ALL;
            else
                return KIO::R_OVERWRITE;
        }

        KIO::RenameDialog_Mode mmode = (KIO::RenameDialog_Mode)0;
        if (copyJobRef->isMulti())
            mmode = (KIO::RenameDialog_Mode)(mmode | KIO::M_MULTI | KIO::M_SKIP);
        else
            mmode = (KIO::RenameDialog_Mode)(mmode /*| KIO::M_SINGLE*/);

        if (mode & KIO::M_OVERWRITE)
            mmode = (KIO::RenameDialog_Mode)(mmode | KIO::M_OVERWRITE);
        if (mode & KIO::M_OVERWRITE_ITSELF)
            mmode = (KIO::RenameDialog_Mode)(mmode | KIO::M_OVERWRITE_ITSELF);

        KIO::RenameDialog_Result res = KIO::JobUiDelegate::askFileRename(job, caption, src, dest,  mode, newDest,
                                       sizeSrc, sizeDest, ctimeSrc, ctimeDest, mtimeSrc, mtimeDest);

        if (res == KIO::R_AUTO_SKIP) {
            copyJobRef->setSkipAll();
            if (!(mode & KIO::M_MULTI))
                res = KIO::R_SKIP;
        }
        if (res == KIO::R_OVERWRITE_ALL) {
            copyJobRef->setOverwriteAll();
            if (!(mode & KIO::M_MULTI))
                res = KIO::R_OVERWRITE;
        }
        return res;
    }

    virtual KIO::SkipDialog_Result askSkip(KJob * job, KIO::SkipDialog_Options options,
                                           const QString & error_text) Q_DECL_OVERRIDE {
        if (copyJobRef->isSkipAll()) {
            if (options & KIO::SkipDialog_MultipleItems)
                return KIO::S_AUTO_SKIP;
            else
                return KIO::S_SKIP;
        }

        KIO::SkipDialog_Result res = KIO::JobUiDelegate::askSkip(job,
                                                                 copyJobRef->isMulti() ? KIO::SkipDialog_MultipleItems: (KIO::SkipDialog_Option)0,
                                                                 error_text);
        if (res == KIO::S_AUTO_SKIP) {
            copyJobRef->setSkipAll();
            if (!(options & KIO::SkipDialog_MultipleItems))
                res = KIO::S_SKIP;
        }

        return res;
    }

private:
    VirtualCopyJob * copyJobRef;
};

VirtualCopyJob::VirtualCopyJob(const QStringList *names, vfs * vfs, const QUrl &dest, const QUrl& baseURL,
                               PreserveMode pmode, KIO::CopyJob::CopyMode mode, bool showProgressInfo, bool autoStart) : KIO::Job(), m_overwriteAll(false),
        m_skipAll(false), m_multi(false), m_totalSize(0), m_totalFiles(0), m_totalSubdirs(0),
        m_processedSize(0), m_processedFiles(0), m_processedSubdirs(0), m_tempSize(0), m_tempFiles(0),
        m_dirsToGetSize(), m_filesToCopy(), m_size(), m_filenum(), m_subdirs(), m_baseURL(baseURL),
        m_dest(dest), m_pmode(pmode), m_mode(mode), m_showProgressInfo(showProgressInfo),
        m_state(ST_STARTING), m_reportTimer(), m_current(), m_currentDir(), m_dirStack()
{

    m_dest = vfs->ensureTrailingSlash(m_dest);

    vfile * file = vfs->vfs_getFirstFile();
    while (file) {
        if (names->contains(file->vfile_getName())) {
            QString relativeDir = QDir(baseURL.path()).relativeFilePath(KIO::upUrl(file->vfile_getUrl()).path());

            if (m_filesToCopy.find(relativeDir) == m_filesToCopy.end()) {
                m_filesToCopy.insert(relativeDir, new QList<QUrl>());
                // initialize the dir content
                m_size[ relativeDir ] = 0;
                m_filenum[ relativeDir ] = 0;
                m_subdirs[ relativeDir ] = 0;
            }
            QList<QUrl> *list = m_filesToCopy[ relativeDir ];

            if (!list->contains(file->vfile_getUrl())) {
                if (file->vfile_isDir()) {
                    m_dirsToGetSize.append(file->vfile_getUrl());
                    m_totalSubdirs++;
                    m_subdirs[ relativeDir ]++;
                } else {
                    m_totalFiles++;
                    m_filenum[ relativeDir ]++;
                    m_totalSize += file->vfile_getSize();
                    m_size[ relativeDir ] += file->vfile_getSize();
                }
                list->append(file->vfile_getUrl());
            }
        }
        file = vfs->vfs_getNextFile();
    }

    if (showProgressInfo) {
        setUiDelegate(new MyUiDelegate(this));
        KIO::getJobTracker()->registerJob(this);
    }

    if (autoStart)
        QTimer::singleShot(0, this, SLOT(slotStart()));
}

VirtualCopyJob::~VirtualCopyJob()
{
    QHashIterator< QString, QList<QUrl> *> lit(m_filesToCopy);
    while (lit.hasNext())
        delete lit.next().value();
}

void VirtualCopyJob::slotStart()
{
    connect(&m_reportTimer, SIGNAL(timeout()), this, SLOT(slotReport()));
    m_reportTimer.setSingleShot(false);
    m_reportTimer.start(REPORT_TIMEOUT);

    statNextDir();
}

void VirtualCopyJob::slotReport()
{
    switch (m_state) {
    case ST_CREATING_DIRECTORY:
        break;
    case ST_CALCULATING_TOTAL_SIZE:
        setTotalAmount(KJob::Bytes, m_totalSize);
        setTotalAmount(KJob::Directories, m_totalSubdirs);
        setTotalAmount(KJob::Files, m_totalFiles);
        break;
    case ST_COPYING: {
        setProcessedAmount(KJob::Directories, m_processedSubdirs);
        setProcessedAmount(KJob::Files, m_processedFiles + m_tempFiles);
        setProcessedAmount(KJob::Bytes, m_processedSize + m_tempSize);

        double percDbl = ((double)(m_processedSize + m_tempSize) / (double)m_totalSize) * 100. + 0.5;
        unsigned long perc = (long)percDbl;
        if (perc > 100)
            perc = 100;
        setPercent(perc);
        break;
    }
    default:
        break;
    }
}

void VirtualCopyJob::statNextDir()
{
    m_state = ST_CALCULATING_TOTAL_SIZE;

    if (m_dirsToGetSize.count() == 0) {
        slotReport(); // report the total size values
        createNextDir();
        return;
    }
    QUrl dirToCheck = m_dirsToGetSize.first();
    m_dirsToGetSize.pop_front();

    m_currentDir = QDir(m_baseURL.path()).relativeFilePath(KIO::upUrl(dirToCheck).path());

    KIO::DirectorySizeJob* kds  = KIO::directorySize(dirToCheck);
    connect(kds, SIGNAL(result(KJob*)), this, SLOT(slotKdsResult(KJob*)));
}

void VirtualCopyJob::slotKdsResult(KJob * job)
{
    KIO::DirectorySizeJob* kds = static_cast<KIO::DirectorySizeJob*>(job);
    m_totalSize += kds->totalSize();
    m_totalFiles += kds->totalFiles();
    m_totalSubdirs += kds->totalSubdirs();

    m_size[ m_currentDir ] += kds->totalSize();
    m_filenum[ m_currentDir ] += kds->totalFiles();
    m_subdirs[ m_currentDir ] += kds->totalSubdirs();
    statNextDir();
}

void VirtualCopyJob::createNextDir()
{
    m_state = ST_CREATING_DIRECTORY;

    if (m_totalFiles + m_totalSubdirs > 1)
        m_multi = true;

    QHashIterator<QString, QList<QUrl> *> diter(m_filesToCopy);
    if (!diter.hasNext()) {
        emitResult();
        return;
    }

    m_currentDir = diter.next().key();
    m_current = m_dest;
    if (m_currentDir != "./" && !m_currentDir.isEmpty())
        m_current = m_current.adjusted(QUrl::StripTrailingSlash);
        m_current.setPath(m_current.path() + '/' + (m_currentDir));

    KIO::Job *job = KIO::stat(m_current, KIO::HideProgressInfo);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotStatResult(KJob*)));
}

void VirtualCopyJob::slotStatResult(KJob *job)
{
    QUrl url = (static_cast<KIO::SimpleJob*>(job))->url();

    if (job && job->error()) {
        if (job->error() == KIO::ERR_DOES_NOT_EXIST && !url.matches(KIO::upUrl(url), QUrl::StripTrailingSlash)) {
            m_dirStack.push_back(url.fileName());
            KIO::Job *job = KIO::stat(KIO::upUrl(url), KIO::HideProgressInfo);
            connect(job, SIGNAL(result(KJob*)), this, SLOT(slotStatResult(KJob*)));
            return;
        }
        job->uiDelegate()->showErrorMessage();
        directoryFinished(m_currentDir);
        createNextDir();
        return;
    }

    if (m_dirStack.count()) {
        url.setPath(vfs::ensureTrailingSlash(url).path() + m_dirStack.last());
        m_dirStack.pop_back();

        KIO::Job *mkdir_job = KIO::mkdir(url);
        connect(mkdir_job, SIGNAL(result(KJob*)), this, SLOT(slotMkdirResult(KJob*)));
    } else
        copyCurrentDir();
}

void VirtualCopyJob::slotMkdirResult(KJob *job)
{
    QUrl url = (static_cast<KIO::SimpleJob*>(job))->url();

    if (job && job->error()) {
        if (uiDelegate() && !m_skipAll) {
            KIO::JobUiDelegate *ui = static_cast<KIO::JobUiDelegate*>(uiDelegate());
            KIO::SkipDialog_Result skipResult = ui->askSkip(this, m_multi ? KIO::SkipDialog_MultipleItems: (KIO::SkipDialog_Option)0, job->errorString());
            switch (skipResult) {
            case KIO::S_CANCEL:
                setError(KIO::ERR_USER_CANCELED);
                emitResult();
                return;
            case KIO::S_AUTO_SKIP:
                m_skipAll = true;
                break;
            case KIO::S_SKIP:
            default:
                break;
            }
        }

        directoryFinished(m_currentDir);
        createNextDir();
        return;
    }

    if (m_dirStack.count()) {
        url.setPath(vfs::ensureTrailingSlash(url).path() + m_dirStack.last());
        m_dirStack.pop_back();

        KIO::Job *mkdir_job = KIO::mkdir(url);
        connect(mkdir_job, SIGNAL(result(KJob*)), this, SLOT(slotMkdirResult(KJob*)));
    } else
        copyCurrentDir();
}

void VirtualCopyJob::copyCurrentDir()
{
    m_state = ST_COPYING;

    KIO::Job * copy_job = PreservingCopyJob::createCopyJob(m_pmode, *m_filesToCopy[ m_currentDir ], m_current,
                          m_mode, false, false);
    copy_job->setParentJob(this);
    copy_job->setUiDelegate(new MyUiDelegate(this));

    connect(copy_job, SIGNAL(copying(KIO::Job *, const QUrl &, const QUrl &)),
            this, SLOT(slotCopying(KIO::Job *, const QUrl &, const QUrl &)));
    connect(copy_job, SIGNAL(moving(KIO::Job *, const QUrl &, const QUrl &)),
            this, SLOT(slotMoving(KIO::Job *, const QUrl &, const QUrl &)));
    connect(copy_job, SIGNAL(processedFiles(KIO::Job *, unsigned long)),
            this, SLOT(slotProcessedFiles(KIO::Job *, unsigned long)));
    connect(copy_job, SIGNAL(processedSize(KJob *, qulonglong)),
            this, SLOT(slotProcessedSize(KJob *, qulonglong)));
    connect(copy_job, SIGNAL(result(KJob*)), this, SLOT(slotCopyResult(KJob*)));
}

void VirtualCopyJob::slotCopyResult(KJob *job)
{
    if (job && job->error()) {
        setError(job->error());
        setErrorText(job->errorText());

        if (job->error() == KIO::ERR_USER_CANCELED) {
            emitResult();
            return;
        }

        if (uiDelegate())
            uiDelegate()->showErrorMessage();
        setError(0);
    }

    directoryFinished(m_currentDir);
    createNextDir();
}

void VirtualCopyJob::directoryFinished(const QString &name)
{
    delete m_filesToCopy[ name ];
    m_filesToCopy.remove(name);

    m_processedSize += m_size[ name ];
    m_processedFiles += m_filenum[ name ];
    m_processedSubdirs += m_subdirs[ name ];

    m_tempSize = m_tempFiles = 0;

    m_size.remove(name);
    m_filenum.remove(name);
    m_subdirs.remove(name);
}

void VirtualCopyJob::slotCopying(KIO::Job *, const QUrl &src, const QUrl &dest)
{
    emit description(this, i18nc("@title job", "Copying"),
                     qMakePair(i18n("Source"), src.toDisplayString()),
                     qMakePair(i18n("Destination"), dest.toDisplayString()));
}

void VirtualCopyJob::slotMoving(KIO::Job *, const QUrl &src, const QUrl &dest)
{
    emit description(this, i18nc("@title job", "Moving"),
                     qMakePair(i18n("Source"), src.toDisplayString()),
                     qMakePair(i18n("Destination"), dest.toDisplayString()));
}

void VirtualCopyJob::slotProcessedFiles(KIO::Job *, unsigned long filenum)
{
    m_tempFiles = filenum;
}

void VirtualCopyJob::slotProcessedSize(KJob *, qulonglong size)
{
    m_tempSize = size;
}

#include "virtualcopyjob.moc"
