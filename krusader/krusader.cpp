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

#include "krusaderversion.h"
#include "kicons.h"
#include "krusaderview.h"
#include "defaults.h"
#include "krslots.h"
#include "krservices.h"
// This makes gcc-4.1 happy. Warning about possible problem with KrAction's dtor not called
#include "krtrashhandler.h"
#include "tabactions.h"
#include "krglobal.h"
#include "kractions.h"
#include "panelmanager.h"
#include "Panel/krcolorcache.h"
#include "Panel/viewactions.h"
#include "Panel/listpanelactions.h"
#include "Panel/krpanel.h"
#include "Panel/krview.h"
#include "Panel/krviewfactory.h"
#include "UserAction/kraction.h"
#include "UserAction/expander.h"
#include "UserAction/useraction.h"
#include "UserMenu/usermenu.h"
#include "Dialogs/popularurls.h"
#include "Dialogs/checksumdlg.h"
#include "Dialogs/krpleasewait.h"
#include "GUI/krremoteencodingmenu.h"
#include "GUI/kfnkeys.h"
#include "GUI/kcmdline.h"
#include "GUI/terminaldock.h"
#include "GUI/krusaderstatus.h"
#include "VFS/vfile.h"
#include "VFS/krpermhandler.h"
#include "MountMan/kmountman.h"
#include "Queue/queue_mgr.h"
#include "Konfigurator/kgprotocols.h"
#include "BookMan/krbookmarkhandler.h"
#include "KViewer/krviewer.h"


#ifdef __KJSEMBED__
#include "KrJS/krjs.h"
#endif


// define the static members
Krusader *Krusader::App = 0;
QString   Krusader::AppName;
UserMenu *Krusader::userMenu = 0;
// KrBookmarkHandler *Krusader::bookman = 0;
//QTextOStream *Krusader::_krOut = QTextOStream(::stdout);

#ifdef __KJSEMBED__
KrJS *Krusader::js = 0;
QAction *Krusader::actShowJSConsole = 0;
#endif

