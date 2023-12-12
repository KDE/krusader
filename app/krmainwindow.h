/*
    SPDX-FileCopyrightText: 2010 Jan Lepper <dehtris@yahoo.de>
    SPDX-FileCopyrightText: 2010-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRMAINWINDOW_H
#define KRMAINWINDOW_H

#include <QAction>
#include <QWidget>

#include "abstractpanelmanager.h"

class KActionCollection;
class KrPanel;
class KrActions;
class KrView;
class ListPanelActions;
class PopularUrls;
class TabActions;
class ViewActions;

/**
 * @brief Abstract file manager main window
 */
class KrMainWindow
{
public:
    virtual ~KrMainWindow()
    {
    }
    virtual QWidget *widget() = 0;
    virtual KrView *activeView() = 0;
    virtual ViewActions *viewActions() = 0;
    virtual KActionCollection *actions() = 0;
    virtual AbstractPanelManager *activeManager() = 0;
    virtual AbstractPanelManager *leftManager() = 0;
    virtual AbstractPanelManager *rightManager() = 0;
    virtual PopularUrls *popularUrls() = 0;
    virtual KrActions *krActions() = 0;
    virtual ListPanelActions *listPanelActions() = 0;
    virtual TabActions *tabActions() = 0;
    virtual void plugActionList(const char *name, QList<QAction *> &list) = 0;

    KrPanel *activePanel()
    {
        return activeManager()->currentPanel();
    }
    KrPanel *leftPanel()
    {
        return leftManager()->currentPanel();
    }
    KrPanel *rightPanel()
    {
        return rightManager()->currentPanel();
    }
};

#endif // KRMAINWINDOW_H
