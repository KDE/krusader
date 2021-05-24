/*
    SPDX-FileCopyrightText: 2010 Jan Lepper <dehtris@yahoo.de>
    SPDX-FileCopyrightText: 2010-2020 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ABSTRACTPANELMANAGER_H
#define ABSTRACTPANELMANAGER_H

// QtCore
#include <QUrl>

class KrPanel;

class AbstractPanelManager
{
public:
    virtual ~AbstractPanelManager() {}
    virtual bool isLeft() const = 0;
    virtual AbstractPanelManager *otherManager() const = 0;
    virtual KrPanel *currentPanel() const = 0;
    virtual void newTab(const QUrl&, KrPanel *nextTo = 0) = 0;
};


#endif // ABSTRACTPANELMANAGER_H
