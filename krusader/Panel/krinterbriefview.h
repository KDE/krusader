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

#ifndef KRINTERBRIEFVIEW_H
#define KRINTERBRIEFVIEW_H

#include <QAbstractItemView>
#include <QVector>
#include <QFont>

#include "krinterview.h"

class KrInterBriefViewItem;
class KrMouseHandler;
class KrVfsModel;
class QHeaderView;

class KrInterBriefView : public QAbstractItemView, public KrInterView
{
    Q_OBJECT
    friend class KrInterBriefViewItem;

public:
    KrInterBriefView(QWidget *parent, const bool &left, KConfig *cfg);
    virtual ~KrInterBriefView();

    virtual void updateView();

    virtual bool ensureVisibilityAfterSelect() {
        return false;
    }
    virtual int  itemsPerPage();
    virtual void setSortMode(KrViewProperties::ColumnType sortColumn, bool descending);
    virtual void setFileIconSize(int size);

    // abstract item view classes
    virtual QRect visualRect(const QModelIndex&) const;
    virtual void scrollTo(const QModelIndex&, QAbstractItemView::ScrollHint = QAbstractItemView::EnsureVisible);
    virtual QModelIndex indexAt(const QPoint&) const;
    virtual QModelIndex moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers);
    virtual int horizontalOffset() const;
    virtual int verticalOffset() const;
    virtual bool isIndexHidden(const QModelIndex&) const;
    // Don't do anything, selections are handled by the mouse handler
    virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) {}
    virtual void selectAll() {}

    virtual QRegion visualRegionForSelection(const QItemSelection&) const;
    virtual void updateGeometries();
    virtual QRect mapToViewport(const QRect &rect) const;

protected slots:
    virtual void renameCurrentItem();
    virtual void currentChanged(const QModelIndex & current, const QModelIndex & previous);

protected:
    virtual void setup();
    virtual void initOperator();

    virtual void doRestoreSettings(KConfigGroup &group);
    virtual void doSaveSettings(KConfigGroup &group);
    virtual void copySettingsFrom(KrView *other);

    virtual void keyPressEvent(QKeyEvent *e);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *ev);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void wheelEvent(QWheelEvent *);
    virtual bool event(QEvent * e);
    virtual void dragEnterEvent(QDragEnterEvent *e);
    virtual void dragMoveEvent(QDragMoveEvent *e);
    virtual void dragLeaveEvent(QDragLeaveEvent *e);
    virtual void dropEvent(QDropEvent *);
    virtual bool eventFilter(QObject *object, QEvent *event);
    virtual bool viewportEvent(QEvent * event);
    virtual void paintEvent(QPaintEvent *e);

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

#endif
