/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krusaderview.h"

// QtCore
#include <QDebug>
#include <QEvent>
#include <QList>
// QtGui
#include <QClipboard>
#include <QKeyEvent>
// QtWidgets
#include <QApplication>
#include <QGridLayout>
#include <QMenuBar>
#include <QStatusBar>

#include <KLocalizedString>
#include <KToggleAction>
#include <KToolBar>

#include "Dialogs/percentalsplitter.h"
#include "GUI/kcmdline.h"
#include "GUI/kfnkeys.h"
#include "GUI/profilemanager.h"
#include "GUI/terminaldock.h"
#include "Panel/listpanel.h"
#include "Panel/panelfunc.h"
#include "defaults.h"
#include "icon.h"
#include "kractions.h"
#include "krservices.h"
#include "krslots.h"
#include "krusader.h"
#include "panelmanager.h"

KrusaderView::KrusaderView(QWidget *parent)
    : QWidget(parent)
    , activeMng(nullptr)
{
}

void KrusaderView::start(const KConfigGroup &cfg, bool restoreSettings, const QList<QUrl> &leftTabs, const QList<QUrl> &rightTabs)
{
    ////////////////////////////////
    // make a 1x1 mainLayout, it will auto-expand:
    mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    // vertical splitter
    vert_splitter = new QSplitter(this); // splits between panels and terminal/cmdline
    vert_splitter->setOrientation(Qt::Vertical);
    // horizontal splitter
    horiz_splitter = new PercentalSplitter(vert_splitter);
    (_terminalDock = new TerminalDock(vert_splitter, krApp))->hide(); // create it hidden

    // create a command line thing
    _cmdLine = new KCMDLine(this);

    // add a panel manager for each side of the splitter
    leftMng = createManager(true);
    rightMng = createManager(false);
    leftMng->setOtherManager(rightMng);
    rightMng->setOtherManager(leftMng);

    // make the left panel focused at program start
    activeMng = leftMng;

    // create the function keys widget
    _fnKeys = new KFnKeys(this, krApp);
    _fnKeys->hide();
    _fnKeys->setWhatsThis(
        i18n("Function keys allow performing fast "
             "operations on files."));

    // and insert the whole thing into the main layout... at last
    mainLayout->addWidget(vert_splitter, 0, 0); //<>
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
        lst[0] = 100;
        lst[1] = 100;
    }

    horiz_splitter->setSizes(lst);

    verticalSplitterSizes = cfg.readEntry("Terminal Emulator Splitter Sizes", QList<int>());
    if (verticalSplitterSizes.count() != 2 || (verticalSplitterSizes[0] < 1 && verticalSplitterSizes[1] < 1)) {
        verticalSplitterSizes.clear();
        verticalSplitterSizes << 100 << 100;
    }

    leftPanel()->start(leftTabs.isEmpty() ? QUrl::fromLocalFile(QDir::homePath()) : leftTabs.at(0));
    rightPanel()->start(rightTabs.isEmpty() ? QUrl::fromLocalFile(QDir::homePath()) : rightTabs.at(0));

    activePanel()->gui->slotFocusOnMe(); // left starts out active

    for (int i = 1; i < leftTabs.count(); i++)
        leftMng->slotNewTab(leftTabs.at(i), false);

    for (int j = 1; j < rightTabs.count(); j++)
        rightMng->slotNewTab(rightTabs.at(j), false);

    // this is needed so that all tab labels get updated
    leftMng->layoutTabs();
    rightMng->layoutTabs();

    if (restoreSettings) {
        if (leftTabs.isEmpty())
            leftMng->loadSettings(KConfigGroup(&cfg, "Left Tab Bar"));
        if (rightTabs.isEmpty())
            rightMng->loadSettings(KConfigGroup(&cfg, "Right Tab Bar"));
        if (cfg.readEntry("Left Side Is Active", false))
            leftPanel()->slotFocusOnMe();
        else
            rightPanel()->slotFocusOnMe();
    }
}

