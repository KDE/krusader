/***************************************************************************
                            vfs.h
                       -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ***************************************************************************

   A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef VFS_H
#define VFS_H

#include "vfilecontainer.h"

// QtCore
#include <QString>
#include <QList>
#include <QObject>
#include <QHash>
#include <QPointer>
#include <QUrl>
// QtGui
#include <QDropEvent>
// QtWidgets
#include <QWidget>

#include <KIO/CopyJob>

#include "vfile.h"
#include "krquery.h"


/**
 * An abstract virtual filesystem. Use the implementations of this class for all file operations.
 *
 * It represents a directory and gives access to its files. All common file operations
 * are supported. Methods with absolute URL as argument can be used independently from the current
 * directory. Otherwise - if the methods argument is a file name - the operation is performed inside
 * the current directory.
 *
 * Notification signals are emitted if the directory content may have been changed.
 */
class vfs : public VfileContainer
{
    Q_OBJECT
public:
    typedef QHash<QString, vfile *> vfileDict;

    enum VFS_TYPE {
        /// Virtual filesystem. Krusaders custom virt:/ protocol
        VFS_VIRT,
        /// Filesystem supporting all KIO protocols (file:/, ftp:/, smb:/, etc.)
        VFS_DEFAULT
    };

    vfs();
    virtual ~vfs();

    // VfileContainer implementation
    virtual inline QList<vfile *> vfiles() { return _vfiles.values(); }
    virtual inline unsigned long numVfiles() { return _vfiles.count(); }
    virtual inline bool isRoot() {
        const QString path = _currentDirectory.path();
        return path.isEmpty() || path == "/";
    }

    /// Copy (copy, move or link) files in this VFS.
    /// Destination is absolute URL. May implemented async.
    virtual void copyFiles(const QList<QUrl> &urls, const QUrl &destination,
                           KIO::CopyJob::CopyMode mode = KIO::CopyJob::Copy,
                           bool showProgressInfo = true, bool enqueue = false) = 0;
    /// Handle file dropping in this VFS. Destination is absolute URL. May implemented async.
    virtual void dropFiles(const QUrl &destination, QDropEvent *event) = 0;

    /// Copy (copy, move or link) files to the current VFS directory or to "dir", the
    /// directory name relative to the current dir. May implemented async.
    virtual void addFiles(const QList<QUrl> &fileUrls, KIO::CopyJob::CopyMode mode,
                          QString dir = "") = 0;
    /// Delete or move a file in the current directory to trash. May implemented async.
    virtual void deleteFiles(const QStringList &fileNames, bool reallyDelete = false) = 0;
    /// Create a new directory in the current directory. May implemented async.
    virtual void mkDir(const QString &name) = 0;
    /// Rename file/directory in the current directory. May implemented async.
    virtual void rename(const QString &fileName, const QString &newName) = 0;

    /// Return an absolute URL for a single file/directory name in the current directory - with no
    /// trailing slash.
    virtual QUrl getUrl(const QString &name) = 0;
    /// Return a list of URLs for multiple files/directories in the current directory.
    virtual QList<QUrl> getUrls(const QStringList &names);

    /// Returns the current directory path of this VFS.
    inline QUrl currentDirectory() { return _currentDirectory; }
    /// Return the vfile for a file name in the current directory. Or 0 if not found.
    inline vfile *getVfile(const QString &name) { return (_vfiles)[name]; }
    /// Return a list of vfiles for a search query. Or an empty list if nothing was found.
    QList<vfile *> searchVfiles(const KRQuery &filter);
    /// The total size of all files in the current directory (only valid after refresh).
    // TODO unused
    KIO::filesize_t vfs_totalSize();
    /// Return the VFS type.
    inline VFS_TYPE type() { return _type; }
    /// Return true if the current directory is local (without recognizing mount points).
    inline bool isLocal() { return _currentDirectory.isLocalFile(); }
    /// Return true if the current directory is a remote (network) location.
    inline bool isRemote() {
        const QString sc = _currentDirectory.scheme();
        return (sc == "fish" || sc == "ftp" || sc == "sftp" || sc == "nfs" || sc == "smb"
                || sc == "webdav");
    }
    /// Returns true if this VFS is currently refreshing the current directory.
    inline bool isRefreshing() { return _isRefreshing; }
    /// Return a displayable string containing special filesystem meta information. Or an empty
    /// string by default.
    virtual QString metaInformation() { return QString(); }

