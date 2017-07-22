/***************************************************************************
                            filesystem.h
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

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "dirlisterinterface.h"

// QtCore
#include <QHash>
#include <QList>
#include <QString>
#include <QPointer>
// QtGui
#include <QDropEvent>
// QtWidgets
#include <QWidget>

#include <KIO/CopyJob>
#include <KIO/DirectorySizeJob>

#include "../JobMan/jobman.h"

class FileItem;

/**
 * An abstract filesystem. Use the implementations of this class for all file operations.
 *
 * It represents a directory and gives access to its files. All common file operations
 * are supported. Methods with absolute URL as argument can be used independently from the current
 * directory. Otherwise - if the methods argument is a file name - the operation is performed inside
 * the current directory.
 *
 * Notification signals are emitted if the directory content may have been changed.
 */
class FileSystem : public DirListerInterface
{
    Q_OBJECT
public:

    enum FS_TYPE {
        /// Virtual filesystem. Krusaders custom virt:/ protocol
        FS_VIRTUAL,
        /// Filesystem supporting all KIO protocols (file:/, ftp:/, smb:/, etc.)
        FS_DEFAULT
    };

    FileSystem();
    virtual ~FileSystem();

    // DirListerInterface implementation
    inline QList<FileItem *> fileItems() const { return _fileItems.values(); }
    inline unsigned long numFileItems() const { return _fileItems.count(); }
    inline bool isRoot() const {
        const QString path = _currentDirectory.path();
        return path.isEmpty() || path == "/";
    }

    /// Copy (copy, move or link) files in this filesystem.
    /// Destination is absolute URL. May implemented async.
    virtual void copyFiles(const QList<QUrl> &urls, const QUrl &destination,
                           KIO::CopyJob::CopyMode mode = KIO::CopyJob::Copy,
                           bool showProgressInfo = true,
                           JobMan::StartMode startMode = JobMan::Default) = 0;
    /// Handle file dropping in this filesystem. Destination is absolute URL. May implemented async.
    virtual void dropFiles(const QUrl &destination, QDropEvent *event) = 0;

    /// Copy (copy, move or link) files to the current filesystem directory or to "dir", the
    /// directory name relative to the current dir. May implemented async.
    virtual void addFiles(const QList<QUrl> &fileUrls, KIO::CopyJob::CopyMode mode,
                          const QString &dir = "") = 0;
    /// Create a new directory in the current directory. May implemented async.
    virtual void mkDir(const QString &name) = 0;
    /// Rename file/directory in the current directory. May implemented async.
    virtual void rename(const QString &fileName, const QString &newName) = 0;

    /// Return an absolute URL for a single file/directory name in the current directory - with no
    /// trailing slash.
    virtual QUrl getUrl(const QString &name) const = 0;
    /// Return a list of URLs for multiple files/directories in the current directory.
    QList<QUrl> getUrls(const QStringList &names) const;
    /// Return true if all files can be moved to trash, else false.
    virtual bool canMoveToTrash(const QStringList &fileNames) const = 0;

    /// Return the filesystem mount point of the current directory. Empty string by default.
    virtual QString mountPoint() const { return QString(); }
    /// Returns true if this filesystem implementation does not need to be notified about changes in the
    /// current directory. Else false.
    virtual bool hasAutoUpdate() const { return false; }
    /// Notify this filesystem that the filesystem info of the current directory may have changed.
    virtual void updateFilesystemInfo() {}

    /**
     * Scan all files and directories in a directory and create the file items for them. Blocking.
     *
     * @param directory if given, the lister tries to change to this directory, else the old
     * directory is refreshed
     * @return true if scan was successful, else (not implemented, scan failed or refresh job
     * was killed) false.
     */
    bool scanDir(const QUrl &directory = QUrl()) { return scanOrRefresh(directory, false); }

    /// Change or refresh the current directory and scan it. Blocking.
    /// Returns true if directory was scanned. Returns false if failed or scan job was killed.
    bool refresh(const QUrl &directory = QUrl()) { return scanOrRefresh(directory, false); }

