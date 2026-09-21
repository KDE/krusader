/*
    SPDX-FileCopyrightText: Krusader Krew <https://krusader.org>
    SPDX-FileCopyrightText: Csaba Karai <krusader@users.sourceforge.net>

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
    connect(this, &QListWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        QListWidgetItem *item = itemAt(pos);
        emit itemRightClicked(item, viewport()->mapToGlobal(pos));
    });
}
