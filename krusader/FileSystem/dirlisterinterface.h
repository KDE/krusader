/*****************************************************************************
 * Copyright (C) 2010 Jan Lepper <dehtris@yahoo.de>                          *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

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
    explicit DirListerInterface(QObject *parent) : QObject(parent) {}
    virtual ~DirListerInterface() {}

    /**
     * Return the file items of all files and directorys. Without current (".") and parent ("..")
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
