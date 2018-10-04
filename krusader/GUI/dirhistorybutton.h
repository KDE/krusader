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

#ifndef DIRHISTORYBUTTON_H
#define DIRHISTORYBUTTON_H

// QtWidgets
#include <QWidget>
#include <QToolButton>

class QMenu;
class QAction;
class DirHistoryQueue;

class DirHistoryButton : public QToolButton
{
    Q_OBJECT
public:
    explicit DirHistoryButton(DirHistoryQueue* hQ, QWidget *parent = nullptr);
    ~DirHistoryButton();

    void showMenu();

signals:
    void aboutToShow();

private:
    QMenu* popupMenu;
    DirHistoryQueue* historyQueue;

public slots: // Public slots
    /** No descriptions */
    void slotPopup();
    /** No descriptions */
    void slotAboutToShow();
    /** No descriptions */
    void slotPopupActivated(QAction *);
signals: // Signals
    /** No descriptions */
    void gotoPos(int pos);
};

#endif
