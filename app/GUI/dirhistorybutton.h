/*
    SPDX-FileCopyrightText: Krusader Krew <https://krusader.org>
    SPDX-FileCopyrightText: Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: Rafi Yanai <yanai@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DIRHISTORYBUTTON_H
#define DIRHISTORYBUTTON_H

// QtWidgets
#include <QToolButton>
#include <QWidget>

class QMenu;
class QAction;
class DirHistoryQueue;

class DirHistoryButton : public QToolButton
{
    Q_OBJECT
public:
    explicit DirHistoryButton(DirHistoryQueue *hQ, QWidget *parent = nullptr);
    ~DirHistoryButton() override;

    void showMenu();

signals:
    void aboutToShow();

private:
    QMenu *popupMenu;
    DirHistoryQueue *historyQueue;

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