void KrusaderView::updateGUI(const KConfigGroup &cfg)
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
        setTerminalEmulator(true); // create konsole_part
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
    auto *panelManager = new PanelManager(horiz_splitter, krApp, left);
    connect(panelManager, &PanelManager::draggingTab, this, &KrusaderView::draggingTab);
    connect(panelManager, &PanelManager::draggingTabFinished, this, &KrusaderView::draggingTabFinished);
    connect(panelManager, &PanelManager::pathChanged, this, &KrusaderView::slotPathChanged);
    connect(panelManager, &PanelManager::setActiveManager, this, &KrusaderView::slotSetActiveManager);

    return panelManager;
}

void KrusaderView::updateCurrentActivePath()
{
    const QString path = activePanel()->gui->lastLocalPath();

    _cmdLine->setCurrent(path);
    KConfigGroup cfg = krConfig->group("General");
    if (_terminalDock->isInitialised() && cfg.readEntry("Send CDs", _SendCDs)) {
        _terminalDock->sendCd(path);
    }
}

KrPanel *KrusaderView::activePanel() const
{
    // active manager might not be set yet
    return activeMng ? activeMng->currentPanel() : nullptr;
}

ListPanel *KrusaderView::leftPanel() const
{
    return leftMng->currentPanel()->gui;
}

ListPanel *KrusaderView::rightPanel() const
{
    return rightMng->currentPanel()->gui;
}

// updates the command line whenever current panel or its path changes
void KrusaderView::slotPathChanged(ListPanel *listPanel)
{
    if (listPanel == activePanel()) {
        updateCurrentActivePath();
    }
}

int KrusaderView::getFocusCandidates(QVector<QWidget *> &widgets)
{
    activePanel()->gui->getFocusCandidates(widgets);
    if (_terminalDock->isTerminalVisible())
        widgets << _terminalDock;
    if (_cmdLine->isVisible())
        widgets << _cmdLine;

    for (int i = 0; i < widgets.count(); i++) {
        if (widgets[i] == focusWidget() || widgets[i]->focusWidget() == focusWidget())
            return i;
    }
    return -1;
}

void KrusaderView::focusUp()
{
    QVector<QWidget *> widgets;
    int currentFocus = getFocusCandidates(widgets);
    qDebug() << "Focus UP - current: " << focusWidget() << " - widgets are: " << widgets.toList();

    if (currentFocus < 0)
        return;
    currentFocus--;

    if (currentFocus >= 0 && currentFocus < widgets.count()) {
        widgets[currentFocus]->setFocus();

        if (currentFocus == 0)
            activePanel()->gui->editLocation();

        qDebug() << "Focus moved to : " << focusWidget();
    }
}

void KrusaderView::focusDown()
{
    QVector<QWidget *> widgets;
    int currentFocus = getFocusCandidates(widgets);
    qDebug() << "Focus DOWN - current: " << focusWidget() << " - widgets are: " << widgets.toList();

    if (currentFocus < 0)
        return;
    currentFocus++;

    if (currentFocus < widgets.count()) {
        widgets[currentFocus]->setFocus();

        // set the urlnavigator before to prevent focus stealing
        if (currentFocus == 1)
            activePanel()->gui->navigateLocation();

        qDebug() << "Focus moved to : " << focusWidget();
    }
}
void KrusaderView::cmdLineFocus() // command line receives keyboard focus
{
    _cmdLine->setFocus();
}

void KrusaderView::cmdLineUnFocus() // return focus to the active panel
{
    activePanel()->gui->slotFocusOnMe();
}

// Tab - switch focus
void KrusaderView::panelSwitch()
{
    activePanel()->otherPanel()->gui->slotFocusOnMe();
}

void KrusaderView::slotSetActiveManager(PanelManager *manager)
{
    activeMng = manager;
    updateCurrentActivePath();
}

void KrusaderView::swapSides()
{
    QList<int> lst = horiz_splitter->sizes();

    horiz_splitter->addWidget(leftMng);

    int old = lst[0];
    lst[0] = lst[1];
    lst[1] = old;

    horiz_splitter->setSizes(lst);

    PanelManager *tmpMng = leftMng;
    leftMng = rightMng;
    rightMng = tmpMng;

    leftMng->setLeft(true);
    rightMng->setLeft(false);

    leftPanel()->updateGeometry();
    rightPanel()->updateGeometry();
}

