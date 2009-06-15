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

#ifndef DIRHISTORYBUTTON_H
#define DIRHISTORYBUTTON_H

#include <qwidget.h>
#include <QtGui/QToolButton>
#include <kurl.h>

class QMenu;
class QAction;
class DirHistoryQueue;

class DirHistoryButton : public QToolButton
{
    Q_OBJECT
public:
    DirHistoryButton(DirHistoryQueue* hQ, QWidget *parent = 0);
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
    void openUrl(const KUrl&);
};

#endif
