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

#include <QTreeView>
#include <QVector>
#include <QFont>

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

    virtual void updateView();

    virtual bool ensureVisibilityAfterSelect() {
        return false;
    }
    virtual int  itemsPerPage();
    virtual void setSortMode(KrViewProperties::ColumnType sortColumn, bool descending);
    virtual void setFileIconSize(int size);
    virtual void doRestoreSettings(KConfigGroup grp);

protected slots:
    virtual void renameCurrentItem();
    virtual void sectionResized(int, int, int);
    void sectionMoved(int, int, int);
    virtual void currentChanged(const QModelIndex & current, const QModelIndex & previous);

protected:
    virtual void setup();
    virtual void copySettingsFrom(KrView *other);
    virtual void saveSettings(KConfigGroup grp, KrViewProperties::PropertyType properties);

    // Don't do anything, selections are handled by the mouse handler
    virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) { Q_UNUSED(rect); Q_UNUSED(command); }
    virtual void selectAll() {}

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

    virtual QRect itemRect(const vfile *vf);

    virtual void showContextMenu(const QPoint & p);
    virtual void recalculateColumnSizes();

private:
    QFont _viewFont;
    bool _autoResizeColumns;
};
#endif // __krinterview__
