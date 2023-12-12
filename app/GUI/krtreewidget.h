/*
    SPDX-FileCopyrightText: 2008 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2008-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRTREEWIDGET_H
#define KRTREEWIDGET_H

// QtWidgets
#include <QAbstractScrollArea>
#include <QTreeWidget>

class KrTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit KrTreeWidget(QWidget *parent);
    void setStretchingColumn(int col)
    {
        _stretchingColumn = col;
    }

    QModelIndex indexOf(QTreeWidgetItem *item, int col = 0)
    {
        return indexFromItem(item, col);
    }
    QTreeWidgetItem *item(const QModelIndex &ndx)
    {
        return itemFromIndex(ndx);
    }

signals:
    void itemRightClicked(QTreeWidgetItem *it, const QPoint &pos, int column);

protected:
    bool event(QEvent *event) override;

private:
    int _stretchingColumn;
    bool _inResize;
};

#endif /* KRTREEWIDGET_H */