// construct the views, statusbar and menu bars and prepare Krusader to start
Krusader::Krusader(const QCommandLineParser &parser) : KParts::MainWindow(0,
                Qt::Window | Qt::WindowTitleHint | Qt::WindowContextHelpButtonHint),
        status(0), _listPanelActions(0), isStarting(true), isExiting(false)
{
    // create the "krusader"
    App = this;
    krMainWindow = this;
    SLOTS = new KRslots(this);
    setXMLFile("krusaderui.rc");   // kpart-related xml file

    plzWait = new KRPleaseWaitHandler(this);

    bool runKonfig = versionControl();

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
    connect(KrGlobal::mountMan, SIGNAL(refreshPanel(const QUrl &)), SLOTS, SLOT(refresh(const QUrl &)));

    // create bookman
    krBookMan = new KrBookmarkHandler(this);

    _popularUrls = new PopularUrls(this);

    queueManager = new QueueManager(this);

    // create the main view
    MAIN_VIEW = new KrusaderView(this);

    // setup all the krusader's actions
    setupActions();

    // init the permmision handler class
    KRpermHandler::init();

    // init the protocol handler
    KgProtocols::init();

    // init the checksum tools
    initChecksumModule();

    KConfigGroup gl(krConfig, "Look&Feel");
    KConfigGroup glgen(krConfig, "General");
    vfile::vfile_loadUserDefinedFolderIcons(gl.readEntry("Load User Defined Folder Icons", _UserDefinedFolderIcons));
    vfile::vfile_enableMimeTypeMagic(glgen.readEntry("Mimetype Magic", _MimetypeMagic));

    KConfigGroup gs(krConfig, "Startup");
    QString     startProfile = gs.readEntry("Starter Profile Name", QString());

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
    MAIN_VIEW->start(gs, startProfile.isEmpty(), leftTabs, rightTabs);

    // create the user menu
    userMenu = new UserMenu(this);
    userMenu->hide();

    // create a status bar
    status = new KrusaderStatus(this);
    setStatusBar(status);
    status->setWhatsThis(i18n("Statusbar will show basic information "
                              "about file below mouse pointer."));

    // create tray icon (even if not shown)
    sysTray = new QSystemTrayIcon(this);
    sysTray->setIcon(krLoader->loadIcon(privIcon(), KIconLoader::Panel, 22));
    QMenu *trayMenu = new QMenu(this);
    trayMenu->addSection(QGuiApplication::applicationDisplayName()); // show "title"
    QAction *restoreAction = new QAction(i18n("Restore"), this);
    connect(restoreAction, SIGNAL(triggered()), SLOT(showFromTray()));
    trayMenu->addAction(restoreAction);
    trayMenu->addSeparator();
    trayMenu->addAction(actionCollection()->action(KStandardAction::name(KStandardAction::Quit)));
    sysTray->setContextMenu(trayMenu);
    // tray is only visible if main window is hidden, so action is always "show"
    connect(sysTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT(showFromTray()));

    setCentralWidget(MAIN_VIEW);

    // manage our keyboard short-cuts
    //KAcceleratorManager::manage(this,true);

    setCursor(Qt::ArrowCursor);

    if (! startProfile.isEmpty())
        MAIN_VIEW->profiles(startProfile);
    // let the good times rool :)
    updateGUI(true);

    if (runKonfig)
        SLOTS->runKonfigurator(true);

    KConfigGroup viewerModuleGrp(krConfig, "ViewerModule");
    if (viewerModuleGrp.readEntry("FirstRun", true)) {
        KrViewer::configureDeps();
        viewerModuleGrp.writeEntry("FirstRun", false);
    }

    if (!runKonfig) {
        KConfigGroup cfg(krConfig, "Private");
        if (cfg.readEntry("Maximized", false))
            KWindowConfig::restoreWindowSize(windowHandle(), cfg);
        else {
            move(oldPos = cfg.readEntry("Start Position", _StartPosition));
            resize(oldSize = cfg.readEntry("Start Size", _StartSize));
        }
    }

    // view initialized; show window or tray
    if (gs.readEntry("Start To Tray", _StartToTray)) {
        sysTray->show();
    } else {
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
    if (!isExiting)    // save the settings if it was not saved (SIGTERM)
        saveSettings();

    delete MAIN_VIEW;
    MAIN_VIEW = 0;
    App = 0;
}

bool Krusader::versionControl()
{
#define FIRST_RUN "First Time"
    bool retval = false;
    // create config file
    krConfig = KSharedConfig::openConfig().data();
    KConfigGroup nogroup(krConfig, QString());

    bool firstRun = nogroup.readEntry(FIRST_RUN, true);

#if 0
    QString oldVerText = nogroup.readEntry("Version", "10.0");
    oldVerText.truncate(oldVerText.lastIndexOf("."));     // remove the third dot
    float oldVer = oldVerText.toFloat();
    // older icompatible version
    if (oldVer <= 0.9) {
        KMessageBox::information(krApp, i18n("A configuration of 1.51 or older was detected. Krusader has to reset your configuration to default values.\nNote: your bookmarks and keybindings will remain intact.\nKrusader will now run Konfigurator."));
        /*if ( !QDir::home().remove( ".kde/share/config/krusaderrc" ) ) {
           KMessageBox::error( krApp, i18n( "Unable to remove your krusaderrc file. Please remove it manually and rerun Krusader." ) );
           exit( 1 );
        }*/
        retval = true;
        krConfig->reparseConfiguration();
    }
#endif

    // first installation of krusader
    if (firstRun) {
        KMessageBox::information(krApp, i18n("<qt><b>Welcome to Krusader.</b><p>As this is your first run, your machine will now be checked for external applications. Then the Konfigurator will be launched where you can customize Krusader to your needs.</p></qt>"));
        retval = true;
    }
    nogroup.writeEntry("Version", VERSION);
    nogroup.writeEntry(FIRST_RUN, false);
    krConfig->sync();

    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/krusader/"));

    return retval;
}

void Krusader::statusBarUpdate(QString& mess)
{
    // change the message on the statusbar for 2 seconds
    if (status) // ugly!!!! But as statusBar() creates a status bar if there is no, I have to ask status to prevent
        // the creation of the KDE default status bar instead of KrusaderStatus.
        statusBar() ->showMessage(mess, 5000);
}

void Krusader::showFromTray() {
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    show();
    sysTray->hide();
}

void Krusader::changeEvent(QEvent *event) {
    QMainWindow::changeEvent(event);
    if (isExiting)
        return;

    // toggle tray when minimizing (if enabled)
    if(event->type() == QEvent::WindowStateChange) {
        if(isMinimized()) {
            KConfigGroup group(krConfig, "Look&Feel");
            if (group.readEntry("Minimize To Tray", _MinimizeToTray)) {
                sysTray->show();
                hide();
            }
        } else if(isVisible()) {
            sysTray->hide();
        }
    }
}

void Krusader::moveEvent(QMoveEvent *e) {
    oldPos = e->oldPos();
    KParts::MainWindow::moveEvent(e);
}

void Krusader::resizeEvent(QResizeEvent *e) {
    oldSize = e->oldSize();
    KParts::MainWindow::resizeEvent(e);
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
    cfg.writeEntry("Maximized", isMaximized());
    if (isMaximized()) {
        KConfigGroup cg = krConfig->group("Private");
        KWindowConfig::saveWindowSize(windowHandle(), cg, KConfigGroup::Normal);
    }
    else {
        cfg.writeEntry("Start Position", isMaximized() ? oldPos : pos());
        cfg.writeEntry("Start Size", isMaximized() ? oldSize : size());
    }

    cfg = krConfig->group("Startup");
    MAIN_VIEW->saveSettings(cfg);

    saveMainWindowSettings(cfg);

    krConfig->sync();
}

void Krusader::saveSettings() {
    KConfigGroup cfg(krConfig, "Main Toolbar");
    toolBar()->saveSettings(cfg);

    cfg = krConfig->group("Actions Toolbar");
    toolBar("actionsToolBar")->saveSettings(cfg);

    cfg = krConfig->group("Startup");

    bool rememberpos = cfg.readEntry("Remember Position", _RememberPos);
    bool uisavesettings = cfg.readEntry("UI Save Settings", _UiSave);

    // save size and position
    if (rememberpos || uisavesettings)
        savePosition();

    // save the gui
    if (uisavesettings) {
        cfg = krConfig->group("Startup");
        cfg.writeEntry("Show status bar", KrActions::actShowStatusBar->isChecked());
        cfg.writeEntry("Show tool bar", !toolBar()->isHidden());
        cfg.writeEntry("Show actions tool bar", !toolBar("actionsToolBar")->isHidden());
        cfg.writeEntry("Show FN Keys", KrActions::actToggleFnkeys->isChecked());
        cfg.writeEntry("Show Cmd Line", KrActions::actToggleCmdline->isChecked());
        cfg.writeEntry("Show Terminal Emulator", KrActions::actToggleTerminal->isChecked());
        cfg.writeEntry("Start To Tray", isHidden());
    }

    // save popular links
    _popularUrls->save();

    krConfig->sync();
}

bool Krusader::queryClose() {
    if (isStarting || isExiting)
        return false;

    if (qApp->isSavingSession()) { // KDE is logging out, accept the close
        acceptClose();
        return true;
    }

    KConfigGroup cfg = krConfig->group("Look&Feel");
    if (cfg.readEntry("Warn On Exit", _WarnOnExit)) {
        // If user decides to cancel...
        if (KMessageBox::warningYesNo(this, i18n("Are you sure you want to quit?"))
            != KMessageBox::Yes) {
            // ... stop quit
            return false;
        }
    }

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
    krConfig->sync();

    emit shutdown();

    // Removes the DBUS registration of the application. Single instance mode requires unique appid.
    // As Krusader is exiting, we release that unique appid, so new Krusader instances
    // can be started.

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.unregisterObject("/Instances/" + Krusader::AppName);

    isExiting = true; // this will also kill the pending jobs

    return;
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

void Krusader::updateGUI(bool enforce) {
    // now, check if we need to create a konsole_part
    // call the XML GUI function to draw the UI
    createGUI(MAIN_VIEW->terminalDock()->part());

    // this needs to be called AFTER createGUI() !!!
    updateUserActions();
    _listPanelActions->guiUpdated();

    KConfigGroup cfg_toolbar(krConfig, "Main Toolbar");
    toolBar()->applySettings(cfg_toolbar);

    KConfigGroup cfg_act(krConfig->group("Actions Toolbar"));
    toolBar("actionsToolBar") ->applySettings(cfg_act);

    KConfigGroup cfg(krConfig, "Startup");
    if (enforce) {
        applyMainWindowSettings(KConfigGroup(&cfg, "MainWindowSettings"));
        // now, hide what need to be hidden
        toolBar()->setVisible(cfg.readEntry("Show tool bar", _ShowToolBar));
        toolBar("actionsToolBar")->setVisible(cfg.readEntry("Show actions tool bar", _ShowActionsToolBar));
        if (!cfg.readEntry("Show status bar", _ShowStatusBar)) {
            statusBar() ->hide();
            KrActions::actShowStatusBar->setChecked(false);
        } else {
            statusBar() ->show();
            KrActions::actShowStatusBar->setChecked(true);
        }

        MAIN_VIEW->updateGUI(cfg);
    }
    // popular urls
    _popularUrls->load();
}

QString Krusader::getTempDir() {
    // try to make krusader temp dir
    KConfigGroup group(krConfig, "General");
    QString tmpDir = group.readEntry("Temp Directory", _TempDirectory);

    if (! QDir(tmpDir).exists()) {
        for (int i = 1 ; i != -1 ; i = tmpDir.indexOf('/', i + 1))
            QDir().mkdir(tmpDir.left(i));
        QDir().mkdir(tmpDir);
        chmod(tmpDir.toLocal8Bit(), 0777);
    }

    // add a secure sub dir under the user UID
    QString uid;
    uid.sprintf("%d", getuid());
    QDir(tmpDir).mkdir(uid);
    tmpDir = tmpDir + '/' + uid + '/';
    chmod(tmpDir.toLocal8Bit(), S_IRUSR | S_IWUSR | S_IXUSR);
    // add a random sub dir to use
    while (QDir().exists(tmpDir))
        tmpDir = tmpDir + KRandom::randomString(8);
    QDir().mkdir(tmpDir);

    if (!QDir(tmpDir).isReadable()) {
        KMessageBox::error(krApp, i18n("Could not create a temporary folder. Handling of Archives will not be possible until this is fixed."));
        return QString();
    }
    return tmpDir;
}

QString Krusader::getTempFile() {
    // try to make krusader temp dir
    KConfigGroup group(krConfig, "General");
    QString tmpDir = group.readEntry("Temp Directory", _TempDirectory);

    if (! QDir(tmpDir).exists()) {
        for (int i = 1 ; i != -1 ; i = tmpDir.indexOf('/', i + 1))
            QDir().mkdir(tmpDir.left(i));
        QDir().mkdir(tmpDir);
        chmod(tmpDir.toLocal8Bit(), 0777);
    }

    // add a secure sub dir under the user UID
    QString uid;
    uid.sprintf("%d", getuid());
    QDir(tmpDir).mkdir(uid);
    tmpDir = tmpDir + '/' + uid + '/';
    chmod(tmpDir.toLocal8Bit(), S_IRUSR | S_IWUSR | S_IXUSR);

    while (QDir().exists(tmpDir))
        tmpDir = tmpDir + KRandom::randomString(8);
    return tmpDir;
}

const char* Krusader::privIcon() {
    if (geteuid())
        return "krusader_user";
    else
        return "krusader_root";
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

