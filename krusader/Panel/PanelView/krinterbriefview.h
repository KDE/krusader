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

// QtCore
#include <QVector>
// QtGui
#include <QFont>
// QtWidgets
#include <QAbstractItemView>
#include <QHeaderView>
#include <QWidget>

#include "krinterview.h"

/**
 * @brief Compact view showing only icon and file name of view items
 */
class KrInterBriefView : public QAbstractItemView, public KrInterView
{
    Q_OBJECT
public:
    KrInterBriefView(QWidget *parent, KrViewInstance &instance, KConfig *cfg);
    virtual ~KrInterBriefView();

    // ---- reimplemented from QAbstractItemView ----
    QRect visualRect(const QModelIndex&) const Q_DECL_OVERRIDE;
    QModelIndex indexAt(const QPoint&) const Q_DECL_OVERRIDE;
    void scrollTo(const QModelIndex &, QAbstractItemView::ScrollHint = QAbstractItemView::EnsureVisible) Q_DECL_OVERRIDE;

    // ---- reimplemented from KrView ----
    int  itemsPerPage() Q_DECL_OVERRIDE;
    void updateView() Q_DECL_OVERRIDE;
    bool ensureVisibilityAfterSelect() Q_DECL_OVERRIDE {
        return false;
    }
    void setSortMode(KrViewProperties::ColumnType sortColumn, bool descending) Q_DECL_OVERRIDE;

    // ---- reimplemented from QAbstractItemView ----
    // Don't do anything, selections are handled by the mouse handler
    void setSelection(const QRect &, QItemSelectionModel::SelectionFlags) Q_DECL_OVERRIDE {}
    void selectAll() Q_DECL_OVERRIDE {}
    // this shouldn't be called
    QRegion visualRegionForSelection(const QItemSelection&) const Q_DECL_OVERRIDE {
        return QRegion();
    }

    // ---- reimplemented from KrView ----
    void setFileIconSize(int size) Q_DECL_OVERRIDE;

protected slots:
    // ---- reimplemented from QAbstractItemView ----
    void updateGeometries() Q_DECL_OVERRIDE;

    // ---- reimplemented from KrView ----
    void currentChanged(const QModelIndex & current, const QModelIndex & previous) Q_DECL_OVERRIDE;

    void renameCurrentItem() Q_DECL_OVERRIDE;

protected:
    // ---- reimplemented from KrView ----
    bool handleKeyEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    // ---- reimplemented from QAbstractItemView ----
    bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    QModelIndex moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers) Q_DECL_OVERRIDE;
    int horizontalOffset() const Q_DECL_OVERRIDE;
    int verticalOffset() const Q_DECL_OVERRIDE;
    bool isIndexHidden(const QModelIndex&) const Q_DECL_OVERRIDE;
//     QRegion visualRegionForSelection(const QItemSelection&) const Q_DECL_OVERRIDE;
    bool event(QEvent * e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *ev) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *e) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *e) Q_DECL_OVERRIDE;
    void dragLeaveEvent(QDragLeaveEvent *e) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *) Q_DECL_OVERRIDE;

    // ---- reimplemented from KrView ----
    void setup() Q_DECL_OVERRIDE;
    void doRestoreSettings(KConfigGroup group) Q_DECL_OVERRIDE;
    void saveSettings(KConfigGroup grp, KrViewProperties::PropertyType properties) Q_DECL_OVERRIDE;
    void copySettingsFrom(KrView *other) Q_DECL_OVERRIDE;
    QRect itemRect(const FileItem *fileitem) Q_DECL_OVERRIDE;
    void showContextMenu(const QPoint & p) Q_DECL_OVERRIDE;
    QRect mapToViewport(const QRect &rect) const;

    int getItemHeight() const;
    int elementWidth(const QModelIndex & index);
    void intersectionSet(const QRect &, QVector<QModelIndex> &);

private:
    QFont _viewFont;
    int _numOfColumns;
    QHeaderView * _header;
};

#endif // KRINTERBRIEFVIEW_H
