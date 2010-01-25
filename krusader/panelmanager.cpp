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

#include <qstackedwidget.h>
#include <QtGui/QToolButton>
#include <QGridLayout>
#include <QtGui/QImage>

#include <klocale.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kiconloader.h>

#include "Panel/listpanel.h"
#include "Panel/panelfunc.h"
#include "krusaderview.h"
#include "defaults.h"
#include "Panel/krviewfactory.h"

#define HIDE_ON_SINGLE_TAB  false

#define SHOW    { _newTab->show(); _tabbar->show(); _closeTab->show(); }
#define HIDE    { _newTab->hide(); _tabbar->hide(); _closeTab->hide(); }

#define _self   (*_selfPtr)
#define _other  (*_otherPtr)

PanelManager::PanelManager(QWidget *parent, bool left) :
        QWidget(parent), _layout(0), _left(left),
        _selfPtr(_left ? &MAIN_VIEW->left : &MAIN_VIEW->right),
        _otherPtr(_left ? &MAIN_VIEW->right : &MAIN_VIEW->left)
{
    _layout = new QGridLayout(this);
    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->setSpacing(0);
    _stack = new QStackedWidget(this);

    // new tab button
    _newTab = new QToolButton(this);
    _newTab->setFixedSize(22, 22);
    _newTab->setText(i18n("Open a new tab in home"));
    _newTab->setToolTip(i18n("Open a new tab in home"));
    _newTab->setIcon(SmallIcon("tab-new"));
    _newTab->adjustSize();
    connect(_newTab, SIGNAL(clicked()), this, SLOT(slotNewTab()));

    // close tab button
    _closeTab = new QToolButton(this);
    _closeTab->setFixedSize(22, 22);
    _closeTab->setText(i18n("Close current tab"));
    _closeTab->setToolTip(i18n("Close current tab"));
    _closeTab->setIcon(SmallIcon("tab-close"));
    _closeTab->adjustSize();
    connect(_closeTab, SIGNAL(clicked()), this, SLOT(slotCloseTab()));
    _closeTab->setEnabled(false);   // disabled when there's only 1 tab

    // tab-bar
    _tabbar = new PanelTabBar(this);
    connect(_tabbar, SIGNAL(changePanel(ListPanel*)), this, SLOT(slotChangePanel(ListPanel *)));
    connect(_tabbar, SIGNAL(closeCurrentTab()), this, SLOT(slotCloseTab()));
    connect(_tabbar, SIGNAL(newTab(const KUrl&)), this, SLOT(slotNewTab(const KUrl&)));

    _layout->addWidget(_stack, 0, 0, 1, 3);
    _layout->addWidget(_newTab, 1, 0);
    _layout->addWidget(_tabbar, 1, 1);
    _layout->addWidget(_closeTab, 1, 2);

    updateTabbarPos();

    setLayout(_layout);

    if (HIDE_ON_SINGLE_TAB) HIDE
        else SHOW
        }

void PanelManager::slotChangePanel(ListPanel *p)
{
    if (p == 0)
        return;
    _self = p;
    _self->otherPanel = _other;
    _other->otherPanel = _self;

//     _stack->setUpdatesEnabled(false);
    _stack->setCurrentWidget(_self);
    // make sure the view is focused (this also causes ListPanel::slotFocusOnMe() to be called)
    _self->view->widget()->setFocus();
//     _stack->setUpdatesEnabled(true);
}

ListPanel* PanelManager::createPanel(int type, bool setCurrent)
{
    // create the panel and add it into the widgetstack
    ListPanel * p = new ListPanel(type, _stack, _left);
    _stack->addWidget(p);

    // now, create the corrosponding tab
    _tabbar->addPanel(p, setCurrent);

    // allow close button if more than 1 tab
    if (_tabbar->count() > 1) {
        _closeTab->setEnabled(true);
        SHOW // needed if we were hidden
    }
    if (setCurrent)
        _stack->setCurrentWidget(p);

    // connect the activePanelChanged signal to enable/disable actions
    connect(p, SIGNAL(activePanelChanged(ListPanel*)), this, SLOT(slotRefreshActions()));
    return p;
}

void PanelManager::startPanel(ListPanel *panel, const KUrl& path)
{
    panel->start(path);
}

