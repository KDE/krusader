/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
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

#ifndef PANELMANAGER_H
#define PANELMANAGER_H

#include "abstractpanelmanager.h"

#include <qwidget.h>
#include <QtGui/QLayout>
#include <QGridLayout>
#include <QHBoxLayout>

#include <kconfiggroup.h>

#include "paneltabbar.h"

class KConfig;
class ListPanel;
class QStackedWidget;
class QToolButton;

/**
 * Implements tabbed-browsing by managing a list of tabs and corrosponding panels.
 */
class PanelManager: public QWidget, public AbstractPanelManager
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.krusader.PanelManager")

public:
    /**
     * PanelManager is created where once panels were created. It accepts three references to pointers
     * (self, other, active), which enables it to manage pointers held by the panels transparently.
     * It also receives a bool (left) which is true if the manager is the left one, or false otherwise.
     */
    PanelManager(QWidget *parent, bool left);
    /**
     * Called once by KrusaderView to create the first panel. Subsequent called are done internally
     * Note: only creates the panel, but doesn't start the VFS inside it. Use startPanel() for that.
     */
    ListPanel* createPanel(bool setCurrent = true, KConfigGroup cfg = KConfigGroup());
    /**
     * Swaps the left / right directions of the panel
     */
    void swapPanels();

    void saveSettings(KConfigGroup config, bool localOnly = true, bool saveHistory = false);
    void loadSettings(KConfigGroup config);
    int findTab(KUrl url);
    int activeTab();
    void setActiveTab(int);
    void setCurrentTab(int);
    void refreshAllTabs(bool invalidate = false);
    void layoutTabs();

    // AbstractPanelManager implementation
    virtual bool isLeft() {
        return _left;
    }
    virtual AbstractPanelManager *otherManager();
    virtual KrPanel *currentPanel();
    virtual void newTab(const KUrl &url) {
        slotNewTab(url);
    }

public slots:
    /**
     * Called externally to start a new tab. Example of usage would be the "open in a new tab"
     * action, from the context-menu.
     */

    Q_SCRIPTABLE void newTab(const QString& url) {
        slotNewTab(KUrl(url));
    }
    Q_SCRIPTABLE void newTabs(const QStringList& urls);

    void slotNewTab(const KUrl& url, bool setCurrent = true);
    void slotNewTab();
    void slotLockTab();
    void slotNextTab();


    void slotPreviousTab();
    void slotCloseTab();
    void slotCloseTab(int index);
    void slotRecreatePanels();
    void slotCloseInactiveTabs();
    void slotCloseDuplicatedTabs();

protected slots:
    void slotChangePanel(ListPanel *p);
    void slotRefreshActions();

private:
    void deletePanel(ListPanel *p);
    void updateTabbarPos();
    void tabsCountChanged();

    QGridLayout *_layout;
    QHBoxLayout *_barLayout;
    bool _left;
    PanelTabBar *_tabbar;
    QStackedWidget *_stack;
    QToolButton *_newTab, *_closeTab;
    ListPanel **_selfPtr, **_otherPtr;
};


#endif // _PANEL_MANAGER_H
