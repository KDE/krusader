// Krusader modifications:
//
// Replace: CopyJob -> PreserveAttrCopyJob
// Replace: COPYJOB -> PRESERVEATTRCOPYJOB
// Compile fixes


// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright 2000       Stephan Kulow <coolo@kde.org>
    Copyright 2000-2006  David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef PRESERVEATTRCOPYJOB_H
#define PRESERVEATTRCOPYJOB_H

#include <sys/types.h> // time_t

#include <QtCore/QObject>
#include <QtCore/QStringList>

#include <kurl.h>
#include <kio/jobclasses.h>
#include <kio/copyjob.h>

class QTimer;

enum DestinationState {
    DEST_NOT_STATED,
    DEST_IS_DIR,
    DEST_IS_FILE,
    DEST_DOESNT_EXIST
};

class Attributes
{
public:
    Attributes();
    Attributes(time_t tIn, uid_t uIn, gid_t gIn, mode_t modeIn, const QString & aclIn);
    Attributes(time_t tIn, QString user, QString group, mode_t modeIn, const QString & aclIn);

    time_t   time;
    uid_t    uid;
    gid_t    gid;
    mode_t   mode;
    QString  acl;
};

/**
 * States:
 *     STATE_STATING for the dest
 *     STATE_STATING for each src url (statNextSrc)
 *          for each: if dir -> STATE_LISTING (filling 'd->dirs' and 'd->files')
 *          but if direct rename possible: STATE_RENAMING instead.
 *     STATE_CREATING_DIRS (createNextDir, iterating over 'd->dirs')
 *          if conflict: STATE_CONFLICT_CREATING_DIRS
 *     STATE_COPYING_FILES (copyNextFile, iterating over 'd->files')
 *          if conflict: STATE_CONFLICT_COPYING_FILES
 *     STATE_DELETING_DIRS (deleteNextDir) (if moving)
 *     STATE_SETTING_DIR_ATTRIBUTES (setNextDirAttribute, iterating over d->m_directoriesCopied)
 *     done.
 */
enum PreserveAttrCopyJobState {
    STATE_STATING,
    STATE_RENAMING,
    STATE_LISTING,
    STATE_CREATING_DIRS,
    STATE_CONFLICT_CREATING_DIRS,
    STATE_COPYING_FILES,
    STATE_CONFLICT_COPYING_FILES,
    STATE_DELETING_DIRS,
    STATE_SETTING_DIR_ATTRIBUTES
};

namespace KIO
{

/**
 * PreserveAttrCopyJob is used to move, copy or symlink files and directories.
 * Don't create the job directly, but use KIO::copy(),
 * KIO::move(), KIO::link() and friends.
 *
 * @see KIO::copy()
 * @see KIO::copyAs()
 * @see KIO::move()
 * @see KIO::moveAs()
 * @see KIO::link()
 * @see KIO::linkAs()
 */
class PreserveAttrCopyJob : public Job
{

    Q_OBJECT

public:
    /**
     * Defines the mode of the operation
     */
    PreserveAttrCopyJob(const KUrl::List& src, const KUrl& dest, CopyJob::CopyMode mode, bool asMethod);
    virtual ~PreserveAttrCopyJob();

    /**
     * Returns the list of source URLs.
     * @return the list of source URLs.
     */
    KUrl::List srcUrls() const;

    /**
     * Returns the destination URL.
     * @return the destination URL
     */
    KUrl destUrl() const;

    /**
     * By default the permissions of the copied files will be those of the source files.
     *
     * But when copying "template" files to "new" files, people prefer the umask
     * to apply, rather than the template's permissions.
     * For that case, call setDefaultPermissions(true)
     */
    void setDefaultPermissions(bool b);

    /**
     * Reimplemented for internal reasons
     */
    virtual bool doSuspend();

signals:

    /**
    * Emitted when the total number of files is known.
    * @param job the job that emitted this signal
    * @param files the total number of files
    */
    void totalFiles(KJob *job, unsigned long files);
    /**
    * Emitted when the toal number of direcotries is known.
    * @param job the job that emitted this signal
    * @param dirs the total number of directories
    */
    void totalDirs(KJob *job, unsigned long dirs);

    /**
    * Emitted when it is known which files / directories are going
    * to be created. Note that this may still change e.g. when
    * existing files with the same name are discovered.
    * @param job the job that emitted this signal
    * @param files a list of items that are about to be created.
    */
    void aboutToCreate(KIO::Job *job, const QList<KIO::CopyInfo> &files);

    /**
    * Sends the number of processed files.
    * @param job the job that emitted this signal
    * @param files the number of processed files
    */
    void processedFiles(KIO::Job *job, unsigned long files);
    /**
    * Sends the number of processed directories.
    * @param job the job that emitted this signal
    * @param dirs the number of processed dirs
    */
    void processedDirs(KIO::Job *job, unsigned long dirs);