void PanelManager::saveSettings(KConfigGroup *config, const QString& key, bool localOnly)
{
    QStringList l;
    QList<int> types;
    QList<int> props;
    int i = 0, cnt = 0;
    while (cnt < _tabbar->count()) {
        ListPanel *panel = _tabbar->getPanel(i);
        if (panel) {
            l << (localOnly ? panel->realPath() : panel->virtualPath().pathOrUrl());
            types << panel->getType();
            props << panel->getProperties();
            ++cnt;
        }
        ++i;
    }
    config->writePathEntry(key, l);
    config->writeEntry(key + " Types", types);
    config->writeEntry(key + " Props", props);
}

void PanelManager::loadSettings(KConfigGroup *config, const QString& key)
{
    QStringList l = config->readPathEntry(key, QStringList());
    QList<int> types = config->readEntry(key + " Types", QList<int>());
    QList<int> props = config->readEntry(key + " Props", QList<int>());

    if (l.count() < 1)
        return;

    while (types.count() < l.count()) {
        KConfigGroup cg(krConfig, "Look&Feel");
        types << cg.readEntry("Default Panel Type", KrViewFactory::defaultViewId());
    }
    while (props.count() < l.count())
        props << 0;

    int i = 0, totalTabs = _tabbar->count();

    while (i < totalTabs && i < (int)l.count()) {
        ListPanel *panel = _tabbar->getPanel(i);
        if (panel) {
            if (panel->getType() != types[ i ])
                panel->changeType(types[ i ]);
            panel->setProperties(props[ i ]);
            panel->otherPanel = _other;
            _other->otherPanel = panel;
            panel->func->files()->vfs_enableRefresh(true);
            panel->func->immediateOpenUrl(KUrl(l[ i ]), true);
        }
        ++i;
    }

    while (i <  totalTabs)
        slotCloseTab(--totalTabs);

    for (; i < (int)l.count(); i++)
        slotNewTab(KUrl(l[i]), false, types[ i ], props[ i ]);

    // this is needed so that all tab labels get updated
    layoutTabs();
}

void PanelManager::layoutTabs()
{
    // delayed url refreshes may be pending -
    // delay the layout too so it happens after them
    QTimer::singleShot(0, _tabbar, SLOT(layoutTabs()));
}

void PanelManager::slotNewTab(const KUrl& url, bool setCurrent, int type, int props)
{
    if (type == -1) {
        KConfigGroup group(krConfig, "Look&Feel");
        type = group.readEntry("Default Panel Type", KrViewFactory::defaultViewId());
    }
    ListPanel *p = createPanel(type, setCurrent);
    // update left/right pointers
    p->otherPanel = _other;
    if (setCurrent) {
        _self = p;
        _other->otherPanel = _self;
    }
    startPanel(p, url);
    p->setProperties(props);
}

void PanelManager::slotNewTab()
{
    slotNewTab(QDir::home().absolutePath());
}

void PanelManager::slotCloseTab()
{
    if (_tabbar->count() <= 1)    /* if this is the last tab don't close it */
        return ;

    // setup current one
    ListPanel * oldp;
    _self = _tabbar->removeCurrentPanel(oldp);
    _stack->setCurrentWidget(_self);
    _stack->removeWidget(oldp);
    deletePanel(oldp);

    // setup pointers
    _self->otherPanel = _other;
    _other->otherPanel = _self;
    _self->slotFocusOnMe();

    // disable close button if only 1 tab is left
    if (_tabbar->count() == 1) {
        _closeTab->setEnabled(false);
        if (HIDE_ON_SINGLE_TAB) HIDE
        }
}

void PanelManager::slotCloseTab(int index)
{
    ListPanel *panel = _tabbar->getPanel(index);
    if (panel) {
        ListPanel *oldp = panel;
        disconnect(oldp);
        if (index == _tabbar->currentIndex()) {
            ListPanel *newCurrentPanel = _tabbar->getPanel(0);
            if (newCurrentPanel != 0) {
                _tabbar->setCurrentIndex(0);
                _self = newCurrentPanel;
                _self->otherPanel = _other;
                _other->otherPanel = _self;
            }
        }
        _tabbar->removeTab(index);

        _stack->removeWidget(oldp);
        deletePanel(oldp);
    }

    if (_tabbar->count() == 1) {
        _closeTab->setEnabled(false);
        if (HIDE_ON_SINGLE_TAB) HIDE
        }
}

