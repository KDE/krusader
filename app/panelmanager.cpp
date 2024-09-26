/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "panelmanager.h"

#include "Panel/PanelView/krview.h"
#include "Panel/PanelView/krviewfactory.h"
#include "Panel/listpanel.h"
#include "Panel/panelfunc.h"
#include "defaults.h"
#include "icon.h"
#include "kractions.h"
#include "krmainwindow.h"
#include "krusaderview.h"
#include "tabactions.h"

#include <assert.h>

// QtGui
#include <QImage>
// QtWidgets
#include <QGridLayout>
#include <QMenu>
#include <QStackedWidget>
#include <QToolButton>

#include <KConfig>
#include <KLocalizedString>

PanelManager::PanelManager(QWidget *parent, KrMainWindow *mainWindow, bool left)
    : QWidget(parent)
    , _otherManager(nullptr)
    , _actions(mainWindow->tabActions())
    , _layout(nullptr)
    , _left(left)
    , _currentPanel(nullptr)
{
    _layout = new QGridLayout(this);
    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->setSpacing(0);
    _stack = new QStackedWidget(this);

    // new tab button
    _newTab = new QToolButton(this);
    _newTab->setAutoRaise(true);
    _newTab->setText(i18n("Open a new tab"));
    _newTab->setToolTip(i18n("Open a new tab"));
    _newTab->setIcon(Icon("tab-new"));
    _newTab->adjustSize();
    connect(_newTab, &QToolButton::clicked, this, &PanelManager::slotNewTabButton);

    // tab-bar
    _tabbar = new PanelTabBar(this, _actions);
    connect(_tabbar, &PanelTabBar::currentChanged, this, &PanelManager::slotCurrentTabChanged);
    connect(_tabbar, &PanelTabBar::tabCloseRequested, this, QOverload<int>::of(&PanelManager::slotCloseTab));
    connect(_tabbar, &PanelTabBar::closeCurrentTab, this, QOverload<>::of(&PanelManager::slotCloseTab));
    connect(_tabbar, &PanelTabBar::duplicateCurrentTab, this, &PanelManager::slotDuplicateTabLMB, Qt::QueuedConnection);
    connect(_tabbar, &PanelTabBar::newTab, this, [=](const QUrl &url) {
        slotNewTab(url);
    });
    connect(_tabbar, &PanelTabBar::draggingTab, this, &PanelManager::slotDraggingTab);
    connect(_tabbar, &PanelTabBar::draggingTabFinished, this, &PanelManager::slotDraggingTabFinished);

    auto *tabbarLayout = new QHBoxLayout;
    tabbarLayout->setSpacing(0);
    tabbarLayout->setContentsMargins(0, 0, 0, 0);

    tabbarLayout->addWidget(_tabbar);
    tabbarLayout->addWidget(_newTab);

    _layout->addWidget(_stack, 0, 0);
    _layout->addLayout(tabbarLayout, 1, 0);

    updateTabbarPos();

    setLayout(_layout);

    addPanel(true);

    tabsCountChanged();
}

void PanelManager::tabsCountChanged()
{
    const KConfigGroup cfg(krConfig, "Look&Feel");
    const bool showTabbar = _tabbar->count() > 1 || cfg.readEntry("Show Tab Bar On Single Tab", true);
    const bool showNewButton = showTabbar && cfg.readEntry("Show New Tab Button", true);
    const bool showCloseButtons = showTabbar && cfg.readEntry("Show Close Tab Buttons", true);

    _tabbar->setVisible(showTabbar);
    _newTab->setVisible(showNewButton);

    // disable close button if only 1 tab is left
    _tabbar->setTabsClosable(showCloseButtons && _tabbar->count() > 1);

    _actions->refreshActions();
}

void PanelManager::activate()
{
    assert(sender() == (currentPanel()->gui));
    emit setActiveManager(this);
    _actions->refreshActions();
}

void PanelManager::slotCurrentTabChanged(int index)
{
    ListPanel *panel = _tabbar->getPanel(index);

    if (!panel || panel == _currentPanel)
        return;

    ListPanel *previousPanel = _currentPanel;
    _currentPanel = panel;

    _stack->setCurrentWidget(_currentPanel);

    if (previousPanel) {
        previousPanel->slotFocusOnMe(false); // FIXME - necessary ?
    }
    _currentPanel->slotFocusOnMe(this == ACTIVE_MNG);

    emit pathChanged(panel);

    if (otherManager()) {
        otherManager()->currentPanel()->otherPanelChanged();
    }

    // go back to pinned url if tab is pinned and switched back to active
    if (panel->isPinned()) {
        panel->func->openUrl(panel->pinnedUrl());
    }
}

