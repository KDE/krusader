/*
    SPDX-FileCopyrightText: 2008 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2008-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRLISTWIDGET_H
#define KRLISTWIDGET_H

// QtWidgets
#include <QListWidget>

class KrListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit KrListWidget(QWidget *parent = nullptr);

signals:
    void itemRightClicked(QListWidgetItem *it, const QPoint &pos);
};

#endif /* KRLISTVIEW_H */
