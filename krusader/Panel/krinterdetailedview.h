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

// QtCore
#include <QVector>
// QtGui
#include <QFont>
// QtWidgets
#include <QTreeView>

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

    void updateView() Q_DECL_OVERRIDE;

    bool ensureVisibilityAfterSelect() Q_DECL_OVERRIDE {
        return false;
    }
    int  itemsPerPage() Q_DECL_OVERRIDE;
    void setSortMode(KrViewProperties::ColumnType sortColumn, bool descending) Q_DECL_OVERRIDE;
    void setFileIconSize(int size) Q_DECL_OVERRIDE;
    void doRestoreSettings(KConfigGroup grp) Q_DECL_OVERRIDE;

protected slots:
    void renameCurrentItem() Q_DECL_OVERRIDE;
    void sectionResized(int, int, int);
    void sectionMoved(int, int, int);
    void currentChanged(const QModelIndex & current, const QModelIndex & previous) Q_DECL_OVERRIDE;

protected:
    void setup() Q_DECL_OVERRIDE;
    void copySettingsFrom(KrView *other) Q_DECL_OVERRIDE;
    void saveSettings(KConfigGroup grp, KrViewProperties::PropertyType properties) Q_DECL_OVERRIDE;

    // Don't do anything, selections are handled by the mouse handler
    void setSelection(const QRect &, QItemSelectionModel::SelectionFlags) Q_DECL_OVERRIDE {}
    void selectAll() Q_DECL_OVERRIDE {}

    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *ev) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *) Q_DECL_OVERRIDE;
    bool event(QEvent * e) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *e) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *e) Q_DECL_OVERRIDE;
    void dragLeaveEvent(QDragLeaveEvent *e) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;
    bool viewportEvent(QEvent * event) Q_DECL_OVERRIDE;
    void drawRow(QPainter *painter, const QStyleOptionViewItem &options,
                 const QModelIndex &index) const Q_DECL_OVERRIDE;

    QRect itemRect(const FileItem *fileitem) Q_DECL_OVERRIDE;

    void showContextMenu(const QPoint & p) Q_DECL_OVERRIDE;
    void recalculateColumnSizes();

private:
    QFont _viewFont;
    bool _autoResizeColumns;
};
#endif // __krinterview__
