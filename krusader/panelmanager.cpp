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

#include "panelmanager.h"

#include "defaults.h"
#include "tabactions.h"
#include "krusaderview.h"
#include "filemanagerwindow.h"
#include "Panel/listpanel.h"
#include "Panel/panelfunc.h"
#include "Panel/krviewfactory.h"

#include <qstackedwidget.h>
#include <QtGui/QToolButton>
#include <QGridLayout>
#include <QtGui/QImage>

#include <klocale.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kiconloader.h>

#include <assert.h>

#define HIDE_ON_SINGLE_TAB  false

PanelManager::PanelManager(QWidget *parent, FileManagerWindow* mainWindow, bool left) :
        QWidget(parent),
        _otherManager(0),
        _actions(mainWindow->tabActions()),
        _layout(0),
        _left(left),
        _self(0)
{
    _layout = new QGridLayout(this);
    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->setSpacing(0);
    _stack = new QStackedWidget(this);

    // new tab button
    _newTab = new QToolButton(this);
    _newTab->setAutoRaise(true);
    _newTab->setText(i18n("Open a new tab in home"));
    _newTab->setToolTip(i18n("Open a new tab in home"));
    _newTab->setIcon(SmallIcon("tab-new"));
    _newTab->adjustSize();
    connect(_newTab, SIGNAL(clicked()), this, SLOT(slotNewTab()));

    // close tab button
    _closeTab = new QToolButton(this);
    _closeTab->setAutoRaise(true);
    _closeTab->setText(i18n("Close current tab"));
    _closeTab->setToolTip(i18n("Close current tab"));
    _closeTab->setIcon(SmallIcon("tab-close"));
    _closeTab->adjustSize();
    connect(_closeTab, SIGNAL(clicked()), this, SLOT(slotCloseTab()));
    _closeTab->setEnabled(false);   // disabled when there's only 1 tab

    // tab-bar
    _tabbar = new PanelTabBar(this, _actions);
    connect(_tabbar, SIGNAL(changePanel(ListPanel*)), this, SLOT(slotChangePanel(ListPanel *)));
    connect(_tabbar, SIGNAL(closeCurrentTab()), this, SLOT(slotCloseTab()));
    connect(_tabbar, SIGNAL(newTab(const KUrl&)), this, SLOT(slotNewTab(const KUrl&)));
    connect(_tabbar, SIGNAL(draggingTab(QMouseEvent*)), this, SLOT(slotDraggingTab(QMouseEvent*)));
    connect(_tabbar, SIGNAL(draggingTabFinished(QMouseEvent*)), this, SLOT(slotDraggingTabFinished(QMouseEvent*)));

    QHBoxLayout *tabbarLayout = new QHBoxLayout;
    tabbarLayout->setSpacing(0);
    tabbarLayout->setContentsMargins(0, 0, 0, 0);

    tabbarLayout->addWidget(_newTab);
    tabbarLayout->addWidget(_tabbar);
    tabbarLayout->addWidget(_closeTab);

    _layout->addWidget(_stack, 0, 0);
    _layout->addLayout(tabbarLayout, 1, 0);

    updateTabbarPos();

    setLayout(_layout);

    addPanel(true);

    tabsCountChanged();
}

void PanelManager::tabsCountChanged()
{
    bool showTabbar = true;
    if (_tabbar->count() <= 1 && HIDE_ON_SINGLE_TAB)
        showTabbar = false;

    KConfigGroup cfg(krConfig, "Look&Feel");
    bool showButtons = showTabbar && cfg.readEntry("Show Tab Buttons", true);

    _newTab->setVisible(showButtons);
    _tabbar->setVisible(showTabbar);
    _closeTab->setVisible(showButtons);

    // disable close button if only 1 tab is left
    _closeTab->setEnabled(_tabbar->count() > 1);

    _actions->refreshActions();
}

void PanelManager::activate()
{
    assert(sender() == currentPanel());
    emit setActiveManager(this);
    _actions->refreshActions();
}

