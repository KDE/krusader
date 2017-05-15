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

#define REVERSE_TASK(A, asym)                                                                      \
    {                                                                                              \
        switch (A) {                                                                               \
        case TT_COPY_TO_LEFT:                                                                      \
            if (asym)                                                                              \
                A = !m_existsRight ? TT_DELETE : TT_COPY_TO_LEFT;                                  \
            else                                                                                   \
                A = TT_COPY_TO_RIGHT;                                                              \
            break;                                                                                 \
        case TT_COPY_TO_RIGHT:                                                                     \
        case TT_DELETE:                                                                            \
            A = TT_COPY_TO_LEFT;                                                                   \
        default:                                                                                   \
            break;                                                                                 \
        }                                                                                          \
    };

SynchronizerFileItem::SynchronizerFileItem(
    const QString &leftNam, const QString &rightNam, const QString &leftDir,
    const QString &rightDir, bool mark, bool exL, bool exR, KIO::filesize_t leftSize,
    KIO::filesize_t rightSize, time_t leftDate, time_t rightDate, const QString &leftLink,
    const QString &rightLink, const QString &leftOwner, const QString &rightOwner,
    const QString &leftGroup, const QString &rightGroup, mode_t leftMode, mode_t rightMode,
    const QString &leftACL, const QString &rightACL, TaskType tsk, bool isDir, bool tmp,
    SynchronizerFileItem *parent)
    : m_leftName(leftNam), m_rightName(rightNam), m_leftDirectory(leftDir),
      m_rightDirectory(rightDir), m_marked(mark), m_existsLeft(exL), m_existsRight(exR),
      m_leftSize(leftSize), m_rightSize(rightSize), m_leftDate(leftDate), m_rightDate(rightDate),
      m_leftLink(leftLink), m_rightLink(rightLink), m_leftOwner(leftOwner),
      m_rightOwner(rightOwner), m_leftGroup(leftGroup), m_rightGroup(rightGroup),
      m_leftMode(leftMode), m_rightMode(rightMode), m_leftACL(leftACL), m_rightACL(rightACL),
      m_task(tsk), m_isDir(isDir), m_parent(parent), m_userData(0), m_overWrite(false),
      m_destination(QUrl()), m_temporary(tmp), m_originalTask(tsk)
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
    SWAP(m_existsLeft, m_existsRight, bool);
    SWAP(m_leftName, m_rightName, QString);
    SWAP(m_leftDirectory, m_rightDirectory, QString);
    SWAP(m_leftSize, m_rightSize, KIO::filesize_t);
    SWAP(m_leftDate, m_rightDate, time_t);
    SWAP(m_leftLink, m_rightLink, QString);
    SWAP(m_leftOwner, m_rightOwner, QString);
    SWAP(m_leftGroup, m_rightGroup, QString);
    SWAP(m_leftACL, m_rightACL, QString);
    REVERSE_TASK(m_originalTask, asym);
    REVERSE_TASK(m_task, asym);
}