void KrusaderView::setTerminalEmulator(bool show, bool fullscreen)
{
    qDebug() << "show=" << show << " fullscreen=" << fullscreen;

    static bool fnKeysShown = true; // first time init. should be overridden
    static bool cmdLineShown = true;
    static bool statusBarShown = true;
    static bool mainToolBarShown = true;
    static bool jobToolBarShown = true;
    static bool actionToolBarShown = true;
    static bool menuBarShown = true;
    static bool terminalEmulatorShown = true;

    if (show) {
        if (fullscreen) {
            // save what is shown
            fnKeysShown = !_fnKeys->isHidden();
            cmdLineShown = !_cmdLine->isHidden();
            statusBarShown = !krApp->statusBar()->isHidden();
            mainToolBarShown = !krApp->toolBar()->isHidden();
            jobToolBarShown = !krApp->toolBar("jobToolBar")->isHidden();
            actionToolBarShown = !krApp->toolBar("actionToolBar")->isHidden();
            menuBarShown = !krApp->menuBar()->isHidden();
            terminalEmulatorShown = _terminalDock->isTerminalVisible();
        }

        if (!_terminalDock->isTerminalVisible()) {
            // show terminal
            const bool isInitialized = _terminalDock->initialise();
            if (!isInitialized) {
                _terminalDock->hide();
                KrActions::actToggleTerminal->setChecked(false);
                return;
            }

            _terminalDock->show();
            _terminalDock->setFocus();
            updateCurrentActivePath();
            KrActions::actToggleTerminal->setChecked(true);
        } else if (fullscreen) {
            // save current terminal size before going to fullscreen
            verticalSplitterSizes = vert_splitter->sizes();
        }

        if (fullscreen) {
            // hide everything else
            leftMng->hide();
            rightMng->hide();
            _fnKeys->hide();
            _cmdLine->hide();
            krApp->statusBar()->hide();
            krApp->toolBar()->hide();
            krApp->toolBar("jobToolBar")->hide();
            krApp->toolBar("actionToolBar")->hide();
            krApp->menuBar()->hide();
            // fix showing nothing if terminal is open but splitter widget size is zero
            vert_splitter->setSizes(QList<int>() << 0 << vert_splitter->height());
        } else {
            vert_splitter->setSizes(verticalSplitterSizes);
        }
    } else { // hide
        const bool isFullscreen = isTerminalEmulatorFullscreen();
        if (!(fullscreen && terminalEmulatorShown)) {
            // hide terminal emulator
            activePanel()->gui->slotFocusOnMe();
            if (_terminalDock->isTerminalVisible() && !isFullscreen) {
                verticalSplitterSizes = vert_splitter->sizes();
            }
            _terminalDock->hide();
            KrActions::actToggleTerminal->setChecked(false);
        } else {
            // not fullscreen anymore but terminal is still visible
            vert_splitter->setSizes(verticalSplitterSizes);
        }
        if (isFullscreen) {
            // restore: unhide everything that was hidden before
            leftMng->show();
            rightMng->show();
            if (fnKeysShown)
                _fnKeys->show();
            if (cmdLineShown)
                _cmdLine->show();
            if (statusBarShown)
                krApp->statusBar()->show();
            if (mainToolBarShown)
                krApp->toolBar()->show();
            if (jobToolBarShown)
                krApp->toolBar("jobToolBar")->show();
            if (actionToolBarShown)
                krApp->toolBar("actionToolBar")->show();
            if (menuBarShown)
                krApp->menuBar()->show();
        }
    }
}

void KrusaderView::focusTerminalEmulator()
{
    if (_terminalDock->isTerminalVisible())
        _terminalDock->setFocus();
}

void KrusaderView::toggleFullScreenTerminalEmulator()
{
    setTerminalEmulator(!isTerminalEmulatorFullscreen(), true);
}

bool KrusaderView::isTerminalEmulatorFullscreen()
{
    return leftMng->isHidden() && rightMng->isHidden();
}