    /**
     * The job is copying a file or directory.
    * @param job the job that emitted this signal
    * @param src the URL of the file or directory that is currently
    *             being copied
    * @param dest the destination of the current operation
     */
    void copying(KIO::Job *job, const KUrl& src, const KUrl& dest);
    /**
     * The job is creating a symbolic link.
    * @param job the job that emitted this signal
    * @param target the URL of the file or directory that is currently
    *             being linked
    * @param to the destination of the current operation
     */
    void linking(KIO::Job *job, const QString& target, const KUrl& to);
    /**
     * The job is moving a file or directory.
    * @param job the job that emitted this signal
    * @param from the URL of the file or directory that is currently
    *             being moved
    * @param to the destination of the current operation
     */
    void moving(KIO::Job *job, const KUrl& from, const KUrl& to);
    /**
     * The job is creating the directory @p dir.
    * @param job the job that emitted this signal
    * @param dir the directory that is currently being created
     */
    void creatingDir(KIO::Job *job, const KUrl& dir);
    /**
     * The user chose to rename @p from to @p to.
    * @param job the job that emitted this signal
    * @param from the original name
    * @param to the new name
     */
    void renamed(KIO::Job *job, const KUrl& from, const KUrl& to);

    /**
     * The job emits this signal when copying or moving a file or directory successfully finished.
     * This signal is mainly for the Undo feature.
    *
    * @param job the job that emitted this signal
     * @param from the source URL
     * @param to the destination URL
     * @param mtime the modification time of the source file, hopefully set on the destination file
     * too (when the kioslave supports it).
     * @param directory indicates whether a file or directory was successfully copied/moved.
    *                  true for a directory, false for file
     * @param renamed indicates that the destination URL was created using a
     * rename operation (i.e. fast directory moving). true if is has been renamed
     */
    void copyingDone(KIO::Job *job, const KUrl &from, const KUrl &to, time_t mtime, bool directory, bool renamed);
    /**
     * The job is copying or moving a symbolic link, that points to target.
     * The new link is created in @p to. The existing one is/was in @p from.
     * This signal is mainly for the Undo feature.
    * @param job the job that emitted this signal
     * @param from the source URL
    * @param target the target
     * @param to the destination URL
     */
    void copyingLinkDone(KIO::Job *job, const KUrl &from, const QString& target, const KUrl& to);
protected Q_SLOTS:
    virtual void slotResult(KJob *job);

protected:
    // This is the dest URL that was initially given to PreserveAttrCopyJob
    // It is copied into m_dest, which can be changed for a given src URL
    // (when using the RENAME dialog in slotResult),
    // and which will be reset for the next src URL.
    KUrl m_globalDest;
    // The state info about that global dest
    DestinationState m_globalDestinationState;
    // See setDefaultPermissions
    bool m_defaultPermissions;
    // Whether URLs changed (and need to be emitted by the next slotReport call)
    bool m_bURLDirty;
    // Used after copying all the files into the dirs, to set mtime (TODO: and permissions?)
    // after the copy is done
    QLinkedList<CopyInfo> m_directoriesCopied;
    QLinkedList<CopyInfo>::const_iterator m_directoriesCopiedIterator;

    CopyJob::CopyMode m_mode;
    bool m_asMethod;
    DestinationState destinationState;
    PreserveAttrCopyJobState state;
    KIO::filesize_t m_totalSize;
    KIO::filesize_t m_processedSize;
    KIO::filesize_t m_fileProcessedSize;
    int m_processedFiles;
    int m_processedDirs;
    QList<CopyInfo> files;
    QList<CopyInfo> dirs;
    KUrl::List dirsToRemove;
    KUrl::List m_srcList;
    KUrl::List::Iterator m_currentStatSrc;
    bool m_bCurrentSrcIsDir;
    bool m_bCurrentOperationIsLink;
    bool m_bSingleFileCopy;
    bool m_bOnlyRenames;
    KUrl m_dest;
    KUrl m_currentDest;
    //
    QStringList m_skipList;
    QStringList m_overwriteList;
    bool m_bAutoSkip;
    bool m_bOverwriteAll;
    int m_conflictError;

    QTimer *m_reportTimer;
    //these both are used for progress dialog reporting
    KUrl m_currentSrcURL;
    KUrl m_currentDestURL;

    QMap<KUrl, Attributes> fileAttributes;
    QList<KUrl>       directoriesToStamp;
    QList<KUrl>       originalDirectories;

public:
    void statCurrentSrc();
    void statNextSrc();

    // Those aren't slots but submethods for slotResult.
    void slotResultStating(KJob * job);
    void startListing(const KUrl & src);
    void slotResultCreatingDirs(KJob * job);
    void slotResultConflictCreatingDirs(KJob * job);
    void createNextDir();
    void slotResultCopyingFiles(KJob * job);
    void slotResultConflictCopyingFiles(KJob * job);
    KIO::Job* linkNextFile(const KUrl& uSource, const KUrl& uDest, JobFlags flags);
    void copyNextFile();
    void slotResultDeletingDirs(KJob * job);
    void deleteNextDir();
    void skip(const KUrl & sourceURL);
    void slotResultRenaming(KJob * job);
    void slotResultSettingDirAttributes(KJob * job);
    void setNextDirAttribute();

    void startRenameJob(const KUrl &slave_url);
    bool shouldOverwrite(const QString& path) const;
    bool shouldSkip(const QString& path) const;
    void skipSrc();

    void saveEntries(KIO::Job*, const KIO::UDSEntryList &list);
    void slotAboutToCreate(const QList< KIO::CopyInfo > &files);
    void slotCopyingDone(const KUrl &from, const KUrl &to, bool postpone);

public slots:
    void slotStart();
    void slotEntries(KIO::Job*, const KIO::UDSEntryList& list);
    void slotFinished();
    /**
     * Forward signal from subjob
     */
    void slotProcessedSize(KJob*, qulonglong data_size);
    /**
     * Forward signal from subjob
     * @param size the total size
     */
    void slotTotalSize(KJob*, qulonglong size);

    void slotReport();
};
}

#endif