ListPanel *PanelManager::createPanel(const KConfigGroup &cfg)
{
    ListPanel *p = new ListPanel(_stack, this, cfg);
    connectPanel(p);
    return p;
}

void PanelManager::connectPanel(ListPanel *p)
{
    connect(p, &ListPanel::activate, this, &PanelManager::activate);
    connect(p, &ListPanel::pathChanged, this, [=]() {
        emit pathChanged(p);
    });
    connect(p, &ListPanel::pathChanged, this, [=]() {
        _tabbar->updateTab(p);
    });
}

void PanelManager::disconnectPanel(ListPanel *p)
{
    disconnect(p, &ListPanel::activate, this, nullptr);
    disconnect(p, &ListPanel::pathChanged, this, nullptr);
}

ListPanel *PanelManager::addPanel(bool setCurrent, const KConfigGroup &cfg, int insertIndex)
{
    // create the panel and add it into the widgetstack
    ListPanel *p = createPanel(cfg);
    _stack->addWidget(p);

    // now, create the corresponding tab
    int index = _tabbar->addPanel(p, setCurrent, insertIndex);
    tabsCountChanged();

    if (setCurrent)
        slotCurrentTabChanged(index);

    return p;
}

ListPanel *PanelManager::duplicatePanel(const KConfigGroup &cfg, KrPanel *nextTo, int insertIndex)
{
    // Search for the position where the passed panel is
    if (insertIndex == -1) {
        int quantOfPanels = _tabbar->count();
        for (int i = 0; i < quantOfPanels; i++) {
            if (_tabbar->getPanel(i) == nextTo) {
                insertIndex = i + 1;
                break;
            }
        }
    }

    return addPanel(true, cfg, insertIndex);
}

void PanelManager::saveSettings(KConfigGroup config, bool saveHistory)
{
    config.writeEntry("ActiveTab", activeTab());

    KConfigGroup grpTabs(&config, "Tabs");
    const auto tabGroups = grpTabs.groupList();
    for (const QString &grpTab : tabGroups)
        grpTabs.deleteGroup(grpTab);

    for (int i = 0; i < _tabbar->count(); i++) {
        ListPanel *panel = _tabbar->getPanel(i);
        KConfigGroup grpTab(&grpTabs, "Tab" + QString::number(i));
        panel->saveSettings(grpTab, saveHistory);
    }
}

void PanelManager::loadSettings(KConfigGroup config)
{
    KConfigGroup grpTabs(&config, "Tabs");
    int numTabsOld = _tabbar->count();
    qsizetype numTabsNew = grpTabs.groupList().count();

    for (int i = 0; i < numTabsNew; i++) {
        KConfigGroup grpTab(&grpTabs, "Tab" + QString::number(i));
        // TODO workaround for bug 371453. Remove this when bug is fixed
        if (grpTab.keyList().isEmpty())
            continue;

        ListPanel *panel = i < numTabsOld ? _tabbar->getPanel(i) : addPanel(false, grpTab, i);
        panel->restoreSettings(grpTab);
        _tabbar->updateTab(panel);
    }

    for (int i = numTabsOld - 1; i >= numTabsNew && i > 0; i--)
        slotCloseTab(i);

    setActiveTab(config.readEntry("ActiveTab", 0));

    // this is needed so that all tab labels get updated
    layoutTabs();
}

void PanelManager::layoutTabs()
{
    // delayed url refreshes may be pending -
    // delay the layout too so it happens after them
    QTimer::singleShot(0, _tabbar, &PanelTabBar::layoutTabs);
}

KrPanel *PanelManager::currentPanel() const
{
    return _currentPanel;
}

void PanelManager::moveTabToOtherSide()
{
    if (tabCount() < 2)
        return;

    ListPanel *p;
    _tabbar->removeCurrentPanel(p);
    _stack->removeWidget(p);
    disconnectPanel(p);

    p->reparent(_otherManager->_stack, _otherManager);
    _otherManager->connectPanel(p);
    _otherManager->_stack->addWidget(p);
    _otherManager->_tabbar->addPanel(p, true);

    _otherManager->tabsCountChanged();
    tabsCountChanged();

    p->slotFocusOnMe();
}

