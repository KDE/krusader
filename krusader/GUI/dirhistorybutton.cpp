/*****************************************************************************
 * Copyright (C) 2004 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2004 Rafi Yanai <yanai@users.sourceforge.net>               *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#include "dirhistorybutton.h"
#include "../icon.h"
#include "../Panel/dirhistoryqueue.h"

#include "../FileSystem/filesystem.h"

// QtCore
#include <QDebug>
#include <QDir>
// QtGui
#include <QPixmap>
// QtWidgets
#include <QMenu>

#include <KI18n/KLocalizedString>

DirHistoryButton::DirHistoryButton(DirHistoryQueue* hQ, QWidget *parent) : QToolButton(parent)
{
    setAutoRaise(true);
    setIcon(Icon("chronometer"));
    setText(i18n("Open the folder history list"));
    setToolTip(i18n("Open the folder history list"));
    setPopupMode(QToolButton::InstantPopup);
    setAcceptDrops(false);

    popupMenu = new QMenu(this);
    Q_CHECK_PTR(popupMenu);

    setMenu(popupMenu);

    historyQueue = hQ;

    connect(popupMenu, SIGNAL(aboutToShow()), this, SLOT(slotAboutToShow()));
    connect(popupMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotPopupActivated(QAction*)));
}

DirHistoryButton::~DirHistoryButton() {}

void DirHistoryButton::showMenu()
{
    QMenu * pP = menu();
    if (pP) {
        menu() ->exec(mapToGlobal(QPoint(0, height())));
    }
}
/** No descriptions */
void DirHistoryButton::slotPopup()
{
//    qDebug() << "History slot";
}
/** No descriptions */
void DirHistoryButton::slotAboutToShow()
{
    emit aboutToShow();
//    qDebug() << "about to show";
    popupMenu->clear();

    for (int i = 0; i < historyQueue->count(); i++) {
        QAction *act = popupMenu->addAction(historyQueue->get(i).toDisplayString());
        act->setData(QVariant(i));
        if(historyQueue->currentPos() == i) {
            act->setCheckable(true);
            act->setChecked(true);
        }
    }
}
/** No descriptions */
void DirHistoryButton::slotPopupActivated(QAction * action)
{
    if (action && action->data().canConvert<int>()) {
        int id = action->data().toInt();
        emit gotoPos(id);
    }
}

