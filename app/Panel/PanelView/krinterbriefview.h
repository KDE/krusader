/*
    SPDX-FileCopyrightText: 2009 Csaba Karai <cskarai@freemail.hu>
    SPDX-FileCopyrightText: 2009-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    ~KrInterBriefView() override;

    // ---- reimplemented from QAbstractItemView ----
    QRect visualRect(const QModelIndex &) const override;
    QModelIndex indexAt(const QPoint &) const override;
    void scrollTo(const QModelIndex &, QAbstractItemView::ScrollHint = QAbstractItemView::EnsureVisible) override;

    // ---- reimplemented from KrView ----
    int itemsPerPage() override;
    void updateView() override;
    bool ensureVisibilityAfterSelect() override
    {
        return false;
    }
    void setSortMode(KrViewProperties::ColumnType sortColumn, bool descending) override;

    // ---- reimplemented from QAbstractItemView ----
    // Don't do anything, selections are handled by the mouse handler
    void setSelection(const QRect &, QItemSelectionModel::SelectionFlags) override
    {
    }
    void selectAll() override
    {
    }
    // this shouldn't be called
    QRegion visualRegionForSelection(const QItemSelection &) const override
    {
        return QRegion();
    }

    // ---- reimplemented from KrView ----
    void setFileIconSize(int size) override;

protected slots:
    // ---- reimplemented from QAbstractItemView ----
    void updateGeometries() override;

    // ---- reimplemented from KrView ----
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

    void renameCurrentItem() override;

protected:
    // ---- reimplemented from KrView ----
    bool handleKeyEvent(QKeyEvent *e) override;
    // ---- reimplemented from QAbstractItemView ----
    bool eventFilter(QObject *object, QEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    QModelIndex moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers) override;
    int horizontalOffset() const override;
    int verticalOffset() const override;
    bool isIndexHidden(const QModelIndex &) const override;
    //     QRegion visualRegionForSelection(const QItemSelection&) const override;
    bool event(QEvent *e) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void wheelEvent(QWheelEvent *) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dragLeaveEvent(QDragLeaveEvent *e) override;
    void dropEvent(QDropEvent *) override;

    // ---- reimplemented from KrView ----
    void setup() override;
    void doRestoreSettings(KConfigGroup group) override;
    void saveSettings(KConfigGroup grp, KrViewProperties::PropertyType properties) override;
    void copySettingsFrom(KrView *other) override;
    QRect itemRect(const FileItem *fileitem) override;
    void showContextMenu(const QPoint &p) override;
    QRect mapToViewport(const QRect &rect) const;

    int getItemHeight() const;
    int elementWidth(const QModelIndex &index);
    void intersectionSet(const QRect &, QVector<QModelIndex> &);

private:
    QFont _viewFont;
    int _numOfColumns;
    QHeaderView *_header;
};

#endif // KRINTERBRIEFVIEW_H
