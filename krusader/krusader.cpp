/***************************************************************************
                          krusader.cpp
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

#include "krusader.h"

// QtCore
#include <QDir>
#include <QDateTime>
#include <QStringList>
#include <QStandardPaths>
// QtGui
#include <QMoveEvent>
#include <QResizeEvent>
// QtWidgets
#include <QApplication>
#include <QMenuBar>
#include <QDesktopWidget>
// QtDBus
#include <QDBusInterface>

#include <KCoreAddons/KRandom>
#include <KConfigCore/KSharedConfig>
#include <KConfigGui/KWindowConfig>
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>
#include <KXmlGui/KActionCollection>
#include <KXmlGui/KXMLGUIFactory>
#include <KXmlGui/KToolBar>
#include <KWidgetsAddons/KAcceleratorManager>
#include <KWidgetsAddons/KCursor>
#include <KWidgetsAddons/KMessageBox>
#include <KWidgetsAddons/KToggleAction>
#include <KWidgetsAddons/KToolBarPopupAction>
#include <KWindowSystem/KStartupInfo>
#include <KWindowSystem/KWindowSystem>

#include "defaults.h"
#include "kicons.h"
#include "kractions.h"
#include "krglobal.h"
#include "krservices.h"
#include "krslots.h"
#include "krtrashhandler.h"
#include "krusaderversion.h"
#include "krusaderview.h"
#include "panelmanager.h"
#include "tabactions.h"

#include "BookMan/krbookmarkhandler.h"
#include "Dialogs/checksumdlg.h"
#include "Dialogs/krpleasewait.h"
#include "Dialogs/popularurls.h"
#include "FileSystem/fileitem.h"
#include "FileSystem/krpermhandler.h"
#include "GUI/kcmdline.h"
#include "GUI/kfnkeys.h"
#include "GUI/krremoteencodingmenu.h"
#include "GUI/krusaderstatus.h"
#include "GUI/terminaldock.h"
#include "JobMan/jobman.h"
#include "KViewer/krviewer.h"
#include "Konfigurator/kgprotocols.h"
#include "MountMan/kmountman.h"
#include "Panel/PanelView/krview.h"
#include "Panel/PanelView/krviewfactory.h"
#include "Panel/krcolorcache.h"
#include "Panel/krpanel.h"
#include "Panel/listpanelactions.h"
#include "Panel/viewactions.h"
#include "UserAction/expander.h"
// This makes gcc-4.1 happy. Warning about possible problem with KrAction's dtor not called
#include "UserAction/kraction.h"
#include "UserAction/useraction.h"

#ifdef __KJSEMBED__
#include "KrJS/krjs.h"
#endif


// define the static members
Krusader *Krusader::App = 0;
QString   Krusader::AppName;
// KrBookmarkHandler *Krusader::bookman = 0;
//QTextOStream *Krusader::_krOut = QTextOStream(::stdout);

#ifdef __KJSEMBED__
KrJS *Krusader::js = 0;
QAction *Krusader::actShowJSConsole = 0;
#endif

// construct the views, statusbar and menu bars and prepare Krusader to start
Krusader::Krusader(const QCommandLineParser &parser) : KParts::MainWindow(0,
                Qt::Window | Qt::WindowTitleHint | Qt::WindowContextHelpButtonHint),
        _listPanelActions(0), isStarting(true), isExiting(false), _quit(false)
{
    // create the "krusader"
    App = this;
    krMainWindow = this;
    SLOTS = new KRslots(this);
    setXMLFile("krusaderui.rc");   // kpart-related xml file

    plzWait = new KRPleaseWaitHandler(this);

    const bool runKonfig = versionControl();

    QString message;
    switch (krConfig->accessMode()) {
    case KConfigBase::NoAccess :
        message = "Krusader's configuration file can't be found. Default values will be used.";
        break;
    case KConfigBase::ReadOnly :
        message = "Krusader's configuration file is in READ ONLY mode (why is that!?) Changed values will not be saved";
        break;
    case KConfigBase::ReadWrite :
        message = "";
        break;
    }
    if (!message.isEmpty()) {
        KMessageBox::error(krApp, message);
    }

    // create an icon loader
    krLoader = KIconLoader::global();
//   iconLoader->addExtraDesktopThemes();

    // create MountMan
    KrGlobal::mountMan = new KMountMan(this);
    connect(KrGlobal::mountMan, SIGNAL(refreshPanel(QUrl)), SLOTS, SLOT(refresh(QUrl)));

    // create bookman
    krBookMan = new KrBookmarkHandler(this);

    // create job manager
    krJobMan = new JobMan(this);

    _popularUrls = new PopularUrls(this);

    // create the main view
    MAIN_VIEW = new KrusaderView(this);

    // setup all the krusader's actions
    setupActions();

    // init the permmision handler class
    KRpermHandler::init();

    // init the protocol handler
    KgProtocols::init();

    const KConfigGroup lookFeelGroup(krConfig, "Look&Feel");
    FileItem::loadUserDefinedFolderIcons(lookFeelGroup.readEntry("Load User Defined Folder Icons",
                                                                 _UserDefinedFolderIcons));

    const KConfigGroup startupGroup(krConfig, "Startup");
    QString startProfile = startupGroup.readEntry("Starter Profile Name", QString());

    QList<QUrl> leftTabs;
    QList<QUrl> rightTabs;

    // get command-line arguments
    if (parser.isSet("left")) {
        leftTabs = KrServices::toUrlList(parser.value("left").split(','));
        startProfile.clear();
    }
    if (parser.isSet("right")) {
        rightTabs = KrServices::toUrlList(parser.value("right").split(','));
        startProfile.clear();
    }
    if (parser.isSet("profile"))
        startProfile = parser.value("profile");

    if (!startProfile.isEmpty()) {
        leftTabs.clear();
        rightTabs.clear();
    }
    // starting the panels
    MAIN_VIEW->start(startupGroup, startProfile.isEmpty(), leftTabs, rightTabs);

    // create a status bar
    KrusaderStatus *status = new KrusaderStatus(this);
    setStatusBar(status);
    status->setWhatsThis(i18n("Statusbar will show basic information "
                              "about file below mouse pointer."));

    // create tray icon (if needed)
    const bool startToTray = startupGroup.readEntry("Start To Tray", _StartToTray);
    setTray(startToTray);

    setCentralWidget(MAIN_VIEW);

    // manage our keyboard short-cuts
    //KAcceleratorManager::manage(this,true);

    setCursor(Qt::ArrowCursor);

    if (! startProfile.isEmpty())
        MAIN_VIEW->profiles(startProfile);

    // restore gui settings
    {
        // now, check if we need to create a konsole_part
        // call the XML GUI function to draw the UI
        createGUI(MAIN_VIEW->terminalDock()->part());

        // this needs to be called AFTER createGUI() !!!
        updateUserActions();
        _listPanelActions->guiUpdated();

        // not using this. See savePosition()
        //applyMainWindowSettings();

        const KConfigGroup cfgToolbar(krConfig, "Main Toolbar");
        toolBar()->applySettings(cfgToolbar);

        const KConfigGroup cfgJobBar(krConfig, "Job Toolbar");
        toolBar("jobToolBar")->applySettings(cfgJobBar);

        const KConfigGroup cfgActionsBar(krConfig, "Actions Toolbar");
        toolBar("actionsToolBar")->applySettings(cfgActionsBar);

        // restore toolbars position and visibility
        restoreState(startupGroup.readEntry("State", QByteArray()));

        statusBar()->setVisible(startupGroup.readEntry("Show status bar", _ShowStatusBar));

        MAIN_VIEW->updateGUI(startupGroup);

        // popular urls
        _popularUrls->load();
    }

    if (runKonfig)
        SLOTS->runKonfigurator(true);

    KConfigGroup viewerModuleGrp(krConfig, "ViewerModule");
    if (viewerModuleGrp.readEntry("FirstRun", true)) {
        KrViewer::configureDeps();
        viewerModuleGrp.writeEntry("FirstRun", false);
    }

    if (!runKonfig) {
        KConfigGroup cfg(krConfig, "Private");
        move(cfg.readEntry("Start Position", _StartPosition));
        resize(cfg.readEntry("Start Size", _StartSize));
    }

    // view initialized; show window or only tray
    if (!startToTray) {
        show();
    }

    KrTrashHandler::startWatcher();
    isStarting = false;

    //HACK - used by [ListerTextArea|KrSearchDialog|LocateDlg]:keyPressEvent()
    KrGlobal::copyShortcut = _listPanelActions->actCopy->shortcut();

    //HACK: make sure the active view becomes focused
    // for some reason sometimes the active view cannot be focused immediately at this point,
    // so queue it for the main loop
    QTimer::singleShot(0, ACTIVE_PANEL->view->widget(), SLOT(setFocus()));

    _openUrlTimer.setSingleShot(true);
    connect(&_openUrlTimer, SIGNAL(timeout()), SLOT(doOpenUrl()));

    KStartupInfo *startupInfo = new KStartupInfo(0, this);
    connect(startupInfo, &KStartupInfo::gotNewStartup,
            this, &Krusader::slotGotNewStartup);
    connect(startupInfo, &KStartupInfo::gotRemoveStartup,
            this, &Krusader::slotGotRemoveStartup);
}

Krusader::~Krusader()
{
    KrTrashHandler::stopWatcher();
    if (!isExiting) // save the settings if it was not saved (SIGTERM received)
        saveSettings();

    delete MAIN_VIEW;
    MAIN_VIEW = 0;
    App = 0;
}

void Krusader::setTray(bool forceCreation)
{
    const bool trayIsNeeded = forceCreation || KConfigGroup(krConfig, "Look&Feel")
                                               .readEntry("Minimize To Tray", _ShowTrayIcon);
    if (!sysTray && trayIsNeeded) {
        sysTray = new KStatusNotifierItem(this);
        sysTray->setIconByName(privIcon());
        // we have our own "quit" method, re-connect
        QAction *quitAction = sysTray->action(QStringLiteral("quit"));
        if (quitAction) {
            disconnect(quitAction, &QAction::triggered, nullptr, nullptr);
            connect(quitAction, &QAction::triggered, this, &Krusader::quit);
        }
    } else if (sysTray && !trayIsNeeded) {
        // user does not want tray anymore :(
        sysTray->deleteLater();
    }
}

bool Krusader::versionControl()
{
    // create config file
    krConfig = KSharedConfig::openConfig().data();
    KConfigGroup nogroup(krConfig, QString());
    const bool firstRun = nogroup.readEntry("First Time", true);
    KrGlobal::sCurrentConfigVersion = nogroup.readEntry("Config Version", -1);

    // first installation of krusader
    if (firstRun) {
        KMessageBox::information(
            krApp, i18n("<qt><b>Welcome to Krusader.</b><p>As this is your first run, your machine "
                        "will now be checked for external applications. Then the Konfigurator will "
                        "be launched where you can customize Krusader to your needs.</p></qt>"));
    }
    nogroup.writeEntry("Version", VERSION);
    nogroup.writeEntry("First Time", false);
    krConfig->sync();

    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
                  QStringLiteral("/krusader/"));

    return firstRun;
}

void Krusader::statusBarUpdate(const QString& mess)
{
    // change the message on the statusbar for 5 seconds
    if (statusBar()->isVisible())
        statusBar()->showMessage(mess, 5000);
}

bool Krusader::event(QEvent *e) {
    if(e->type() == QEvent::ApplicationPaletteChange) {
        KrColorCache::getColorCache().refreshColors();
    }
    return KParts::MainWindow::event(e);
}

// <patch> Moving from Pixmap actions to generic filenames - thanks to Carsten Pfeiffer
void Krusader::setupActions() {
    QAction *bringToTopAct = new QAction(i18n("Bring Main Window to Top"), this);
    actionCollection()->addAction("bring_main_window_to_top", bringToTopAct);
    connect(bringToTopAct, SIGNAL(triggered()), SLOT(moveToTop()));

    KrActions::setupActions(this);
    _krActions = new KrActions(this);
    _viewActions = new ViewActions(this, this);
    _listPanelActions = new ListPanelActions(this, this);
    _tabActions = new TabActions(this, this);
}

///////////////////////////////////////////////////////////////////////////
//////////////////// implementation of slots //////////////////////////////
///////////////////////////////////////////////////////////////////////////

void Krusader::savePosition() {
    KConfigGroup cfg(krConfig, "Private");
    cfg.writeEntry("Start Position", pos());
    cfg.writeEntry("Start Size", size());

    cfg = krConfig->group("Startup");
    MAIN_VIEW->saveSettings(cfg);

    // NOTE: this would save current window state/size, statusbar and settings for each toolbar.
    // We are not using this and saving everything manually because
    // - it does not save window position
    // - window size save/restore does sometimes not work (multi-monitor setup)
    // - saving the statusbar visibility should be independent from window position and restoring it
    //   does not work properly.
    //KConfigGroup cfg = KConfigGroup(&cfg, "MainWindowSettings");
    //saveMainWindowSettings(cfg);
    //statusBar()->setVisible(cfg.readEntry("StatusBar", "Enabled") != "Disabled");

    krConfig->sync();
}

void Krusader::saveSettings() {
    // workaround: revert terminal fullscreen mode before saving widget and toolbar visibility
    if (MAIN_VIEW->isTerminalEmulatorFullscreen()) {
        MAIN_VIEW->setTerminalEmulator(false, true);
    }

    KConfigGroup noGroup(krConfig, QString());
    noGroup.writeEntry("Config Version", KrGlobal::sConfigVersion);

    // save toolbar settings
    KConfigGroup cfg(krConfig, "Main Toolbar");
    toolBar()->saveSettings(cfg);

    cfg = krConfig->group("Job Toolbar");
    toolBar("jobToolBar")->saveSettings(cfg);

    cfg = krConfig->group("Actions Toolbar");
    toolBar("actionsToolBar")->saveSettings(cfg);

    cfg = krConfig->group("Startup");
    // save toolbar visibility and position
    cfg.writeEntry("State", saveState());
    cfg.writeEntry("Show status bar", statusBar()->isVisible());

    // save panel and window settings
    if (cfg.readEntry("Remember Position", _RememberPos))
        savePosition();

    // save the gui components visibility
    if (cfg.readEntry("UI Save Settings", _UiSave)) {
        cfg.writeEntry("Show FN Keys", KrActions::actToggleFnkeys->isChecked());
        cfg.writeEntry("Show Cmd Line", KrActions::actToggleCmdline->isChecked());
        cfg.writeEntry("Show Terminal Emulator", KrActions::actToggleTerminal->isChecked());
    }

    // save popular links
    _popularUrls->save();

    krConfig->sync();
}

void Krusader::closeEvent(QCloseEvent *event)
{
    if (!sysTray || _quit) {
        _quit = false; // in case quit will be aborted
        KParts::MainWindow::closeEvent(event); // (may) quit, continues with queryClose()...
    } else {
        // close window to tray
        event->ignore();
        hide();
    }
}

void Krusader::showEvent(QShowEvent *event)
{
    const KConfigGroup lookFeelGroup(krConfig, "Look&Feel");
    if (sysTray && !lookFeelGroup.readEntry("Minimize To Tray", _ShowTrayIcon)) {
        // restoring from "start to tray", tray icon is not needed anymore
        sysTray->deleteLater();
    }

    KParts::MainWindow::showEvent(event);
}

bool Krusader::queryClose() {
    if (isStarting || isExiting)
        return false;

    if (qApp->isSavingSession()) { // KDE is logging out, accept the close
        acceptClose();
        return true;
    }

    const KConfigGroup cfg = krConfig->group("Look&Feel");
    const bool confirmExit = cfg.readEntry("Warn On Exit", _WarnOnExit);

    // ask user and wait until all KIO::job operations are terminated. Krusader won't exit before
    // that anyway
    if (!krJobMan->waitForJobs(confirmExit))
        return false;

    /* First try to close the child windows, because it's the safer
       way to avoid crashes, then close the main window.
       If closing a child is not successful, then we cannot let the
       main window close. */

    for (;;) {
        QWidgetList list = QApplication::topLevelWidgets();
        QWidget *activeModal = QApplication::activeModalWidget();
        QWidget *w = list.at(0);

        if (activeModal &&
                activeModal != this &&
                activeModal != menuBar() &&
                list.contains(activeModal) &&
                !activeModal->isHidden()) {
            w = activeModal;
        } else {
            int i = 1;
            for (; i < list.count(); ++i) {
                w = list.at(i);
                if (!(w && (w == this || w->isHidden() || w == menuBar())))
                    break;
            }

            if (i == list.count())
                w = 0;
        }

        if (!w) break;

        if (!w->close()) {
            if (w->inherits("QDialog")) {
                fprintf(stderr, "Failed to close: %s\n", w->metaObject()->className());
            }
            return false;
        }
    }

    acceptClose();
    return true;
}

