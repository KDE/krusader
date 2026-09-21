/*
    SPDX-FileCopyrightText: Krusader Krew <https://krusader.org>
    SPDX-FileCopyrightText: Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: Rafi Yanai <krusader@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KFNKEYS_H
#define KFNKEYS_H

// QtWidgets
#include <QGridLayout>
#include <QLayout>
#include <QPushButton>
#include <QWidget>

class KrMainWindow;

// Function Keys widget
///////////////////////
class KFnKeys : public QWidget
{
    Q_OBJECT

public:
    // constructor
    KFnKeys(QWidget *parent, KrMainWindow *mainWindow);
    void updateShortcuts();

private:
    typedef QPair<QPushButton *, QPair<QAction *, QString>> ButtonEntry;

    ButtonEntry setup(QAction *action, const QString &text);

    KrMainWindow *m_mainWindow;
    QList<ButtonEntry> buttonList;
};

#endif
