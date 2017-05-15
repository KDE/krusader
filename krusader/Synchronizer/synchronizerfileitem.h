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
    SynchronizerFileItem(const QString &leftNam, const QString &rightNam, const QString &leftDir,
                         const QString &rightDir, bool mark, bool exL, bool exR,
                         KIO::filesize_t leftSize, KIO::filesize_t rightSize, time_t leftDate,
                         time_t rightDate, const QString &leftLink, const QString &rightLink,
                         const QString &leftOwner, const QString &rightOwner,
                         const QString &leftGroup, const QString &rightGroup, mode_t leftMode,
                         mode_t rightMode, const QString &leftACL, const QString &rightACL,
                         TaskType tsk, bool isDir, bool tmp, SynchronizerFileItem *parent);

    inline bool isMarked() const { return m_marked; }
    inline const QString &leftName() const { return m_leftName; }
    inline const QString &rightName() const { return m_rightName; }
    inline const QString &leftDirectory() const { return m_leftDirectory; }
    inline const QString &rightDirectory() const { return m_rightDirectory; }
    inline bool existsInLeft() const { return m_existsLeft; }
    inline bool existsInRight() const { return m_existsRight; }
    inline bool overWrite() const { return m_overWrite; }
    inline KIO::filesize_t leftSize() const { return m_leftSize; }
    inline KIO::filesize_t rightSize() const { return m_rightSize; }
    inline time_t leftDate() const { return m_leftDate; }
    inline time_t rightDate() const { return m_rightDate; }
    inline const QString &leftLink() const { return m_leftLink; }
    inline const QString &rightLink() const { return m_rightLink; }
    inline const QString &leftOwner() const { return m_leftOwner; }
    inline const QString &rightOwner() const { return m_rightOwner; }
    inline const QString &leftGroup() const { return m_leftGroup; }
    inline const QString &rightGroup() const { return m_rightGroup; }
    inline mode_t leftMode() const { return m_leftMode; }
    inline mode_t rightMode() const { return m_rightMode; }
    inline const QString &leftACL() const { return m_leftACL; }
    inline const QString &rightACL() const { return m_rightACL; }
    inline TaskType task() const { return m_task; }
    inline bool isDir() const { return m_isDir; }
    inline SynchronizerFileItem *parent() const { return m_parent; }
    inline void *userData() const { return m_userData; }
    inline const QUrl &destination() const { return m_destination; }
    inline bool isTemporary() const { return m_temporary; }
    inline TaskType originalTask() const { return m_originalTask; }

    inline void setMarked(bool flag) { m_marked = flag; }
    inline void setPermanent() { m_temporary = false; }
    inline void restoreOriginalTask() { m_task = m_originalTask; }
    inline void setUserData(void *ud) { m_userData = ud; }
    inline void setOverWrite() { m_overWrite = true; }
    inline void setDestination(QUrl d) { m_destination = d; }
    inline void setTask(TaskType t) { m_task = t; }

    void compareContentResult(bool res);
    void swap(bool asym = false);

private:
    QString m_leftName;             // the left file name
    QString m_rightName;            // the right file name
    QString m_leftDirectory;        // the left relative directory path from the base
    QString m_rightDirectory;       // the left relative directory path from the base
    bool m_marked;                  // flag, indicates to show the file
    bool m_existsLeft;              // flag, the file exists in the left directory
    bool m_existsRight;             // flag, the file exists in the right directory
    KIO::filesize_t m_leftSize;     // the file size at the left directory
    KIO::filesize_t m_rightSize;    // the file size at the right directory
    time_t m_leftDate;              // the file date at the left directory
    time_t m_rightDate;             // the file date at the left directory
    QString m_leftLink;             // the left file's symbolic link destination
    QString m_rightLink;            // the right file's symbolic link destination
    QString m_leftOwner;            // the left file's owner
    QString m_rightOwner;           // the right file's owner
    QString m_leftGroup;            // the left file's group
    QString m_rightGroup;           // the right file's group
    mode_t m_leftMode;              // mode for left
    mode_t m_rightMode;             // mode for right
    QString m_leftACL;              // ACL of the left file
    QString m_rightACL;             // ACL of the right file
    TaskType m_task;                // the task with the file
    bool m_isDir;                   // flag, indicates that the file is a directory
    SynchronizerFileItem *m_parent; // pointer to the parent directory item or 0
    void *m_userData;               // user data
    bool m_overWrite;               // overwrite flag
    QUrl m_destination;             // the destination URL at rename
    bool m_temporary;               // flag indicates temporary directory
    TaskType m_originalTask;        // the original task type
};

#endif /* __SYNCHRONIZER_FILE_ITEM_H__ */