void PanelManager::slotRefreshActions()
{
    krCloseTab->setEnabled(_tabbar->count() > 1);
    krCloseInactiveTabs->setEnabled(_tabbar->count() > 1);
    krCloseDuplicatedTabs->setEnabled(_tabbar->count() > 1);
    krNextTab->setEnabled(_tabbar->count() > 1);
    krPreviousTab->setEnabled(_tabbar->count() > 1);
    ListPanel *actPanel = _tabbar->getPanel(activeTab());
    if (actPanel) {
        bool locked = actPanel->isLocked();
        Krusader::actLockTab->setText(locked ? i18n("Unlock Tab") : i18n("Lock Tab"));
    }
}

void PanelManager::updateTabbarPos()
{
    KConfigGroup group(krConfig, "Look&Feel");
    if(group.readEntry("Tab Bar Position", "bottom") == "top") {
        _layout->addWidget(_stack, 2, 0, 1, 3);
        _tabbar->setShape(QTabBar::RoundedNorth);
    } else {
        _layout->addWidget(_stack, 0, 0, 1, 3);
        _tabbar->setShape(QTabBar::RoundedSouth);
    }
}

int PanelManager::activeTab()
{
    return _tabbar->currentIndex();
}

void PanelManager::setActiveTab(int panelIndex)
{
    _tabbar->setCurrentIndex(panelIndex);
    slotChangePanel(_tabbar->getPanel(panelIndex));
}

void PanelManager::setCurrentTab(int panelIndex)
{
    _tabbar->setCurrentIndex(panelIndex);
    _self = _tabbar->getPanel(panelIndex);
    _self->otherPanel = _other;
    _other->otherPanel = _self;

    _stack->setCurrentWidget(_self);
}

void PanelManager::slotRecreatePanels()
{
    int actTab = activeTab();

    for (int i = 0; i != _tabbar->count(); i++) {
        ListPanel *updatedPanel = _tabbar->getPanel(i);

        ListPanel *oldPanel = updatedPanel;
        int type = oldPanel->getType();
        int properties = oldPanel->getProperties();

        ListPanel *newPanel = new ListPanel(type, _stack, _left);
        newPanel->setProperties(properties);
        _tabbar->changePanel(i, newPanel);

        _stack->insertWidget(i, newPanel);
        _stack->removeWidget(oldPanel);

        disconnect(oldPanel);
        connect(newPanel, SIGNAL(activePanelChanged(ListPanel*)), this, SLOT(slotRefreshActions()));
        connect(newPanel, SIGNAL(pathChanged(ListPanel*)), _tabbar, SLOT(updateTab(ListPanel*)));

        newPanel->otherPanel = _other;
        if (_other->otherPanel == oldPanel)
            _other->otherPanel = newPanel;
        updatedPanel = newPanel;
        newPanel->start(oldPanel->virtualPath(), true);
        if (_self == oldPanel) {
            _self = newPanel;
            _self->otherPanel = _other;
            _other->otherPanel = _self;
        }
        deletePanel(oldPanel);

        _tabbar->updateTab(newPanel);
    }

    updateTabbarPos();

    setActiveTab(actTab);
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
    if (ACTIVE_PANEL == p)
        ACTIVE_PANEL = _self;

    if (p && p->func && p->func->files() && !p->func->files()->vfs_canDelete()) {
        connect(p->func->files(), SIGNAL(deleteAllowed()), p, SLOT(deleteLater()));
        p->func->files()->vfs_requestDelete();
        return;
    }
    delete p;
}

void PanelManager::swapPanels()
{
    _left = !_left;
    _selfPtr = _left ? &MAIN_VIEW->left : &MAIN_VIEW->right;
    _otherPtr = _left ? &MAIN_VIEW->right : &MAIN_VIEW->left;
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

void PanelManager::slotLockTab()
{
    if (ACTIVE_PANEL)
        ACTIVE_PANEL->setLocked(!ACTIVE_PANEL->isLocked());
}

#include "panelmanager.moc"
