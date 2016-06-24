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

// QtGui
#include <QFont>

class KrItemView : public QAbstractItemView, public KrInterView
{
    Q_OBJECT
public:
    KrItemView(QWidget *parent, KrViewInstance &instance, KConfig *cfg);
    virtual ~KrItemView();

    // ---- reimplemented from QAbstractItemView ----
    // Don't do anything, selections are handled by the mouse handler
    virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) Q_DECL_OVERRIDE { Q_UNUSED(rect); Q_UNUSED(command); }
    virtual void selectAll() Q_DECL_OVERRIDE {}
    // this shouldn't be called
    virtual QRegion visualRegionForSelection(const QItemSelection&) const Q_DECL_OVERRIDE {
        return QRegion();
    }

    // ---- reimplemented from KrView ----
    virtual void setFileIconSize(int size) Q_DECL_OVERRIDE;

protected slots:
    // ---- reimplemented from KrView ----
    virtual void currentChanged(const QModelIndex & current, const QModelIndex & previous) Q_DECL_OVERRIDE;

    virtual void renameCurrentItem() Q_DECL_OVERRIDE;

protected:
    // ---- reimplemented from QAbstractItemView ----
    virtual bool event(QEvent * e) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    virtual void mouseDoubleClickEvent(QMouseEvent *ev) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    virtual void wheelEvent(QWheelEvent *) Q_DECL_OVERRIDE;
    virtual void dragEnterEvent(QDragEnterEvent *e) Q_DECL_OVERRIDE;
    virtual void dragMoveEvent(QDragMoveEvent *e) Q_DECL_OVERRIDE;
    virtual void dragLeaveEvent(QDragLeaveEvent *e) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *) Q_DECL_OVERRIDE;
    virtual bool viewportEvent(QEvent * event) Q_DECL_OVERRIDE;

    // ---- reimplemented from KrView ----
    virtual int elementWidth(const QModelIndex & index) = 0;
    virtual QRect mapToViewport(const QRect &rect) const;

    QFont _viewFont;
};

#endif // __KRITEMVIEW_H__
