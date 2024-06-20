/*
    SPDX-FileCopyrightText: 2003 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIRTUALFILESYSTEM_H
#define VIRTUALFILESYSTEM_H

// QtCore
#include <QHash>

#include <KConfig>

#include "filesystem.h"

/**
 * Custom virtual filesystem implementation: It holds arbitrary lists of files which are only
 * virtual references to real files. The filename of a virtual file is the full path of the real
 * file.
 *
 * Only two filesystem levels are supported: On root level only directories can be created; these
 * virtual root directories can contain a set of virtual files and directories. Entering a directory
 * on the sublevel is out of scope and the real directory will be opened.
 *
 * The filesystem content is saved in a separate config file and preserved between application runs.
 *
 * Used at least by bookmarks, locate, search and synchronizer dialog.
 */
class VirtualFileSystem : public FileSystem
{
    Q_OBJECT
public:
    VirtualFileSystem();

    /// Create virtual files in this filesystem. Copy mode and showProgressInfo are ignored.
    void copyFiles(const QList<QUrl> &urls,
                   const QUrl &destination,
                   KIO::CopyJob::CopyMode mode = KIO::CopyJob::Copy,
                   bool showProgressInfo = false,
                   JobMan::StartMode startMode = JobMan::Start) override;
    /// Handle file dropping in this filesystem: Always creates virtual files.
    void dropFiles(const QUrl &destination, QDropEvent *event, QWidget *targetWidget) override;

    /// Add virtual files to the current directory.
    void addFiles(const QList<QUrl> &fileUrls, KIO::CopyJob::CopyMode mode = KIO::CopyJob::Copy, const QString &dir = "") override;
    /// Create a virtual directory. Only possible in the root directory.
    void mkDir(const QString &name) override;
    /// Rename a (real) file in the current directory.
    void rename(const QString &fileName, const QString &newName) override;
    /// Returns the URL of the real file or an empty URL if file with name does not exist.
    QUrl getUrl(const QString &name) const override;
    bool canMoveToTrash(const QStringList &fileNames) const override;

    /// Remove virtual files or directories. Real files stay untouched.
    void remove(const QStringList &fileNames);
    /// Set meta information to be displayed in UI for the current directory
    void setMetaInformation(const QString &info);

protected:
    bool refreshInternal(const QUrl &origin, bool onlyScan) override;

private:
    /// Return current dir: "/" or pure directory name
    inline QString currentDir()
    {
        const QString path = _currentDirectory.path().mid(1); // remove slash
        return path.isEmpty() ? "/" : path;
    }
    void mkDirInternal(const QString &name);
    /// Save the dictionary to file
    void save();
    /// Restore the dictionary from file
    void restore();

    /// Create local or KIO fileItem. Returns 0 if file does not exist
    FileItem *createFileItem(const QUrl &url);

    /// Return the configuration file storing the urls of virtual files
    KConfig &getVirtDB();

private slots:
    void slotStatResult(KJob *job);

private:
    void showError(const QString &error);
    static QHash<QString, QList<QUrl> *> _virtFilesystemDict; // map virtual directories to containing files
    static QHash<QString, QString> _metaInfoDict; // map virtual directories to meta info

    QString _metaInfo; // displayable string with information about the current virtual directory
    KIO::UDSEntry _fileEntry; // for async call, save stat job result here
};

#endif