void KrusaderView::profiles(const QString &profileName)
{
    ProfileManager profileManager("Panel", this);
    profileManager.hide();
    connect(&profileManager, &ProfileManager::saveToProfile, this, &KrusaderView::savePanelProfiles);
    connect(&profileManager, &ProfileManager::loadFromProfile, this, &KrusaderView::loadPanelProfiles);
    if (profileName.isEmpty())
        profileManager.profilePopup();
    else
        profileManager.loadProfile(profileName);
}

void KrusaderView::loadPanelProfiles(const QString &group)
{
    KConfigGroup ldg(krConfig, group);
    leftMng->loadSettings(KConfigGroup(&ldg, "Left Tabs"));
    rightMng->loadSettings(KConfigGroup(&ldg, "Right Tabs"));
    if (ldg.readEntry("Left Side Is Active", true))
        leftPanel()->slotFocusOnMe();
    else
        rightPanel()->slotFocusOnMe();
}

void KrusaderView::savePanelProfiles(const QString &group)
{
    KConfigGroup svr(krConfig, group);

    svr.writeEntry("Vertical Mode", isVertical());
    svr.writeEntry("Left Side Is Active", activePanel()->gui->isLeft());
    leftMng->saveSettings(KConfigGroup(&svr, "Left Tabs"), false);
    rightMng->saveSettings(KConfigGroup(&svr, "Right Tabs"), false);
}

void KrusaderView::toggleVerticalMode()
{
    if (horiz_splitter->orientation() == Qt::Vertical) {
        horiz_splitter->setOrientation(Qt::Horizontal);
        KrActions::actVerticalMode->setText(i18n("Vertical Mode"));
        KrActions::actVerticalMode->setIcon(Icon("view-split-top-bottom"));
    } else {
        horiz_splitter->setOrientation(Qt::Vertical);
        KrActions::actVerticalMode->setText(i18n("Horizontal Mode"));
        KrActions::actVerticalMode->setIcon(Icon("view-split-left-right"));
    }
}

void KrusaderView::saveSettings(KConfigGroup &cfg)
{
    QList<int> lst = horiz_splitter->sizes();
    cfg.writeEntry("Splitter Sizes", lst);
    QList<int> vertSplitterSizes = _terminalDock->isVisible()
            && !isTerminalEmulatorFullscreen()
            // fix sizes() not returning correct values on fullscreen+shutdown
            && vert_splitter->sizes().first() != 0
        ? vert_splitter->sizes()
        : verticalSplitterSizes;
    cfg.writeEntry("Terminal Emulator Splitter Sizes", vertSplitterSizes);
    cfg.writeEntry("Vertical Mode", isVertical());
    cfg.writeEntry("Left Side Is Active", activePanel()->gui->isLeft());
    leftMng->saveSettings(KConfigGroup(&cfg, "Left Tab Bar"), true);
    rightMng->saveSettings(KConfigGroup(&cfg, "Right Tab Bar"), true);
}

bool KrusaderView::cursorIsOnOtherSide(PanelManager *of, const QPoint &globalPos)
{
    int border = -1;
    int pos = -1;

    if (horiz_splitter->orientation() == Qt::Horizontal) {
        pos = globalPos.x();
        if (of == leftMng)
            border = leftMng->mapToGlobal(QPoint(leftMng->width(), 0)).x();
        else
            border = rightMng->mapToGlobal(QPoint(0, 0)).x();
    } else {
        pos = globalPos.y();
        if (of == leftMng)
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

    QCursor cursor(Icon(icon).pixmap(22));

    if (cursorIsOnOtherSide(from, e->globalPosition().toPoint())) {
        if (!qApp->overrideCursor())
            qApp->setOverrideCursor(cursor);
    } else
        qApp->restoreOverrideCursor();
}

void KrusaderView::draggingTabFinished(PanelManager *from, QMouseEvent *e)
{
    qApp->restoreOverrideCursor();

    if (cursorIsOnOtherSide(from, e->globalPosition().toPoint()))
        from->moveTabToOtherSide();
}