void PanelManager::moveTabToLeft()
{
    // don't move the leftmost tab - also skip a single tab, always leftmost
    if (_tabbar->currentIndex() == 0)
        return;

    _tabbar->moveTab(_tabbar->currentIndex(), _tabbar->currentIndex() - 1);
}

void PanelManager::moveTabToRight()
{
    // don't move the rightmost tab - also skip a single tab, always rightmost
    if (_tabbar->currentIndex() == tabCount() - 1)
        return;

    _tabbar->moveTab(_tabbar->currentIndex(), _tabbar->currentIndex() + 1);
}

void PanelManager::slotNewTab(const QUrl &url, bool setCurrent, int insertIndex)
{
    ListPanel *p = addPanel(setCurrent, KConfigGroup(), insertIndex);
    p->start(url);
}

void PanelManager::slotNewTabButton()
{
    KConfigGroup group(krConfig, "Look&Feel");
    int insertIndex = group.readEntry("Insert Tabs After Current", false) ? _tabbar->currentIndex() + 1 : _tabbar->count();

    if (group.readEntry("New Tab Button Duplicates", false)) {
        slotDuplicateTab(currentPanel()->virtualPath(), currentPanel(), insertIndex);
        _currentPanel->slotFocusOnMe();
    } else {
        slotNewTab(insertIndex);
    }
}

void PanelManager::slotNewTab(int insertIndex)
{
    slotNewTab(QUrl::fromLocalFile(QDir::home().absolutePath()), true, insertIndex);
    _currentPanel->slotFocusOnMe();
}

void PanelManager::slotDuplicateTab(const QUrl &url, KrPanel *nextTo, int insertIndex)
{
    ListPanel *p = duplicatePanel(KConfigGroup(), nextTo, insertIndex);
    if (nextTo && nextTo->gui) {
        // We duplicate tab settings by writing original settings to a temporary
        // group and making the new tab read settings from it. Duplicating
        // settings directly would add too much complexity.
        QString grpName = "PanelManager_" + QString::number(qApp->applicationPid());
        krConfig->deleteGroup(grpName); // make sure the group is empty
        KConfigGroup cfg(krConfig, grpName);

        nextTo->gui->saveSettings(cfg, true);
        // reset undesired duplicated settings
        cfg.writeEntry("Properties", 0);
        p->restoreSettings(cfg);
        krConfig->deleteGroup(grpName);
    }
    p->start(url);
}

void PanelManager::slotDuplicateTabLMB()
{
    slotDuplicateTab(currentPanel()->virtualPath(), currentPanel());
}

void PanelManager::slotCloseTab()
{
    slotCloseTab(_tabbar->currentIndex());
}

void PanelManager::slotCloseTab(int index)
{
    if (_tabbar->count() <= 1) /* if this is the last tab don't close it */
        return;

    // Back up some data that will be useful if the user wants to
    // undo the closing of the tab
    QByteArray backupData;
    QDataStream tabStream(&backupData, QIODevice::WriteOnly); // In order to serialize data
    tabStream << _left;
    tabStream << index;
    ListPanel *panel = static_cast<ListPanel *>(currentPanel());
    const QUrl urlTab = panel->virtualPath();
    tabStream << urlTab;
    tabStream << panel->getProperties();
    tabStream << panel->pinnedUrl();
    tabStream << panel->view->selectedUrls();

    QAction *actReopenTab = KrActions::actClosedTabsMenu->updateAfterClosingATab(urlTab, backupData, _actions);

    // Save settings of the tab. Note: The code is
    // based on the one of slotDuplicateTab()
    QString grpName = QString("closedTab_%1").arg(reinterpret_cast<qulonglong>(actReopenTab));
    krConfig->deleteGroup(grpName); // make sure the group is empty
    KConfigGroup cfg(krConfig, grpName);
    panel->gui->saveSettings(cfg, true);
    // reset undesired duplicated settings
    cfg.writeEntry("Properties", 0);

    _tabbar->removePanel(index, panel); // this automatically changes the current panel

    _stack->removeWidget(panel);
    deletePanel(panel);
    tabsCountChanged();
}

