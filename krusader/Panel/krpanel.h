/*****************************************************************************
 * Copyright (C) 2010 Jan Lepper <krusader@users.sourceforge.net>            *
 * Copyright (C) 2010-2019 Krusader Krew [https://krusader.org]              *
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


#ifndef KRPANEL_H
#define KRPANEL_H

// QtCore
#include <QUrl>

class AbstractPanelManager;
class ListPanelFunc;
class ListPanel;
class KrView;

class KrPanel
{
public:
    KrPanel(AbstractPanelManager *manager, ListPanel *panel, ListPanelFunc *func) :
        gui(panel), func(func), view(0), _manager(manager) {}
    virtual ~KrPanel() {}
    QUrl virtualPath() const; // the current directory path of this panel
    AbstractPanelManager *manager() const {
        return _manager;
    }
    KrPanel *otherPanel() const;
    bool isLeft() const;
    virtual void otherPanelChanged() = 0;

    ListPanel *const gui;
    ListPanelFunc *const func;
    KrView *view;

protected:
    AbstractPanelManager *_manager;
};

#endif