    /// Returns the current directory path of this filesystem.
    inline QUrl currentDirectory() const { return _currentDirectory; }
    /// Return the file item for a file name in the current directory. Or 0 if not found.
    FileItem *getFileItem(const QString &name) const;
    /// The total size of all files in the current directory (only valid after scan).
    // TODO unused
    KIO::filesize_t totalSize() const;
    /// Return the filesystem type.
    inline FS_TYPE type() const { return _type; }
    /// Return true if the current directory is local (without recognizing mount points).
    inline bool isLocal() const { return _currentDirectory.isLocalFile(); }
    /// Return true if the current directory is a remote (network) location.
    inline bool isRemote() const {
        const QString sc = _currentDirectory.scheme();
        return (sc == "fish" || sc == "ftp" || sc == "sftp" || sc == "nfs" || sc == "smb"
                || sc == "webdav");
    }
    /// Returns true if this filesystem is currently refreshing the current directory.
    inline bool isRefreshing() const { return _isRefreshing; }

    /// Delete or trash files in the current directory. Implemented async.
    void deleteFiles(const QStringList &fileNames, bool moveToTrash = true);
    /// Delete or trash arbitrary files. Implemented async. Universal refresh not fully implemented.
    void deleteAnyFiles(const QList<QUrl> &urls, bool moveToTrash);

    /// Return the input URL with a trailing slash if absent.
    static QUrl ensureTrailingSlash(const QUrl &url);
    /// Return the input URL without trailing slash.
    static QUrl cleanUrl(const QUrl &url) {
        return url.adjusted(QUrl::StripTrailingSlash);
    }
    /// Add 'file' scheme to non-empty URL without scheme
    static QUrl preferLocalUrl(const QUrl &url);

    /// Return a file item for a local file inside a directory
    static FileItem *createLocalFileItem(const QString &name, const QString &directory,
                                   bool virt = false);
    /// Return a file item for a KIO result. Returns 0 if entry is not needed
    static FileItem *createFileItemFromKIO(const KIO::UDSEntry &entry, const QUrl &directory,
                                     bool virt = false);

    // set the parent window to be used for dialogs
    void setParentWindow(QWidget *widget) { parentWindow = widget; }

signals:
    /// Emitted when this filesystem is currently refreshing the filesystem directory.
    void refreshJobStarted(KIO::Job *job);
    /// Emitted when an error occurred in this filesystem during refresh.
    void error(const QString &msg);
    /// Emitted when the content of a directory was changed by this filesystem.
    void fileSystemChanged(const QUrl &directory);
    /// Emitted when the information for the filesystem of the current directory changed.
    /// Information is either
    /// * 'metaInfo': a displayable string about the fs, empty by default, OR
    /// * 'fsType', 'total' and 'free': filesystem type, size and free space,
    ///   empty string or 0 by default
    void fileSystemInfoChanged(const QString &metaInfo,  const QString &fsType,
                               KIO::filesize_t total, KIO::filesize_t free);
    /// Emitted before a directory path is opened for reading. Used for automounting.
    void aboutToOpenDir(const QString &path);

protected:
    /// Fill the filesystem dictionary with file items, must be implemented for each filesystem.
    virtual bool refreshInternal(const QUrl &origin, bool stayInDir) = 0;

    /// Connect the result signal of a file operation job - source URLs.
    void connectJobToSources(KJob *job, const QList<QUrl> urls);
    /// Connect the result signal of a file operation job - destination URL.
    void connectJobToDestination(KJob *job, const QUrl &destination);
    /// Returns true if showing hidden files is set in config.
    bool showHiddenFiles();
    /// Add a new file item to the internal dictionary (while refreshing).
    void addFileItem(FileItem *item);

    FS_TYPE _type;          // the filesystem type.
    QUrl _currentDirectory; // the path or file the filesystem originates from.
    bool _isRefreshing;     // true if filesystem is busy with refreshing
    QPointer<QWidget> parentWindow;

protected slots:
    /// Handle result after job (except when refreshing!) finished
    void slotJobResult(KJob *job, bool refresh);

private:
    typedef QHash<QString, FileItem *> FileItemDict;

    // optional TODO: add an async version of this
    bool scanOrRefresh(const QUrl &directory, bool onlyScan);

    /// Delete and clear file items.
    void clear(FileItemDict &fileItems);

    FileItemDict _fileItems;  // the list of files in the current dictionary
};

#endif