    /// Calculate the amount of space occupied by a file or directory in the current directory
    /// (recursive).
    virtual void calcSpace(const QString &name, KIO::filesize_t *totalSize,
                           unsigned long *totalFiles, unsigned long *totalDirs, bool *stop);

    /// Return the input URL with a trailing slash if absent.
    static QUrl ensureTrailingSlash(const QUrl &url);
    /// Return the input URL without trailing slash.
    static QUrl cleanUrl(const QUrl &url) {
        return url.adjusted(QUrl::StripTrailingSlash);
    }
    /// Add 'file' scheme to non-empty URL without scheme
    static QUrl preferLocalUrl(const QUrl &url);

public slots:
    /// Re-read the current directory files or change to another directory. Blocking.
    /// If the directory was read true is returned else false.
    // optional TODO: add an async version of this
    bool refresh(const QUrl &directory = QUrl());
    /// Notify this VFS that the current directory content may have changed.
    void mayRefresh();

signals:
    /// Emitted when this VFS is currently refreshing the VFS directory.
    void refreshJobStarted(KIO::Job *job);
    /// Emitted when an error occured in this VFS.
    /// The error can be caused by refresh or any filesystem operation
    void error(const QString &msg);
    /// Emitted when the content of a directory was changed by this VFS.
    void filesystemChanged(const QUrl &directory);
    /// Emitted when a new file operation job was created. The job may be prepared to be queued
    /// i.e. in suspend state.
    void newJob(KIO::Job *job);
    /// Emitted before a directory path is opened for reading. Used for automounting.
    void aboutToOpenDir(const QString &path);

protected:
    /// Fill the vfs dictionary with vfiles, must be implemented for each VFS.
    virtual bool refreshInternal(const QUrl &origin, bool showHidden) = 0;
    /// Returns true if this VFS implementation does not need to be notified about changes in the
    /// current directory.
    virtual bool ignoreRefresh() { return false; }

    /// Returns true if showing hidden files is set in config.
    bool showHiddenFiles();
    /// Add a new vfile to the internal dictionary (while refreshing).
    inline void addVfile(vfile *vf) { _vfiles.insert(vf->vfile_getName(), vf); }

    /// Calculate the size of a file or directory (recursive).
    void calcSpace(const QUrl &url, KIO::filesize_t *totalSize, unsigned long *totalFiles,
                        unsigned long *totalDirs, bool *stop);
    /// Calculate the size of a local file or directory (recursive).
    void calcSpaceLocal(const QString &path, KIO::filesize_t *totalSize, unsigned long *totalFiles,
                        unsigned long *totalDirs, bool *stop);
    /// Calculate the size of any KIO file or directory.
    void calcSpaceKIO(const QUrl &url, KIO::filesize_t *totalSize, unsigned long *totalFiles,
                      unsigned long *totalDirs, bool *stop);

    /// Return a vfile for a local file inside a directory
    static vfile *createLocalVFile(const QString &name, const QString &directory,
                                   bool virt = false);
    /// Return a vfile for a KIO result. Returns 0 if entry is not needed
    static vfile *createVFileFromKIO(const KIO::UDSEntry &_calcEntry, const QUrl &directory,
                                     bool virt = false);

    VFS_TYPE _type;         // the vfs type.
    QUrl _currentDirectory; // the path or file the VFS originates from.
    bool _isRefreshing; // true if vfs is busy with refreshing

protected slots:
    /// Handle result after job (except when refreshing!) finished
    void slotJobResult(KJob *job, bool refresh = false);

private slots:
    /// Handle result of KIO::DirectorySizeJob when calculating URL size
    void slotCalcKdsResult(KJob *job);
    /// Handle result of KIO::StatJob when calculating URL size
    void slotCalcStatResult(KJob *job);

private:
    /// Delete and clear vfiles.
    void clear(vfileDict &vfiles);

    vfileDict _vfiles;  // The list of files in the current dictionary

    // used in the calcSpace function
    bool *_calcKdsBusy;
    bool _calcStatBusy;
    KIO::UDSEntry _calcEntry;
    KIO::filesize_t *_calcKdsTotalSize;
    unsigned long *_calcKdsTotalFiles;
    unsigned long *_calcKdsTotalDirs;

};

#endif