void PanelManager::slotChangePanel(ListPanel *p, bool makeActive)
{
    if (p == 0)
        return;

    ListPanel *prev = _self;
    _self = p;

//     _stack->setUpdatesEnabled(false);

    _stack->setCurrentWidget(_self);

    _self->slotFocusOnMe((this == ACTIVE_MNG) || makeActive);

    emit pathChanged(p);

    if(otherManager() && _self != prev)
        otherManager()->currentPanel()->otherPanelChanged();

//     _stack->setUpdatesEnabled(true);
}

ListPanel* PanelManager::createPanel(KConfigGroup cfg)
{
    ListPanel * p = new ListPanel(_stack, this, cfg);
    connect(p, SIGNAL(activate()), this, SLOT(activate()));
    connect(p, SIGNAL(pathChanged(ListPanel*)), this, SIGNAL(pathChanged(ListPanel*)));
    connect(p, SIGNAL(pathChanged(ListPanel*)), _tabbar, SLOT(updateTab(ListPanel*)));
    return p;
}

ListPanel* PanelManager::addPanel(bool setCurrent, KConfigGroup cfg, KrPanel *nextTo)
{
    // create the panel and add it into the widgetstack
    ListPanel * p = createPanel(cfg);
    _stack->addWidget(p);

    // now, create the corrosponding tab
    _tabbar->addPanel(p, setCurrent, nextTo);
    tabsCountChanged();

    if (setCurrent)
        slotChangePanel(p, false);

    return p;
}

void PanelManager::saveSettings(KConfigGroup config, bool localOnly, bool saveHistory)
{
    config.writeEntry("ActiveTab", activeTab());

    KConfigGroup grpTabs(&config, "Tabs");
    foreach(QString grpTab, grpTabs.groupList())
        grpTabs.deleteGroup(grpTab);

    for(int i = 0; i < _tabbar->count(); i++) {
        ListPanel *panel = _tabbar->getPanel(i);
        KConfigGroup grpTab(&grpTabs, "Tab" + QString::number(i));
        panel->saveSettings(grpTab, localOnly, saveHistory);
    }
}

void PanelManager::loadSettings(KConfigGroup config)
{
    KConfigGroup grpTabs(&config, "Tabs");
    int numTabsOld = _tabbar->count();
    int numTabsNew = grpTabs.groupList().count();

    for(int i = 0;  i < numTabsNew; i++) {
        KConfigGroup grpTab(&grpTabs, "Tab" + QString::number(i));
        ListPanel *panel;
        if(i < numTabsOld)
            panel = _tabbar->getPanel(i);
        else
            panel = addPanel(false, grpTab);
        panel->restoreSettings(grpTab);
    }

    for(int i = numTabsOld - 1; i >= numTabsNew && i > 0; i--)
        slotCloseTab(i);

    setActiveTab(config.readEntry("ActiveTab", 0));

    // this is needed so that all tab labels get updated
    layoutTabs();
}

void PanelManager::layoutTabs()
{
    // delayed url refreshes may be pending -
    // delay the layout too so it happens after them
    QTimer::singleShot(0, _tabbar, SLOT(layoutTabs()));
}

void PanelManager::moveTabToOtherSide()
{
    if(tabCount() < 2)
        return;

    ListPanel *p;
    _self = _tabbar->removeCurrentPanel(p);
    _stack->setCurrentWidget(_self);
    _stack->removeWidget(p);

    PanelManager *otherMng = static_cast<PanelManager*>(otherManager());
    p->reparent(otherMng->_stack, otherMng);
    int idx = otherMng->_tabbar->addPanel(p, true);
    otherMng->_stack->addWidget(p);
    otherMng->_stack->setCurrentWidget(p);
    otherMng->_self = p;

    otherMng->tabsCountChanged();
    tabsCountChanged();

    p->slotFocusOnMe();
}

void PanelManager::slotNewTab(const KUrl& url, bool setCurrent, KrPanel *nextTo)
{
    ListPanel *p = addPanel(setCurrent, KConfigGroup(), nextTo);
    p->start(url);
}

void PanelManager::slotNewTab()
{
    slotNewTab(QDir::home().absolutePath());
}

void PanelManager::slotCloseTab()
{
    slotCloseTab(_tabbar->currentIndex());
}

