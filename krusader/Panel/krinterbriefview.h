/*****************************************************************************
 * Copyright (C) 2009 Csaba Karai <cskarai@freemail.hu>                      *
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

#ifndef __KRINTERBRIEFVIEW_H__
#define __KRINTERBRIEFVIEW_H__

#include <QAbstractItemView>
#include <QVector>
#include <QFont>

#include "kritemview.h"

class QHeaderView;

class KrInterBriefView : public KrItemView
{
    Q_OBJECT
public:
    KrInterBriefView(QWidget *parent, KrViewInstance &instance, const bool &left, KConfig *cfg);
    virtual ~KrInterBriefView();

    // ---- reimplemented from QAbstractItemView ----
    virtual QRect visualRect(const QModelIndex&) const;
    virtual QModelIndex indexAt(const QPoint&) const;
    virtual void scrollTo(const QModelIndex&, QAbstractItemView::ScrollHint = QAbstractItemView::EnsureVisible);

    // ---- reimplemented from KrView ----
    virtual int  itemsPerPage();
    virtual void updateView();
    virtual bool ensureVisibilityAfterSelect() {
        return false;
    }
    virtual void setSortMode(KrViewProperties::ColumnType sortColumn, bool descending);

protected slots:
    // ---- reimplemented from QAbstractItemView ----
    virtual void updateGeometries();

protected:
    // ---- reimplemented from QAbstractItemView ----
    virtual bool eventFilter(QObject *object, QEvent *event);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void wheelEvent(QWheelEvent *);
    virtual void paintEvent(QPaintEvent *e);
    virtual QModelIndex moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers);
    virtual int horizontalOffset() const;
    virtual int verticalOffset() const;
    virtual bool isIndexHidden(const QModelIndex&) const;
//     virtual QRegion visualRegionForSelection(const QItemSelection&) const;

    // ---- reimplemented from KrView ----
    virtual void setup();
    virtual void doRestoreSettings(KConfigGroup &group);
    virtual void doSaveSettings(KConfigGroup &group);
    virtual void copySettingsFrom(KrView *other);
    virtual QRect itemRect(const vfile *vf);
    virtual void showContextMenu(const QPoint & p);

    int getItemHeight() const;
    int elementWidth(const QModelIndex & index);
    void intersectionSet(const QRect &, QVector<QModelIndex> &);

private:
    QFont _viewFont;
    int _numOfColumns;
    QHeaderView * _header;
};

#endif // __KRINTERBRIEFVIEW_H__