void PanelManager::slotUndoCloseTab()
{
    const int fixedMenuEntries = KrActions::actClosedTabsMenu->quantFixedMenuEntries;
    Q_ASSERT(KrActions::actClosedTabsMenu->menu()->actions().size() > fixedMenuEntries);
    // Performs the same action as when clicking on that menu item
    KrActions::actClosedTabsMenu->slotTriggered(KrActions::actClosedTabsMenu->menu()->actions().at(fixedMenuEntries));
}

void PanelManager::undoCloseTab(const QAction *action)
{
    QDataStream tabStream(action->data().toByteArray());
    // Deserialize data
    bool closedInTheLeftPan;
    tabStream >> closedInTheLeftPan;
    int insertIndex;
    tabStream >> insertIndex;
    QUrl urlClosedTab;
    tabStream >> urlClosedTab;
    int tabProperties;
    tabStream >> tabProperties;
    QUrl pinnedUrl;
    tabStream >> pinnedUrl;
    QList<QUrl> selectedUrls;
    tabStream >> selectedUrls;

    // This variable points to the PanelManager where the closed tab is going to be restored
    PanelManager *whereToUndo = closedInTheLeftPan ? LEFT_MNG : RIGHT_MNG;

    MAIN_VIEW->slotSetActiveManager(whereToUndo);
    // Open a new tab where to apply the planned changes
    whereToUndo->slotNewTab(urlClosedTab, true, insertIndex);

    // Restore settings of the tab. Note: The code is
    // based on the one of slotDuplicateTab()
    QString grpName = QString("closedTab_%1").arg(reinterpret_cast<qulonglong>(action));
    KConfigGroup cfg(krConfig, grpName);
    ListPanel *panel = static_cast<ListPanel *>(whereToUndo->currentPanel());
    panel->restoreSettings(cfg);
    krConfig->deleteGroup(grpName);
    panel->setProperties(tabProperties);
    panel->setPinnedUrl(pinnedUrl);
    // Note: In `void PanelManager::layoutTabs()` there was a similar `QTimer::singleShot(`
    // and a comment about it: "delayed url refreshes may be pending - delay the layout too
    // so it happens after them"
    QTimer::singleShot(1, this, [=] {
        panel->view->setSelectionUrls(selectedUrls);
    });
}

void PanelManager::delAllClosedTabs()
{
    const int quantFixedMenuEntries = KrActions::actClosedTabsMenu->quantFixedMenuEntries;
    RecentlyClosedTabsMenu *closedTabsMenu = KrActions::actClosedTabsMenu;
    if (closedTabsMenu) {
        const qsizetype quantActions = closedTabsMenu->menu()->actions().size();
        // Remove the actions (and related information) that follow the
        // fixed menu entries
        for (qsizetype x = quantActions - 1; x >= quantFixedMenuEntries; x--) {
            QAction *action = closedTabsMenu->menu()->actions().at(x);
            delClosedTab(action);
        }
    }
}

void PanelManager::delClosedTab(QAction *action)
{
    // Delete the settings of the closed tab.
    // Note: The code is based on the one of slotCloseTab()
    QString grpName = QString("closedTab_%1").arg(reinterpret_cast<qulonglong>(action));
    krConfig->deleteGroup(grpName);

    // Remove the menu entry and the rest of its information
    KrActions::actClosedTabsMenu->removeAction(action);
}

void PanelManager::updateTabbarPos()
{
    KConfigGroup group(krConfig, "Look&Feel");
    if (group.readEntry("Tab Bar Position", _TabBarPosition) == "top") {
        _layout->addWidget(_stack, 2, 0);
        _tabbar->setShape(QTabBar::RoundedNorth);
    } else {
        _layout->addWidget(_stack, 0, 0);
        _tabbar->setShape(QTabBar::RoundedSouth);
    }
}

int PanelManager::activeTab()
{
    return _tabbar->currentIndex();
}

void PanelManager::setActiveTab(int index)
{
    _tabbar->setCurrentIndex(index);
}

