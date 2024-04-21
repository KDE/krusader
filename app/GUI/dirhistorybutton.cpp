/*
    SPDX-FileCopyrightText: 2004 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dirhistorybutton.h"
#include "../Panel/dirhistoryqueue.h"
#include "../icon.h"

#include "../FileSystem/filesystem.h"

// QtCore
#include <QDebug>
#include <QDir>
// QtGui
#include <QPixmap>
// QtWidgets
#include <QMenu>

#include <KLocalizedString>

DirHistoryButton::DirHistoryButton(DirHistoryQueue *hQ, QWidget *parent)
    : QToolButton(parent)
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

    connect(popupMenu, &QMenu::aboutToShow, this, &DirHistoryButton::slotAboutToShow);
    connect(popupMenu, &QMenu::triggered, this, &DirHistoryButton::slotPopupActivated);
}

DirHistoryButton::~DirHistoryButton() = default;

void DirHistoryButton::showMenu()
{
    QMenu *pP = menu();
    if (pP) {
        menu()->exec(mapToGlobal(QPoint(0, height())));
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
        if (historyQueue->currentPos() == i) {
            act->setCheckable(true);
            act->setChecked(true);
        }
    }
}
/** No descriptions */
void DirHistoryButton::slotPopupActivated(QAction *action)
{
    if (action && action->data().canConvert<int>()) {
        int id = action->data().toInt();
        emit gotoPos(id);
    }
}