void PanelManager::slotCloseTab(int index)
{
    if (_tabbar->count() <= 1)    /* if this is the last tab don't close it */
        return ;

    ListPanel *oldp;

    if (index == _tabbar->currentIndex())
        slotChangePanel(_tabbar->removeCurrentPanel(oldp), false);
    else
        _tabbar->removePanel(index, oldp);

    _stack->removeWidget(oldp);
    deletePanel(oldp);
    tabsCountChanged();
}

void PanelManager::updateTabbarPos()
{
    KConfigGroup group(krConfig, "Look&Feel");
    if(group.readEntry("Tab Bar Position", "bottom") == "top") {
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
    slotChangePanel(_tabbar->getPanel(index));
}

void PanelManager::slotRecreatePanels()
{
    int actTab = activeTab();
    QString grpName = "PanelManager_" + QString::number(qApp->applicationPid());
    KConfigGroup cfg(krConfig, grpName);

    for (int i = 0; i != _tabbar->count(); i++) {
        ListPanel *oldPanel = _tabbar->getPanel(i);
        oldPanel->saveSettings(cfg, false);
        disconnect(oldPanel);

        ListPanel *newPanel = createPanel(cfg);
        newPanel->restoreSettings(cfg);

        _tabbar->changePanel(i, newPanel);
        _stack->insertWidget(i, newPanel);
        _stack->removeWidget(oldPanel);

        if (_self == oldPanel)
            _self = newPanel;

        deletePanel(oldPanel);

        _tabbar->updateTab(newPanel);
    }

    krConfig->deleteGroup(grpName);

    updateTabbarPos();
    setActiveTab(actTab);
    tabsCountChanged();
}

void PanelManager::slotNextTab()
{
    int currTab = _tabbar->currentIndex();
    int nextInd = (currTab == _tabbar->count() - 1 ? 0 : currTab + 1);
    ListPanel *nextp = _tabbar->getPanel(nextInd);
    _tabbar->setCurrentIndex(nextInd);
    slotChangePanel(nextp);
}


void PanelManager::slotPreviousTab()
{
    int currTab = _tabbar->currentIndex();
    int nextInd = (currTab == 0 ? _tabbar->count() - 1 : currTab - 1);
    ListPanel *nextp = _tabbar->getPanel(nextInd);
    _tabbar->setCurrentIndex(nextInd);
    slotChangePanel(nextp);
}

void PanelManager::refreshAllTabs(bool invalidate)
{
    int i = 0;
    while (i < _tabbar->count()) {
        ListPanel *panel = _tabbar->getPanel(i);
        if (panel && panel->func) {
            vfs * vfs = panel->func->files();
            if (vfs) {
                if (invalidate)
                    vfs->vfs_invalidate();
                vfs->vfs_refresh();
            }
        }
        ++i;
    }
}

void PanelManager::deletePanel(ListPanel * p)
{
    disconnect(p);
    if (p && p->func && p->func->files() && !p->func->files()->vfs_canDelete()) {
        connect(p->func->files(), SIGNAL(deleteAllowed()), p, SLOT(deleteLater()));
        p->func->files()->vfs_requestDelete();
        return;
    }
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
        ListPanel * panel1 = _tabbar->getPanel(i);
        if (panel1 != 0) {
            for (int j = i + 1; j < _tabbar->count(); j++) {
                ListPanel * panel2 = _tabbar->getPanel(j);
                if (panel2 != 0 && panel1->virtualPath().equals(panel2->virtualPath(), KUrl::CompareWithoutTrailingSlash)) {
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

int PanelManager::findTab(KUrl url)
{
    url.cleanPath();
    for(int i = 0; i < _tabbar->count(); i++) {
        if(_tabbar->getPanel(i)) {
            KUrl panelUrl = _tabbar->getPanel(i)->virtualPath();
            panelUrl.cleanPath();
            if(panelUrl.equals(url, KUrl::CompareWithoutTrailingSlash))
                return i;
        }
    }
    return -1;
}

void PanelManager::slotLockTab()
{
    currentPanel()->gui->setLocked(!currentPanel()->gui->isLocked());
    _actions->refreshActions();
}

void PanelManager::newTabs(const QStringList& urls) {
    for(int i = 0; i < urls.count(); i++)
        slotNewTab(KUrl(urls[i]));
}

KrPanel *PanelManager::currentPanel()
{
    return _self;
}

#include "panelmanager.moc"
