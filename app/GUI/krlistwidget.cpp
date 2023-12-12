/*
    SPDX-FileCopyrightText: 2008 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2008-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krlistwidget.h"
#include "krstyleproxy.h"

// QtGui
#include <QContextMenuEvent>

KrListWidget::KrListWidget(QWidget *parent)
    : QListWidget(parent)
{
    auto *style = new KrStyleProxy();
    style->setParent(this);
    setStyle(style);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QListWidget::customContextMenuRequested, [=](const QPoint &pos) {
        QListWidgetItem *item = itemAt(pos);
        emit itemRightClicked(item, viewport()->mapToGlobal(pos));
    });
}
