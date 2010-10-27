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

#ifndef __KRITEMVIEW_H__
#define __KRITEMVIEW_H__

#include "krinterview.h"

#include <QFont>

class KrItemView : public QAbstractItemView, public KrInterView
{
    Q_OBJECT
public:
    KrItemView(QWidget *parent, KrViewInstance &instance, const bool &left, KConfig *cfg);
    virtual ~KrItemView();

    // ---- reimplemented from QAbstractItemView ----
    // Don't do anything, selections are handled by the mouse handler
    virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) {}
    virtual void selectAll() {}
    // this shouldn't be called
    virtual QRegion visualRegionForSelection(const QItemSelection&) const {
        return QRegion();
    }

    // ---- reimplemented from KrView ----
    virtual void setFileIconSize(int size);

protected slots:
    // ---- reimplemented from KrView ----
    virtual void currentChanged(const QModelIndex & current, const QModelIndex & previous);

    virtual void renameCurrentItem();

protected:
    // ---- reimplemented from QAbstractItemView ----
    virtual bool event(QEvent * e);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *ev);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void wheelEvent(QWheelEvent *);
    virtual void dragEnterEvent(QDragEnterEvent *e);
    virtual void dragMoveEvent(QDragMoveEvent *e);
    virtual void dragLeaveEvent(QDragLeaveEvent *e);
    virtual void dropEvent(QDropEvent *);
    virtual bool viewportEvent(QEvent * event);

    // ---- reimplemented from KrView ----
    virtual void doSaveSettings(KConfigGroup &group);
    virtual void doRestoreSettings(KConfigGroup &group);

    virtual int elementWidth(const QModelIndex & index) = 0;
    virtual QRect mapToViewport(const QRect &rect) const;

    QFont _viewFont;
};

#endif // __KRITEMVIEW_H__
