/*
    SPDX-FileCopyrightText: 2010 Jan Lepper <dehtris@yahoo.de>
    SPDX-FileCopyrightText: 2010-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DIRLISTERINTERFACE_H
#define DIRLISTERINTERFACE_H

// QtCore
#include <QObject>
#include <QUrl>

class FileItem;

/**
 * A minimal interface representing a list of files in a directory.
 */
class DirListerInterface : public QObject
{
    Q_OBJECT
public:
    explicit DirListerInterface(QObject *parent)
        : QObject(parent)
    {
    }
    ~DirListerInterface() override = default;

    /**
     * Return the file items of all files and directories. Without current (".") and parent ("..")
     * directory.
     */
    virtual QList<FileItem *> fileItems() const = 0;
    /**
     * Return the number of all file items.
     */
    virtual unsigned long numFileItems() const = 0;
    /**
     * Return true if the directory does not have a parent, else false.
     */
    virtual bool isRoot() const = 0;

signals:
    /**
     * Emitted when scanning the directory for file items finished. The list of file items should
     * now be updated by the view.
     * @param dirChange true if changed to another directory.
     */
    void scanDone(bool dirChange);
    /**
     * Emitted when all file items were removed. The file items may be deleted after this signal and
     * should not be used anymore.
     */
    void cleared();
    /**
     * Emitted when a file was added to the list of file items (not by scan).
     */
    void addedFileItem(FileItem *fileItem);
    /**
     * Emitted when a file item (with the same name) was replaced.
     * The old file item will be deleted after this signal.
     */
    void updatedFileItem(FileItem *newFileItem);
};

#endif // DIRLISTERINTERFACE_H