void Krusader::acceptClose() {
    saveSettings();

    emit shutdown();

    // Removes the DBUS registration of the application. Single instance mode requires unique appid.
    // As Krusader is exiting, we release that unique appid, so new Krusader instances
    // can be started.

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.unregisterObject("/Instances/" + Krusader::AppName);

    isExiting = true;
}

// the please wait dialog functions
void Krusader::startWaiting(QString msg, int count , bool cancel) {
    plzWait->startWaiting(msg , count, cancel);
}

bool Krusader::wasWaitingCancelled() const {
    return plzWait->wasCancelled();
}

void Krusader::stopWait() {
    plzWait->stopWait();
}

void Krusader::updateUserActions() {
    KActionMenu *userActionMenu = (KActionMenu *) KrActions::actUserMenu;
    if (userActionMenu) {
        userActionMenu->menu()->clear();

        userActionMenu->addAction(KrActions::actManageUseractions);
        userActionMenu->addSeparator();
        krUserAction->populateMenu(userActionMenu, NULL);
    }
}

const char* Krusader::privIcon() {
    if (geteuid())
        return "krusader_user";
    else
        return "krusader_root";
}

void Krusader::quit()
{
    _quit = true; // remember that we want to quit and not close to tray
    close(); // continues with closeEvent()...
}

