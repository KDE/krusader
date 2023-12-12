/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

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

    KrMainWindow *mainWindow;
    QList<ButtonEntry> buttonList;
};

#endif
