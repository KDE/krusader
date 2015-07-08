/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
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

#ifndef KRINTERDETAILEDVIEW_H
#define KRINTERDETAILEDVIEW_H

#include <QtCore/QVector>
#include <QtGui/QFont>
#include <QtWidgets/QTreeView>

#include "krinterview.h"

class QMouseEvent;
class QKeyEvent;
class QDragEnterEvent;

class KrInterDetailedView : public QTreeView, public KrInterView
{
    Q_OBJECT

public:
    KrInterDetailedView(QWidget *parent, KrViewInstance &instance, KConfig *cfg);
    virtual ~KrInterDetailedView();

    virtual void updateView() Q_DECL_OVERRIDE;

    virtual bool ensureVisibilityAfterSelect() Q_DECL_OVERRIDE {
        return false;
    }
    virtual int  itemsPerPage() Q_DECL_OVERRIDE;
    virtual void setSortMode(KrViewProperties::ColumnType sortColumn, bool descending) Q_DECL_OVERRIDE;
    virtual void setFileIconSize(int size) Q_DECL_OVERRIDE;
    virtual void doRestoreSettings(KConfigGroup grp) Q_DECL_OVERRIDE;

protected slots:
    virtual void renameCurrentItem() Q_DECL_OVERRIDE;
    virtual void sectionResized(int, int, int);
    void sectionMoved(int, int, int);
    virtual void currentChanged(const QModelIndex & current, const QModelIndex & previous) Q_DECL_OVERRIDE;

protected:
    virtual void setup() Q_DECL_OVERRIDE;
    virtual void copySettingsFrom(KrView *other) Q_DECL_OVERRIDE;
    virtual void saveSettings(KConfigGroup grp, KrViewProperties::PropertyType properties) Q_DECL_OVERRIDE;

    // Don't do anything, selections are handled by the mouse handler
    virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) Q_DECL_OVERRIDE { Q_UNUSED(rect); Q_UNUSED(command); }
    virtual void selectAll() Q_DECL_OVERRIDE {}

    virtual void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    virtual void mouseDoubleClickEvent(QMouseEvent *ev) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    virtual void wheelEvent(QWheelEvent *) Q_DECL_OVERRIDE;
    virtual bool event(QEvent * e) Q_DECL_OVERRIDE;
    virtual void dragEnterEvent(QDragEnterEvent *e) Q_DECL_OVERRIDE;
    virtual void dragMoveEvent(QDragMoveEvent *e) Q_DECL_OVERRIDE;
    virtual void dragLeaveEvent(QDragLeaveEvent *e) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *) Q_DECL_OVERRIDE;
    virtual bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;
    virtual bool viewportEvent(QEvent * event) Q_DECL_OVERRIDE;

    virtual QRect itemRect(const vfile *vf) Q_DECL_OVERRIDE;

    virtual void showContextMenu(const QPoint & p) Q_DECL_OVERRIDE;
    virtual void recalculateColumnSizes();

private:
    QFont _viewFont;
    bool _autoResizeColumns;
};
#endif // __krinterview__
