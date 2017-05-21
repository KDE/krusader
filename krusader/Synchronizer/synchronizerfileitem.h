/*****************************************************************************
 * Copyright (C) 2006 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2006-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#ifndef SYNCHRONIZERFILEITEM_H
#define SYNCHRONIZERFILEITEM_H

// QtCore
#include <QString>
#include <QUrl>

#include <KIO/Global>

#include "../FileSystem/fileitem.h"

class SyncViewItem;

typedef enum {
    TT_EQUALS = 0,        // the files are equals     -> do nothing
    TT_DIFFERS = 1,       // the files are differents -> don't know what to do
    TT_COPY_TO_LEFT = 2,  // the right file is newer  -> copy from right to left
    TT_COPY_TO_RIGHT = 3, // the left file is newer   -> copy from left to right
    TT_DELETE = 4,        // the left file is single  -> delete it
    TT_UNKNOWN = 5,       // (5-9) the type of the task is not yet known
    TT_MAX = 10           // the maximum number of task types
} TaskType;

class SynchronizerFileItem
{
public:
    SynchronizerFileItem(const FileItem &leftFile, const FileItem &rightFile,
                         const QString &leftDir, const QString &rightDir,
                         bool mark, TaskType tsk, bool tmp, SynchronizerFileItem *parent);

    inline bool existsLeft() const { return !m_leftFile.getName().isNull(); }
    inline bool existsRight() const { return !m_rightFile.getName().isNull(); }
    inline bool isDir() const
    {
        return !m_leftFile.getName().isNull() ? m_leftFile.isDir() : m_rightFile.isDir();
    }

    inline const QString &leftName() const
    {
        return existsLeft() ? m_leftFile.getName() : m_rightFile.getName();
    }
    inline const QString &rightName() const
    {
        return existsRight() ? m_rightFile.getName() : m_leftFile.getName();
    }
    inline KIO::filesize_t leftSize() const { return m_leftFile.getSize(); }
    inline KIO::filesize_t rightSize() const { return m_rightFile.getSize(); }
    inline time_t leftDate() const { return m_leftFile.getTime_t(); }
    inline time_t rightDate() const { return m_rightFile.getTime_t(); }
    inline const QString &leftLink() const { return m_leftFile.getSymDest(); }
    inline const QString &rightLink() const { return m_rightFile.getSymDest(); }
    inline const QString &leftOwner() const { return m_leftFile.getOwner(); }
    inline const QString &rightOwner() const { return m_rightFile.getOwner(); }
    inline const QString &leftGroup() const { return m_leftFile.getGroup(); }
    inline const QString &rightGroup() const { return m_rightFile.getGroup(); }
    inline mode_t leftMode() const { return m_leftFile.getMode(); }
    inline mode_t rightMode() const { return m_rightFile.getMode(); }
    inline const QString &leftACL() { return m_leftFile.getACL(); }
    inline const QString &rightACL() { return m_rightFile.getACL(); }

    inline bool isMarked() const { return m_marked; }
    inline const QString &leftDirectory() const { return m_leftDirectory; }
    inline const QString &rightDirectory() const { return m_rightDirectory; }
    inline bool overWrite() const { return m_overWrite; }
    inline TaskType task() const { return m_task; }
    inline SynchronizerFileItem *parent() const { return m_parent; }
    inline SyncViewItem *viewItem() const { return m_viewItem; }
    inline const QUrl &destination() const { return m_destination; }
    inline bool isTemporary() const { return m_temporary; }
    inline TaskType originalTask() const { return m_originalTask; }

    inline void setMarked(bool flag) { m_marked = flag; }
    inline void setPermanent() { m_temporary = false; }
    inline void restoreOriginalTask() { m_task = m_originalTask; }
    inline void setViewItem(SyncViewItem *viewItem) { m_viewItem = viewItem; }
    inline void setOverWrite() { m_overWrite = true; }
    inline void setDestination(const QUrl &destination) { m_destination = destination; }
    inline void setTask(TaskType t) { m_task = t; }

    void compareContentResult(bool res);
    void swap(bool asym = false);

private:
    void reverseTask(TaskType &taskType, bool asym);

    FileItem m_leftFile;
    FileItem m_rightFile;
    QString m_leftDirectory;  // the left relative directory path from the base
    QString m_rightDirectory; // the left relative directory path from the base

    bool m_marked;                  // flag, indicates to show the file
    TaskType m_task;                // the task with the file
    SynchronizerFileItem *m_parent; // pointer to the parent directory item or 0
    SyncViewItem *m_viewItem;       // user data
    bool m_overWrite;               // overwrite flag
    QUrl m_destination;             // the destination URL at rename
    bool m_temporary;               // flag indicates temporary directory
    TaskType m_originalTask;        // the original task type
};

#endif /* __SYNCHRONIZER_FILE_ITEM_H__ */