void Krusader::moveToTop() {
    if (isHidden())
        show();

    KWindowSystem::forceActiveWindow(winId());
}

bool Krusader::isRunning() {
    moveToTop(); //FIXME - doesn't belong here
    return true;
}

bool Krusader::isLeftActive()  {
    return MAIN_VIEW->isLeftActive();
}

bool Krusader::openUrl(QString url)
{
    _urlToOpen = url;
    _openUrlTimer.start(0);
    return true;
}

void Krusader::doOpenUrl()
{
    QUrl url = QUrl::fromUserInput(_urlToOpen, QDir::currentPath(), QUrl::AssumeLocalFile);
    _urlToOpen.clear();
    int tab = ACTIVE_MNG->findTab(url);
    if(tab >= 0)
        ACTIVE_MNG->setActiveTab(tab);
    else if((tab = OTHER_MNG->findTab(url)) >= 0) {
        OTHER_MNG->setActiveTab(tab);
        OTHER_MNG->currentPanel()->view->widget()->setFocus();
    } else
        ACTIVE_MNG->slotNewTab(url);
}

void Krusader::slotGotNewStartup(const KStartupInfoId &id, const KStartupInfoData &data)
{
    Q_UNUSED(id)
    Q_UNUSED(data)
    // This is here to show busy mouse cursor when _other_ applications are launched, not for krusader itself.
    qApp->setOverrideCursor(Qt::BusyCursor);
}

void Krusader::slotGotRemoveStartup(const KStartupInfoId &id, const KStartupInfoData &data)
{
    Q_UNUSED(id)
    Q_UNUSED(data)
    qApp->restoreOverrideCursor();
}

KrView *Krusader::activeView()
{
    return ACTIVE_PANEL->view;
}

AbstractPanelManager *Krusader::activeManager()
{
    return MAIN_VIEW->activeManager();
}

AbstractPanelManager *Krusader::leftManager()
{
    return MAIN_VIEW->leftManager();
}

AbstractPanelManager *Krusader::rightManager()
{
    return MAIN_VIEW->rightManager();
}

