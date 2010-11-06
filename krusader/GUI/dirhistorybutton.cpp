/*****************************************************************************
 * Copyright (C) 2004 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2004 Rafi Yanai <yanai@users.sourceforge.net>               *
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

#include "dirhistorybutton.h"
#include "../Panel/dirhistoryqueue.h"

#include "../VFS/vfs.h"
#include <QtGui/QMenu>
#include <QtCore/QDir>
#include <QPixmap>
#include <klocale.h>
#include <kicon.h>

#include <kdebug.h>

DirHistoryButton::DirHistoryButton(DirHistoryQueue* hQ, QWidget *parent) : QToolButton(parent)
{
    setAutoRaise(true);
    setIcon(KIcon("chronometer"));
    setText(i18n("Open the directory history list"));
    setToolTip(i18n("Open the directory history list"));
    setPopupMode(QToolButton::InstantPopup);
    setAcceptDrops(false);

    popupMenu = new QMenu(this);
    Q_CHECK_PTR(popupMenu);

    setMenu(popupMenu);

    historyQueue = hQ;

    connect(popupMenu, SIGNAL(aboutToShow()), this, SLOT(slotAboutToShow()));
    connect(popupMenu, SIGNAL(triggered(QAction *)), this, SLOT(slotPopupActivated(QAction *)));
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
    //  kDebug() << "History slot" << endl;
}
/** No descriptions */
void DirHistoryButton::slotAboutToShow()
{
    emit aboutToShow();
    //  kDebug() << "about to show" << endl;
    popupMenu->clear();
    KUrl::List::iterator it;

    for (int i = 0; i < historyQueue->count(); i++) {
        QAction *act = popupMenu->addAction(historyQueue->get(i).prettyUrl());
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

#include "dirhistorybutton.moc"
