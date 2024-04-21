/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PANELMANAGER_H
#define PANELMANAGER_H

#include "abstractpanelmanager.h"

// QtWidgets
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLayout>
#include <QWidget>

#include <KConfigGroup>

#include "paneltabbar.h"

class QStackedWidget;
class QToolButton;
class ListPanel;
class KrMainWindow;
class TabActions;

/**
 * Implements tabbed-browsing by managing a list of tabs and corresponding panels.
 */
class PanelManager : public QWidget, public AbstractPanelManager
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.krusader.PanelManager")

public:
    /**
     * PanelManager is created where once panels were created. It accepts three references to pointers
     * (self, other, active), which enables it to manage pointers held by the panels transparently.
     * It also receives a bool (left) which is true if the manager is the left one, or false otherwise.
     */
    PanelManager(QWidget *parent, KrMainWindow *mainWindow, bool left);

    void saveSettings(KConfigGroup config, bool saveHistory);
    void loadSettings(KConfigGroup config);
    int findTab(QUrl url);
    int tabCount()
    {
        return _tabbar->count();
    }
    int activeTab();
    void setActiveTab(int index);
    void moveTabToOtherSide();
    void moveTabToLeft();
    void moveTabToRight();
    /** Refresh all tabs after config changes. */
    void reloadConfig();
    void layoutTabs();
    void setLeft(bool left)
    {
        _left = left;
    }
    void setOtherManager(PanelManager *other)
    {
        _otherManager = other;
    }

    // AbstractPanelManager implementation
    bool isLeft() const override
    {
        return _left;
    }
    AbstractPanelManager *otherManager() const override
    {
        return _otherManager;
    }
    KrPanel *currentPanel() const override;
    void newTab(const QUrl &url, int insertIndex) override
    {
        slotNewTab(url, true, insertIndex);
    }
    void duplicateTab(const QUrl &url, KrPanel *nextTo) override
    {
        slotDuplicateTab(url, nextTo);
    }
    /**
     * Undo the closing of a tab. The `action` argument is utilized to
     * retrieve information about the previously closed tab.
     */
    void undoCloseTab(const QAction *action);
    /**
     * Delete what is stored about all the closed tabs.
     */
    void delAllClosedTabs();
    /**
     * Delete what is stored about a closed tab.
     */
    void delClosedTab(QAction *action);

signals:
    void draggingTab(PanelManager *from, QMouseEvent *);
    void draggingTabFinished(PanelManager *from, QMouseEvent *);
    void setActiveManager(PanelManager *manager);
    void pathChanged(ListPanel *panel);

public slots:
    /**
     * Called externally to start a new tab. Example of usage would be the "open in a new tab"
     * action, from the context-menu.
     */

    Q_SCRIPTABLE void newTab(const QString &url)
    {
        slotNewTab(QUrl::fromUserInput(url, QString(), QUrl::AssumeLocalFile));
    }
    Q_SCRIPTABLE void newTabs(const QStringList &urls);

    void slotNewTab(const QUrl &url, bool setCurrent = true, int insertIndex = -1);
    void slotNewTabButton();
    void slotNewTab(int insertIndex = -1);
    void slotDuplicateTab(const QUrl &url, KrPanel *nextTo, int insertIndex = -1);
    void slotDuplicateTabLMB();
    void slotLockTab();
    void slotPinTab();
    void slotNextTab();
    void slotPreviousTab();
    void slotCloseTab();
    void slotCloseTab(int index);
    void slotUndoCloseTab(); //! Undo the last tab closing
    void slotRecreatePanels();
    void slotCloseInactiveTabs();
    void slotCloseDuplicatedTabs();

protected slots:
    void slotCurrentTabChanged(int index);
    void activate();
    void slotDraggingTab(QMouseEvent *e)
    {
        emit draggingTab(this, e);
    }
    void slotDraggingTabFinished(QMouseEvent *e)
    {
        emit draggingTabFinished(this, e);
    }

private:
    void deletePanel(ListPanel *p);
    void updateTabbarPos();
    void tabsCountChanged();
    ListPanel *addPanel(bool setCurrent = true, const KConfigGroup &cfg = KConfigGroup(), int insertIndex = -1);
    ListPanel *duplicatePanel(const KConfigGroup &cfg, KrPanel *nextTo, int insertIndex = -1);
    ListPanel *createPanel(const KConfigGroup &cfg);
    void connectPanel(ListPanel *p);
    void disconnectPanel(ListPanel *p);

    PanelManager *_otherManager;
    TabActions *_actions;
    QGridLayout *_layout;
    QHBoxLayout *_barLayout;
    bool _left;
    PanelTabBar *_tabbar;
    QStackedWidget *_stack;
    QToolButton *_newTab;
    ListPanel *_currentPanel;
};

#endif // _PANEL_MANAGER_H
