/***************************************************************************
                               krusaderview.cpp
                            -------------------
   copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
   e-mail               : krusader@users.sourceforge.net
   web site             : http://krusader.sourceforge.net
---------------------------------------------------------------------------
 Description
***************************************************************************

 A

    db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
    88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
    88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
    88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
    88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
    YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                    S o u r c e    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "krusaderview.h"

#include <QGridLayout>
#include <QList>
#include <QKeyEvent>
#include <QEvent>
#include <QtGui/QClipboard>

#include <kstatusbar.h>
#include <kmenubar.h>
#include <kshortcut.h>
#include <ktoolbar.h>
#include <ktoggleaction.h>
#include <klibloader.h>

#include "krusader.h"
#include "kractions.h"
#include "krslots.h"
#include "defaults.h"
#include "Panel/listpanel.h"
#include "Panel/panelfunc.h"
#include "GUI/kcmdline.h"
#include "GUI/kfnkeys.h"
#include "GUI/terminaldock.h"
#include "resources.h"
#include "panelmanager.h"
#include "GUI/profilemanager.h"
#include "Dialogs/percentalsplitter.h"
#include "krservices.h"
#include "Panel/krviewfactory.h"

KrusaderView::KrusaderView(QWidget *parent) : QWidget(parent),
    activeMng(0)
{
}

void KrusaderView::start(KConfigGroup &cfg, bool restoreSettings, QStringList leftTabs, QStringList rightTabs)
{
    ////////////////////////////////
    // make a 1x1 mainLayout, it will auto-expand:
    mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    // vertical splitter
    vert_splitter = new QSplitter(this);   // splits between panels and terminal/cmdline
    vert_splitter->setOrientation(Qt::Vertical);
    // horizontal splitter
    horiz_splitter = new PercentalSplitter(vert_splitter);
    (_terminalDock = new TerminalDock(vert_splitter, krApp))->hide();     // create it hidden

    // create a command line thing
    _cmdLine = new KCMDLine(this);

    // add a panel manager for each side of the splitter
    leftMng  = createManager(true);
    rightMng = createManager(false);
    leftMng->setOtherManager(rightMng);
    rightMng->setOtherManager(leftMng);

    // make the left panel focused at program start
    activeMng = leftMng;

    // create the function keys widget
    _fnKeys = new KFnKeys(this, krApp);
    _fnKeys->hide();
    _fnKeys->setWhatsThis(i18n("Function keys allow performing fast "
                              "operations on files."));

    // and insert the whole thing into the main layout... at last
    mainLayout->addWidget(vert_splitter, 0, 0);    //<>
    mainLayout->addWidget(_cmdLine, 1, 0);
    mainLayout->addWidget(_fnKeys, 2, 0);
    mainLayout->activate();

    // get the last saved sizes of the splitter

    QList<int> lst = cfg.readEntry("Splitter Sizes", QList<int>());
    if (lst.count() != 2) {
        lst.clear();
        lst.push_back(100);
        lst.push_back(100);
    } else if (lst[0] < 1 && lst[1] < 1) {
        lst[ 0 ] = 100;
        lst[ 1 ] = 100;
    }

    horiz_splitter->setSizes(lst);

    verticalSplitterSizes = cfg.readEntry("Terminal Emulator Splitter Sizes", QList<int> ());

    if (verticalSplitterSizes.count() != 2) {
        verticalSplitterSizes.clear();
        verticalSplitterSizes << 100 << 100;
    }  else if (verticalSplitterSizes[0] < 1 && verticalSplitterSizes[1] < 1) {
        verticalSplitterSizes[0] = 100;
        verticalSplitterSizes[1] = 100;
    }


    KUrl leftUrl(QDir::homePath()), rightUrl(QDir::homePath());
    if(leftTabs.count())
        leftUrl = leftTabs[0];
    if(rightTabs.count())
        rightUrl = rightTabs[0];
    leftPanel()->start(leftUrl);
    rightPanel()->start(rightUrl);

    ACTIVE_PANEL->gui->slotFocusOnMe();  // left starts out active

    for (int i = 1; i < leftTabs.count(); i++)
        leftMng->slotNewTab(leftTabs[ i ], false);

    for (int j = 1; j < rightTabs.count(); j++)
        rightMng->slotNewTab(rightTabs[ j ], false);

    // this is needed so that all tab labels get updated
    leftMng->layoutTabs();
    rightMng->layoutTabs();

    if(restoreSettings) {
        if(!leftTabs.count())
            leftMng->loadSettings(KConfigGroup(&cfg, "Left Tab Bar"));
        if(!rightTabs.count())
            rightMng->loadSettings(KConfigGroup(&cfg, "Right Tab Bar"));
        if (cfg.readEntry("Left Side Is Active", false))
            leftPanel()->slotFocusOnMe();
        else
            rightPanel()->slotFocusOnMe();
    }
}

void KrusaderView::updateGUI(KConfigGroup &cfg)
{
    if (!cfg.readEntry("Show Cmd Line", _ShowCmdline)) {
        cmdLine()->hide();
        KrActions::actToggleCmdline->setChecked(false);
    } else {
        cmdLine()->show();
        KrActions::actToggleCmdline->setChecked(true);
    }
    // update the Fn bar to the shortcuts selected by the user
    fnKeys()->updateShortcuts();
    if (!cfg.readEntry("Show FN Keys", _ShowFNkeys)) {
        fnKeys()->hide();
        KrActions::actToggleFnkeys->setChecked(false);
    } else {
        fnKeys()->show();
        KrActions::actToggleFnkeys->setChecked(true);
    }
    // set vertical mode
    if (cfg.readEntry("Vertical Mode", false)) {
        toggleVerticalMode();
    }
    if (cfg.readEntry("Show Terminal Emulator", _ShowTerminalEmulator)) {
        slotTerminalEmulator(true);   // create konsole_part
    };
}

void KrusaderView::setPanelSize(bool leftPanel, int percent)
{
    QList<int> panelSizes = horiz_splitter->sizes();
    int totalSize = panelSizes[0] + panelSizes[1];

    if (leftPanel) {
        panelSizes[0] = totalSize * percent / 100;
        panelSizes[1] = totalSize * (100 - percent) / 100;
    } else { // == RIGHT_PANEL
        panelSizes[0] = totalSize * (100 - percent) / 100;
        panelSizes[1] = totalSize * percent / 100;
    }

    horiz_splitter->setSizes(panelSizes);
}

PanelManager *KrusaderView::createManager(bool left)
{
    PanelManager *p = new PanelManager(horiz_splitter, krApp, left);
    connect(p, SIGNAL(draggingTab(PanelManager*, QMouseEvent*)),
                     SLOT(draggingTab(PanelManager*, QMouseEvent*)));
    connect(p, SIGNAL(draggingTabFinished(PanelManager*, QMouseEvent*)),
                     SLOT(draggingTabFinished(PanelManager*, QMouseEvent*)));
    connect(p, SIGNAL(pathChanged(ListPanel*)), SLOT(slotPathChanged(ListPanel*)));
    connect(p, SIGNAL(setActiveManager(PanelManager*)),
                     SLOT(slotSetActiveManager(PanelManager*)));

    return p;
}

ListPanel* KrusaderView::leftPanel()
{
    return leftMng->currentPanel()->gui;
}

ListPanel* KrusaderView::rightPanel()
{
    return rightMng->currentPanel()->gui;
}

// updates the command line whenever current panel or its path changes
//////////////////////////////////////////////////////////
void KrusaderView::slotPathChanged(ListPanel *p)
{
    if(p == ACTIVE_PANEL) {
        _cmdLine->setCurrent(p->realPath());
        KConfigGroup cfg = krConfig->group("General");
        if (cfg.readEntry("Send CDs", _SendCDs)) { // hopefully, this is cached in kconfig
            _terminalDock->sendCd(p->realPath());
        }
    }
}

int KrusaderView::getFocusCandidates(QVector<QWidget*> &widgets)
{
    ACTIVE_PANEL->gui->getFocusCandidates(widgets);
    if(_terminalDock->isTerminalVisible())
        widgets << _terminalDock;
    if(_cmdLine->isVisible())
        widgets << _cmdLine;

    for(int i = 0; i < widgets.count(); i++) {
        if(widgets[i] == focusWidget() || widgets[i]->focusWidget() == focusWidget())
            return i;
    }
    return -1;
}

void KrusaderView::focusUp()
{
    QVector<QWidget*> widgets;
    int currentFocus = getFocusCandidates(widgets);

    if(currentFocus < 0)
        return;
    currentFocus--;

    if(currentFocus >= 0 && currentFocus < widgets.count())
        widgets[currentFocus]->setFocus();
}

void KrusaderView::focusDown()
{
    QVector<QWidget*> widgets;
    int currentFocus = getFocusCandidates(widgets);

    if(currentFocus < 0)
        return;
    currentFocus++;

    if(currentFocus < widgets.count())
        widgets[currentFocus]->setFocus();
}

void KrusaderView::cmdLineFocus()    // command line receive's keyboard focus
{
    _cmdLine->setFocus();
}

void KrusaderView::cmdLineUnFocus()   // return focus to the active panel
{
    ACTIVE_PANEL->gui->slotFocusOnMe();
}

// Tab - switch focus
void KrusaderView::panelSwitch()
{
    ACTIVE_PANEL->otherPanel()->gui->slotFocusOnMe();
}

void KrusaderView::slotSetActiveManager(PanelManager *manager)
{
    activeMng = manager;
    slotPathChanged(manager->currentPanel()->gui);
}

void KrusaderView::swapSides()
{
    QList<int> lst = horiz_splitter->sizes();

    horiz_splitter->addWidget(leftMng);

    int old = lst[ 0 ];
    lst[ 0 ] = lst [ 1 ];
    lst[ 1 ] = old;

    horiz_splitter->setSizes(lst);

    PanelManager *tmpMng = leftMng;
    leftMng = rightMng;
    rightMng = tmpMng;

    leftMng->setLeft(true);
    rightMng->setLeft(false);

    leftPanel()->updateGeometry();
    rightPanel()->updateGeometry();
}

void KrusaderView::slotTerminalEmulator(bool show)
{
    KConfigGroup cfg = krConfig->group("Look&Feel");
    bool fullscreen = cfg.readEntry("Fullscreen Terminal Emulator", false);
    static bool fnKeysShown = true; // first time init. should be overridden
    static bool cmdLineShown = true;
    static bool statusBarShown = true;
    static bool toolBarShown = true;
    static bool menuBarShown = true;

    if (!show) {    // hiding the terminal
        ACTIVE_PANEL->gui->slotFocusOnMe();
        if (_terminalDock->isTerminalVisible() && !fullscreen)
            verticalSplitterSizes = vert_splitter->sizes();

        _terminalDock->hide();
        QList<int> newSizes;
        newSizes.push_back(vert_splitter->height());
        newSizes.push_back(0);
        vert_splitter->setSizes(newSizes);
        // in full screen, we unhide everything that was hidden before
        if (fullscreen) {
            leftMng->show();
            rightMng->show();
            if (fnKeysShown) _fnKeys->show();
            if (cmdLineShown) _cmdLine->show();
            if (statusBarShown) krApp->statusBar()->show();
            if (toolBarShown) {
                krApp->toolBar()->show();
                krApp->toolBar("actionsToolBar")->show();
            }
            if (menuBarShown) krApp->menuBar()->show();
        }
        return ;
    }
    // else implied
    _terminalDock->initialise();
    if (_terminalDock->isInitialised()) {        // if we succeeded in creating the konsole
        if (!verticalSplitterSizes.empty())
            vert_splitter->setSizes(getTerminalEmulatorSplitterSizes());

        _terminalDock->show();
        slotPathChanged(ACTIVE_PANEL->gui);

        _terminalDock->setFocus();

        krToggleTerminal->setChecked(true);
        // in full screen mode, we hide everything else, but first, see what was actually shown
        if (fullscreen) {
            fnKeysShown = !_fnKeys->isHidden();
            cmdLineShown = !_cmdLine->isHidden();
            statusBarShown = !krApp->statusBar()->isHidden();
            toolBarShown = !krApp->toolBar()->isHidden();
            menuBarShown = !krApp->menuBar()->isHidden();
            leftMng->hide();
            rightMng->hide();
            _fnKeys->hide();
            _cmdLine->hide();
            krApp->statusBar()->hide();
            krApp->toolBar()->hide();
            krApp->toolBar("actionsToolBar")->hide();
            krApp->menuBar()->hide();
        }
    } else {
        _terminalDock->hide();
        krToggleTerminal->setChecked(false);
    }
}

void KrusaderView::focusTerminalEmulator()
{
    if (_terminalDock->isTerminalVisible())
        _terminalDock->setFocus();
}

void KrusaderView::switchFullScreenTE()
{
    if (_terminalDock->isTerminalVisible()) {
        KConfigGroup grp = krConfig->group("Look&Feel");
        bool fullscreen = grp.readEntry("Fullscreen Terminal Emulator", false);
        slotTerminalEmulator(false);
        grp.writeEntry("Fullscreen Terminal Emulator", !fullscreen);
        slotTerminalEmulator(true);
    }
}

QList<int> KrusaderView::getTerminalEmulatorSplitterSizes()
{
    if (_terminalDock->isVisible())
        return vert_splitter->sizes();
    else
        return verticalSplitterSizes;
}

void KrusaderView::killTerminalEmulator()
{
    slotTerminalEmulator(false);    // hide the docking widget
    krToggleTerminal->setChecked(false);
}


void KrusaderView::profiles(QString profileName)
{
    ProfileManager profileManager("Panel");
    profileManager.hide();
    connect(&profileManager, SIGNAL(saveToProfile(QString)), this, SLOT(savePanelProfiles(QString)));
    connect(&profileManager, SIGNAL(loadFromProfile(QString)), this, SLOT(loadPanelProfiles(QString)));
    if (profileName.isEmpty())
        profileManager.profilePopup();
    else
        profileManager.loadProfile(profileName);
}

void KrusaderView::loadPanelProfiles(QString group)
{
    KConfigGroup ldg(krConfig, group);
    leftMng->loadSettings(KConfigGroup(&ldg, "Left Tabs"));
    rightMng->loadSettings(KConfigGroup(&ldg, "Right Tabs"));
    if (ldg.readEntry("Left Side Is Active", true))
        leftPanel()->slotFocusOnMe();
    else
        rightPanel()->slotFocusOnMe();
}

void KrusaderView::savePanelProfiles(QString group)
{
    KConfigGroup svr(krConfig, group);

    svr.writeEntry("Vertical Mode", isVertical());
    svr.writeEntry("Left Side Is Active", ACTIVE_PANEL->gui->isLeft());
    leftMng->saveSettings(KConfigGroup(&svr, "Left Tabs"), false);
    rightMng->saveSettings(KConfigGroup(&svr, "Right Tabs"), false);
}

void KrusaderView::toggleVerticalMode()
{
    if (horiz_splitter->orientation() == Qt::Vertical) {
        horiz_splitter->setOrientation(Qt::Horizontal);
        KrActions::actVerticalMode->setText(i18n("Vertical Mode"));
        KrActions::actVerticalMode->setIcon(KIcon("view-split-top-bottom"));
    } else {
        horiz_splitter->setOrientation(Qt::Vertical);
        KrActions::actVerticalMode->setText(i18n("Horizontal Mode"));
        KrActions::actVerticalMode->setIcon(KIcon("view-split-left-right"));
    }
}

void KrusaderView::saveSettings(KConfigGroup &cfg)
{
    QList<int> lst = horiz_splitter->sizes();
    cfg.writeEntry("Splitter Sizes", lst);
    if (!getTerminalEmulatorSplitterSizes().isEmpty())
        cfg.writeEntry("Terminal Emulator Splitter Sizes", getTerminalEmulatorSplitterSizes());
    cfg.writeEntry("Vertical Mode", isVertical());
    cfg.writeEntry("Left Side Is Active", ACTIVE_PANEL->gui->isLeft());
    leftMng->saveSettings(KConfigGroup(&cfg, "Left Tab Bar"), true);
    rightMng->saveSettings(KConfigGroup(&cfg, "Right Tab Bar"), true);
}

bool KrusaderView::cursorIsOnOtherSide(PanelManager *of, const QPoint &globalPos)
{
    int border = -1;
    int pos = -1;

    if (horiz_splitter->orientation() == Qt::Horizontal) {
        pos = globalPos.x();
        if(of == leftMng)
            border = leftMng->mapToGlobal(QPoint(leftMng->width(), 0)).x();
        else
            border = rightMng->mapToGlobal(QPoint(0, 0)).x();
    } else {
        pos = globalPos.y();
        if(of == leftMng)
            border = leftMng->mapToGlobal(QPoint(0, leftMng->height())).y();
        else
            border = rightMng->mapToGlobal(QPoint(0, 0)).y();
    }

    return (of == leftMng) ? pos > border : pos < border;
}

void KrusaderView::draggingTab(PanelManager *from, QMouseEvent *e)
{
    QString icon;
    if (horiz_splitter->orientation() == Qt::Horizontal)
        icon = (from == leftMng) ? "arrow-right" : "arrow-left";
    else
        icon = (from == leftMng) ? "arrow-down" : "arrow-up";

    QCursor cursor(KIcon(icon).pixmap(22));

    if (cursorIsOnOtherSide(from, e->globalPos())) {
        if(!qApp->overrideCursor())
            qApp->setOverrideCursor(cursor);
    } else
        qApp->restoreOverrideCursor();
}

void KrusaderView::draggingTabFinished(PanelManager *from, QMouseEvent *e)
{
    qApp->restoreOverrideCursor();

    if (cursorIsOnOtherSide(from, e->globalPos())) 
        from->moveTabToOtherSide();
}


#include "krusaderview.moc"
