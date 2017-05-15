/*****************************************************************************
 * Copyright (C) 2017 Krusader Krew                                          *
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

#include <synchronizerfileitem.h>

#define SWAP(A, B, TYPE)                                                                           \
    {                                                                                              \
        TYPE TMP = A;                                                                              \
        A = B;                                                                                     \
        B = TMP;                                                                                   \
    }

SynchronizerFileItem::SynchronizerFileItem(const FileItem &leftFile, const FileItem &rightFile,
                                           const QString &leftDir, const QString &rightDir,
                                           bool mark, TaskType tsk, bool tmp,
                                           SynchronizerFileItem *parent)
    : m_leftFile(leftFile), m_rightFile(rightFile), m_leftDirectory(leftDir),
      m_rightDirectory(rightDir), m_marked(mark), m_task(tsk), m_parent(parent), m_viewItem(0),
      m_overWrite(false), m_destination(QUrl()), m_temporary(tmp), m_originalTask(tsk)
{
}

void SynchronizerFileItem::compareContentResult(bool res)
{
    if (res == true)
        m_task = m_originalTask = TT_EQUALS;
    else if (m_originalTask >= TT_UNKNOWN)
        m_task = m_originalTask = (TaskType)(m_originalTask - TT_UNKNOWN);
}

void SynchronizerFileItem::swap(bool asym)
{
    SWAP(m_leftFile, m_rightFile, FileItem);
    SWAP(m_leftDirectory, m_rightDirectory, QString);
    reverseTask(m_originalTask, asym);
    reverseTask(m_task, asym);
}

void SynchronizerFileItem::reverseTask(TaskType &taskType, bool asym)
{
    switch (taskType) {
    case TT_COPY_TO_LEFT:
        if (asym)
            taskType = !existsInRight() ? TT_DELETE : TT_COPY_TO_LEFT;
        else
            taskType = TT_COPY_TO_RIGHT;
        break;
    case TT_COPY_TO_RIGHT:
    case TT_DELETE:
        taskType = TT_COPY_TO_LEFT;
    default:
        break;
    }
}
