/*
    SPDX-FileCopyrightText: Krusader Krew <https://krusader.org>
    SPDX-FileCopyrightText: Jan Lepper <krusader@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    KrPanel(AbstractPanelManager *manager, ListPanel *panel, ListPanelFunc *panelCallback)
        : gui(panel)
        , func(panelCallback)
        , view(nullptr)
        , _manager(manager)
    {
    }
    virtual ~KrPanel()
    {
    }
    QUrl virtualPath() const; // the current directory path of this panel
    AbstractPanelManager *manager() const
    {
        return _manager;
    }
    KrPanel *otherPanel() const;
    bool isLeft() const;
    virtual void otherPanelChanged() = 0;

    ListPanel *const gui;
    ListPanelFunc *const func;
    KrView *view; // the view of file items in this panel

protected:
    AbstractPanelManager *_manager;
};

#endif