void PanelManager::slotRecreatePanels()
{
    updateTabbarPos();

    for (int i = 0; i != _tabbar->count(); i++) {
        QString grpName = "PanelManager_" + QString::number(qApp->applicationPid());
        krConfig->deleteGroup(grpName); // make sure the group is empty
        KConfigGroup cfg(krConfig, grpName);

        ListPanel *oldPanel = _tabbar->getPanel(i);
        oldPanel->view->setFileIconSize(oldPanel->view->defaultFileIconSize());
        oldPanel->saveSettings(cfg, true);
        disconnect(oldPanel);

        ListPanel *newPanel = createPanel(cfg);
        _stack->insertWidget(i, newPanel);
        _tabbar->changePanel(i, newPanel);

        if (_currentPanel == oldPanel) {
            _currentPanel = newPanel;
            _stack->setCurrentWidget(_currentPanel);
        }

        _stack->removeWidget(oldPanel);
        deletePanel(oldPanel);

        newPanel->restoreSettings(cfg);

        _tabbar->updateTab(newPanel);

        krConfig->deleteGroup(grpName);
    }
    tabsCountChanged();
    _currentPanel->slotFocusOnMe(this == ACTIVE_MNG);
    emit pathChanged(_currentPanel);
}

void PanelManager::slotNextTab()
{
    int currTab = _tabbar->currentIndex();
    int nextInd = (currTab == _tabbar->count() - 1 ? 0 : currTab + 1);
    _tabbar->setCurrentIndex(nextInd);
}

void PanelManager::slotPreviousTab()
{
    int currTab = _tabbar->currentIndex();
    int nextInd = (currTab == 0 ? _tabbar->count() - 1 : currTab - 1);
    _tabbar->setCurrentIndex(nextInd);
}

void PanelManager::reloadConfig()
{
    for (int i = 0; i < _tabbar->count(); i++) {
        ListPanel *panel = _tabbar->getPanel(i);
        if (panel) {
            panel->func->refresh();
        }
    }
}

void PanelManager::deletePanel(ListPanel *p)
{
    disconnect(p);
    delete p;
}

void PanelManager::slotCloseInactiveTabs()
{
    int i = 0;
    while (i < _tabbar->count()) {
        if (i == activeTab())
            i++;
        else
            slotCloseTab(i);
    }
}

void PanelManager::slotCloseDuplicatedTabs()
{
    int i = 0;
    while (i < _tabbar->count() - 1) {
        ListPanel *panel1 = _tabbar->getPanel(i);
        if (panel1 != nullptr) {
            for (int j = i + 1; j < _tabbar->count(); j++) {
                ListPanel *panel2 = _tabbar->getPanel(j);
                if (panel2 != nullptr && panel1->virtualPath().matches(panel2->virtualPath(), QUrl::StripTrailingSlash)) {
                    if (j == activeTab()) {
                        slotCloseTab(i);
                        i--;
                        break;
                    } else {
                        slotCloseTab(j);
                        j--;
                    }
                }
            }
        }
        i++;
    }
}

int PanelManager::findTab(QUrl url)
{
    url.setPath(QDir::cleanPath(url.path()));
    for (int i = 0; i < _tabbar->count(); i++) {
        if (_tabbar->getPanel(i)) {
            QUrl panelUrl = _tabbar->getPanel(i)->virtualPath();
            panelUrl.setPath(QDir::cleanPath(panelUrl.path()));
            if (panelUrl.matches(url, QUrl::StripTrailingSlash))
                return i;
        }
    }
    return -1;
}

void PanelManager::slotLockTab()
{
    ListPanel *panel = _currentPanel;
    panel->gui->setTabState(panel->gui->isLocked() ? ListPanel::TabState::DEFAULT : ListPanel::TabState::LOCKED);
    _actions->refreshActions();
    _tabbar->updateTab(panel);
}

void PanelManager::slotPinTab()
{
    ListPanel *panel = _currentPanel;
    panel->gui->setTabState(panel->gui->isPinned() ? ListPanel::TabState::DEFAULT : ListPanel::TabState::PINNED);
    if (panel->gui->isPinned()) {
        QUrl virtualPath = panel->virtualPath();
        panel->setPinnedUrl(virtualPath);
    }
    _actions->refreshActions();
    _tabbar->updateTab(panel);
}

void PanelManager::newTabs(const QStringList &urls)
{
    for (int i = 0; i < urls.count(); i++)
        slotNewTab(QUrl::fromUserInput(urls[i], QString(), QUrl::AssumeLocalFile));
}
