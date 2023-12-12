/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    ~KrInterDetailedView() override;

    void updateView() override;

    bool ensureVisibilityAfterSelect() override
    {
        return false;
    }
    int itemsPerPage() override;
    void setSortMode(KrViewProperties::ColumnType sortColumn, bool descending) override;
    void setFileIconSize(int size) override;
    void doRestoreSettings(KConfigGroup grp) override;

protected slots:
    void renameCurrentItem() override;
    void sectionResized(int, int, int);
    void sectionMoved(int, int, int);
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

protected:
    void setup() override;
    void copySettingsFrom(KrView *other) override;
    void saveSettings(KConfigGroup grp, KrViewProperties::PropertyType properties) override;

    // Don't do anything, selections are handled by the mouse handler
    void setSelection(const QRect &, QItemSelectionModel::SelectionFlags) override
    {
    }
    void selectAll() override
    {
    }

    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void wheelEvent(QWheelEvent *) override;
    bool event(QEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dragLeaveEvent(QDragLeaveEvent *e) override;
    void dropEvent(QDropEvent *) override;
    bool eventFilter(QObject *object, QEvent *event) override;
    bool viewportEvent(QEvent *event) override;
    void drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;

    QRect itemRect(const FileItem *fileitem) override;

    void showContextMenu(const QPoint &p) override;
    void recalculateColumnSizes();

private:
    QFont _viewFont;
    bool _autoResizeColumns;
};
#endif // __krinterview__
