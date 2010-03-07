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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <unistd.h>

#include <QtGui/QPixmap>
#include <QtCore/QStringList>
#include <QtCore/QDir>
#include <QtGui/QPrinter>
#include <qwidget.h>
#include <QtCore/QDateTime>
#include <QActionGroup>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QHideEvent>
#include <QDesktopWidget>
#include <QtDBus/QtDBus>

#include <krandom.h>
#include <kxmlguifactory.h>
#include <kactioncollection.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <ktoolbar.h>
#include <ktoggleaction.h>
#include <ktoolbarpopupaction.h>
#include <kcursor.h>
#include <ksystemtrayicon.h>
#include <kmenubar.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kacceleratormanager.h>
#include <kwindowsystem.h>
#include <kdeversion.h>

#include "krusaderversion.h"
#include "krglobal.h"
#include "kicons.h"
#include "VFS/krpermhandler.h"
#include "GUI/krusaderstatus.h"
#include "Dialogs/krpleasewait.h"
#include "krusaderview.h"
#include "Konfigurator/konfigurator.h"
#include "Konfigurator/kgprotocols.h"
#include "MountMan/kmountman.h"
#include "Panel/panelpopup.h"
#include "Queue/queue_mgr.h"
#include "defaults.h"
#include "resources.h"
#include "GUI/kfnkeys.h"
#include "GUI/kcmdline.h"
#include "GUI/terminaldock.h"
#include "krslots.h"
#include "krservices.h"
#include "UserAction/useraction.h"
#include "Panel/listpanel.h"
#include "Panel/krviewfactory.h"
// This makes gcc-4.1 happy. Warning about possible problem with KrAction's dtor not called
#include "UserAction/kraction.h"
#include "UserAction/expander.h"
#include "UserMenu/usermenu.h"
#include "BookMan/krbookmarkhandler.h"
#include "Dialogs/popularurls.h"
#include "GUI/krremoteencodingmenu.h"
#include "Dialogs/checksumdlg.h"
#include "VFS/vfile.h"
#include "krtrashhandler.h"

#ifdef __KJSEMBED__
#include "KrJS/krjs.h"
#endif

#include "krglobal.h"
#include "kractions.h"

// define the static members
Krusader *Krusader::App = 0;
QString   Krusader::AppName;
#if 0
KAction *Krusader::actProperties = 0;
KAction *Krusader::actPack = 0;
KAction *Krusader::actUnpack = 0;
KAction *Krusader::actTest = 0;
KAction *Krusader::actCopy = 0;
KAction *Krusader::actPaste = 0;
KAction *Krusader::actCompare = 0;
KAction *Krusader::actCalculate = 0;
KAction *Krusader::actCreateChecksum = 0;
KAction *Krusader::actMatchChecksum = 0;
KAction *Krusader::actSelect = 0;
KAction *Krusader::actSelectAll = 0;
KAction *Krusader::actUnselect = 0;
KAction *Krusader::actUnselectAll = 0;
KAction *Krusader::actInvert = 0;
KAction *Krusader::actCompDirs = 0;
KAction *Krusader::actSync = 0;
KAction *Krusader::actDiskUsage = 0;
KAction *Krusader::actQueueManager = 0;
KAction *Krusader::actHomeTerminal = 0;
KAction *Krusader::actFTPConnect = 0;
KAction *Krusader::actFTPNewConnect = 0;
KAction *Krusader::actFTPDisconnect = 0;
KAction *Krusader::actRemoteEncoding = 0;
KAction *Krusader::actProfiles = 0;
KAction *Krusader::actMultiRename = 0;
KAction *Krusader::actAllFilter = 0;
KAction *Krusader::actExecFilter = 0;
KAction *Krusader::actCustomFilter = 0;
KAction *Krusader::actMountMan = 0;
KAction *Krusader::actNewTool = 0;
KAction *Krusader::actKonfigurator = 0;
KAction *Krusader::actToolsSetup = 0;
KAction *Krusader::actSwapPanels = 0;
KAction *Krusader::actSwapSides = 0;
KAction *Krusader::actBack = 0;
KAction *Krusader::actRoot = 0;
KAction *Krusader::actFind = 0;
KAction *Krusader::actLocate = 0;
KAction *Krusader::actSwitchFullScreenTE = 0;
//KAction *Krusader::actAddBookmark = 0;
KAction *Krusader::actSavePosition = 0;
KAction *Krusader::actSelectColorMask = 0;
KAction *Krusader::actOpenLeftBm = 0;
KAction *Krusader::actOpenRightBm = 0;
KAction *Krusader::actDirUp = 0;
KAction *Krusader::actCmdlinePopup = 0;
KAction *Krusader::actNewTab = 0;
KAction *Krusader::actDupTab = 0;
KAction *Krusader::actLockTab = 0;
KAction *Krusader::actCloseTab = 0;
KAction *Krusader::actNextTab = 0;
KAction *Krusader::actPreviousTab = 0;
KAction *Krusader::actCloseInactiveTabs = 0;
KAction *Krusader::actCloseDuplicatedTabs = 0;
KAction *Krusader::actSplit = 0;
KAction *Krusader::actCombine = 0;
KAction *Krusader::actUserMenu = 0;
KAction *Krusader::actManageUseractions = 0;
KAction *Krusader::actSyncDirs = 0;
KAction *Krusader::actSyncBrowse = 0;
KAction *Krusader::actCancelRefresh = 0;
KAction *Krusader::actF2 = 0;
KAction *Krusader::actF3 = 0;
KAction *Krusader::actF4 = 0;
KAction *Krusader::actF5 = 0;
KAction *Krusader::actF6 = 0;
KAction *Krusader::actF7 = 0;
KAction *Krusader::actF8 = 0;
KAction *Krusader::actF9 = 0;
KAction *Krusader::actF10 = 0;
KAction *Krusader::actShiftF5 = 0;
KAction *Krusader::actShiftF6 = 0;
KAction *Krusader::actEmptyTrash = 0;
KAction *Krusader::actTrashBin = 0;
KAction *Krusader::actLocationBar = 0;
KAction *Krusader::actPopularUrls = 0;
KAction *Krusader::actJumpBack = 0;
KAction *Krusader::actSetJumpBack = 0;
KAction *Krusader::actView0 = 0;
KAction *Krusader::actView1 = 0;
KAction *Krusader::actView2 = 0;
KAction *Krusader::actView3 = 0;
KAction *Krusader::actView4 = 0;
KAction *Krusader::actView5 = 0;

KAction *Krusader::actZoomIn = 0;
KAction *Krusader::actZoomOut = 0;
KAction *Krusader::actDefaultZoom = 0;

KToggleAction *Krusader::actToggleTerminal = 0;
KAction  *Krusader::actVerticalMode = 0;
KAction  *Krusader::actSelectNewerAndSingle = 0;
KAction  *Krusader::actSelectSingle = 0;
KAction  *Krusader::actSelectNewer = 0;
KAction  *Krusader::actSelectDifferentAndSingle = 0;
KAction  *Krusader::actSelectDifferent = 0;
KAction  **Krusader::compareArray[] = {&actSelectNewerAndSingle, &actSelectNewer, &actSelectSingle,
                                       &actSelectDifferentAndSingle, &actSelectDifferent, 0
                                      };
KAction *Krusader::actExecStartAndForget = 0;
KAction *Krusader::actExecCollectSeparate = 0;
KAction *Krusader::actExecCollectTogether = 0;
KAction *Krusader::actExecTerminalExternal = 0;
KAction *Krusader::actExecTerminalEmbedded = 0;
KAction **Krusader::execTypeArray[] = {&actExecStartAndForget, &actExecCollectSeparate, &actExecCollectTogether,
                                       &actExecTerminalExternal, &actExecTerminalEmbedded, 0
                                      };
#endif
UserMenu *Krusader::userMenu = 0;
// KrBookmarkHandler *Krusader::bookman = 0;
//QTextOStream *Krusader::_krOut = QTextOStream(::stdout);


#ifdef __KJSEMBED__
KrJS *Krusader::js = 0;
KAction *Krusader::actShowJSConsole = 0;
#endif

// construct the views, statusbar and menu bars and prepare Krusader to start
Krusader::Krusader() : KParts::MainWindow(0,
                Qt::Window | Qt::WindowTitleHint | Qt::WindowContextHelpButtonHint),
        status(NULL), sysTray(0), isStarting(true), isExiting(false), directExit(false)
{

    setAttribute(Qt::WA_DeleteOnClose);
    // parse command line arguments
    KCmdLineArgs * args = KCmdLineArgs::parsedArgs();

    // create the "krusader"
    App = this;
    krMainWindow = this;
    SLOTS = new KRslots(this);
    setXMLFile("krusaderui.rc");   // kpart-related xml file

    plzWait = new KRPleaseWaitHandler();

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
    KrGlobal::mountMan = new KMountMan();

    // create bookman
    krBookMan = new KrBookmarkHandler();

    popularUrls = new PopularUrls(this);

    queueManager = new QueueManager();

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

    QStringList leftTabs;
    QStringList rightTabs;

    // get command-line arguments
    if (args->isSet("left")) {
        leftTabs = args->getOption("left").split(',');

        // make sure left or right are not relative paths
        for (int i = 0; i != leftTabs.count(); i++) {
            leftTabs[ i ] = leftTabs[ i ].trimmed();
            if (!leftTabs[ i ].startsWith('/') && leftTabs[ i ].indexOf(":/") < 0)
                leftTabs[ i ] = QDir::currentPath() + '/' + leftTabs[ i ];
        }
        startProfile.clear();
    }
    if (args->isSet("right")) {
        rightTabs = args->getOption("right").split(',');

        // make sure left or right are not relative paths
        for (int i = 0; i != rightTabs.count(); i++) {
            rightTabs[ i ] = rightTabs[ i ].trimmed();
            if (!rightTabs[ i ].startsWith('/') && rightTabs[ i ].indexOf(":/") < 0)
                rightTabs[ i ] = QDir::currentPath() + '/' + rightTabs[ i ];
        }
        startProfile.clear();
    }
    if (args->isSet("profile"))
        startProfile = args->getOption("profile");

    if (!startProfile.isEmpty()) {
        leftTabs.clear();
        rightTabs.clear();
    }
    // starting the panels
    MAIN_VIEW->start(gs, startProfile.isEmpty(), leftTabs, rightTabs);

    // create the user menu
    userMenu = new UserMenu(this);
    userMenu->hide();

    // setup keyboard accelerators
    setupAccels();

    // create a status bar
    status = new KrusaderStatus(this);
    setStatusBar(status);
    status->setWhatsThis(i18n("Statusbar will show basic information "
                              "about file below mouse pointer."));

    KGlobal::ref(); // FIX: krusader exits at closing the viewer when minimized to tray
    // This enables Krusader to show a tray icon
    sysTray = new KSystemTrayIcon(this);
    // Krusader::privIcon() returns either "krusader_blue" or "krusader_red" if the user got root-privileges
    sysTray->setIcon(krLoader->loadIcon(privIcon(), KIconLoader::Panel, 22));
    sysTray->hide();

    connect(sysTray, SIGNAL(quitSelected()), this, SLOT(setDirectExit()));

    setCentralWidget(MAIN_VIEW);
    bool startToTray = gs.readEntry("Start To Tray", _StartToTray);
    bool minimizeToTray = gl.readEntry("Minimize To Tray", _MinimizeToTray);
    bool singleInstanceMode = gl.readEntry("Single Instance Mode", _SingleInstanceMode);

    startToTray = startToTray && minimizeToTray;

    if (singleInstanceMode && minimizeToTray)
        sysTray->show();


    // manage our keyboard short-cuts
    //KAcceleratorManager::manage(this,true);

    setCursor(Qt::ArrowCursor);

    if (! startProfile.isEmpty())
        MAIN_VIEW->profiles(startProfile);
    // let the good times rool :)
    updateGUI(true);

    if (runKonfig)
        SLOTS->runKonfigurator(true);

    if (!runKonfig) {
        KConfigGroup cfg(krConfig, "Private");
        if (cfg.readEntry("Maximized", false))
            restoreWindowSize(cfg);
        else {
            move(oldPos = cfg.readEntry("Start Position", _StartPosition));
            resize(oldSize = cfg.readEntry("Start Size", _StartSize));
        }
    }

    if (startToTray) {
        sysTray->show();
        hide();
    } else
        show();


    KrTrashHandler::startWatcher();
    isStarting = false;

    //HACK: make sure the active view becomes focused
    // for some reason sometimes the active view cannot be focused immediately at this point,
    // so queue it for the main loop
    QTimer::singleShot(0, ACTIVE_PANEL->view->widget(), SLOT(setFocus()));
}

Krusader::~Krusader()
{
    KrTrashHandler::stopWatcher();
    delete queueManager;
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
    // TODO: according to docs, KGlobal::config() should return KConfig*, but in reality (in beta1), it returns KSharedPtr<KConfig> or something ?!
    krConfig = KGlobal::config().data();
    KConfigGroup nogroup(krConfig, QString());

    bool firstRun = nogroup.readEntry(FIRST_RUN, true);

#if 0
    QString oldVerText = nogroup.readEntry("Version", "10.0");
    oldVerText.truncate(oldVerText.lastIndexOf("."));     // remove the third dot
    float oldVer = oldVerText.toFloat();
    // older icompatible version
    if (oldVer <= 0.9) {
        KMessageBox::information(krApp, i18n("A configuration of 1.51 or older was detected. Krusader has to reset your configuration to default values.\nNote: Your bookmarks and keybindings will remain intact.\n Krusader will now run Konfigurator."));
        /*if ( !QDir::home().remove( ".kde/share/config/krusaderrc" ) ) {
           KMessageBox::error( krApp, i18n( "Unable to remove your krusaderrc file! Please remove it manually and rerun Krusader." ) );
           exit( 1 );
        }*/
        retval = true;
        krConfig->reparseConfiguration();
    }
#endif

    // first installation of krusader
    if (firstRun) {
        KMessageBox::information(krApp, i18n("<qt><b>Welcome to Krusader!</b><p>As this is your first run, your machine will now be checked for external applications. Then the Konfigurator will be launched where you can customize Krusader to your needs.</p></qt>"));
        retval = true;
    }
    nogroup.writeEntry("Version", VERSION);
    nogroup.writeEntry(FIRST_RUN, false);
    krConfig->sync();
    return retval;
}

void Krusader::statusBarUpdate(QString& mess)
{
    // change the message on the statusbar for 2 seconds
    if (status) // ugly!!!! But as statusBar() creates a status bar if there is no, I have to ask status to prevent
        // the creation of the KDE default status bar instead of KrusaderStatus.
        statusBar() ->showMessage(mess, 5000);
}

void Krusader::showEvent(QShowEvent *)
{
    if (isExiting)
        return;
    KConfigGroup group(krConfig, "Look&Feel");
    bool showTrayIcon = group.readEntry("Minimize To Tray", _MinimizeToTray);
    bool singleInstanceMode = group.readEntry("Single Instance Mode", _SingleInstanceMode);

    if (showTrayIcon && !singleInstanceMode && sysTray)
        sysTray->hide();
    show(); // needed to make sure krusader is removed from
    // the taskbar when minimizing (system tray issue)
}

void Krusader::hideEvent(QHideEvent *e)
{
    if (isExiting) {
        KParts::MainWindow::hideEvent(e);
        if (sysTray)
            sysTray->hide();
        return;
    }
    KConfigGroup group(krConfig, "Look&Feel");
    bool showTrayIcon = group.readEntry("Minimize To Tray", _MinimizeToTray);

    bool isModalTopWidget = false;

    QWidget *actWnd = qApp->activeWindow();
    if (actWnd)
        isModalTopWidget = actWnd->isModal();

#ifdef Q_WS_X11
    // KWindowSystem::windowInfo is only available for X11
    if (showTrayIcon  && !isModalTopWidget  && KWindowSystem::windowInfo(winId(), NET::WMDesktop).isOnCurrentDesktop()) {
#else
    if (showTrayIcon  && !isModalTopWidget) {
#endif
        if (sysTray)
            sysTray->show();
        hide(); // needed to make sure krusader is removed from
        // the taskbar when minimizing (system tray issue)
    } else KParts::MainWindow::hideEvent(e);
}

void Krusader::moveEvent(QMoveEvent *e) {
    oldPos = e->oldPos();
    KParts::MainWindow::moveEvent(e);
}

void Krusader::resizeEvent(QResizeEvent *e) {
    oldSize = e->oldSize();
    KParts::MainWindow::resizeEvent(e);
}

void Krusader::setupAccels() {
    KAction * tab = new KAction("Tab-Switch panel", this);
    tab->setShortcut(Qt::Key_Tab);
    connect(tab, SIGNAL(triggered(bool)), MAIN_VIEW, SLOT(panelSwitch()));
    actionCollection()->addAction("tab", tab);
}

// <patch> Moving from Pixmap actions to generic filenames - thanks to Carsten Pfeiffer
void Krusader::setupActions() {
#if 0
#define NEW_KACTION(VAR, TEXT, ICON_NAME, SHORTCUT, RECV_OBJ, SLOT_NAME, NAME) \
    if (ICON_NAME == 0) VAR = new KAction(TEXT, this); \
    else VAR = new KAction( KIcon(ICON_NAME), TEXT, this); \
    VAR->setShortcut(SHORTCUT); \
    connect(VAR, SIGNAL(triggered(bool)), RECV_OBJ, SLOT_NAME); \
    actionCollection()->addAction(NAME, VAR);

#define NEW_KTOGGLEACTION(VAR, TEXT, ICON_NAME, SHORTCUT, RECV_OBJ, SLOT_NAME, NAME) \
    if (ICON_NAME == 0) VAR = new KToggleAction(TEXT, this); \
    else VAR = new KToggleAction( KIcon(ICON_NAME), TEXT, this); \
    VAR->setShortcut(SHORTCUT); \
    connect(VAR, SIGNAL(triggered(bool)), RECV_OBJ, SLOT_NAME); \
    actionCollection()->addAction(NAME, VAR);

    // first come the TODO actions
    //actSync =       0;//new KAction(i18n("S&ynchronize Dirs"),                         0, this, 0, actionCollection(), "sync dirs");
    //actNewTool =    0;//new KAction(i18n("&Add a new tool"),                          0, this, 0, actionCollection(), "add tool");
    //actToolsSetup = 0;//new KAction(i18n("&Tools Menu Setup"),                        0, 0, this, 0, actionCollection(), "tools setup");
    //KStandardAction::print(SLOTS, 0,actionCollection(),"std_print");
    //KStandardAction::showMenubar( SLOTS, SLOT( showMenubar() ), actionCollection(), "std_menubar" );


    // second, the KDE standard action
//PORTME: second shortcut for up: see actDirUp
//   KStandardAction::up( SLOTS, SLOT( dirUp() ), actionCollection() )->setShortcut(Qt::Key_Backspace);

    /* Shortcut disabled because of the Terminal Emulator bug. */
    KConfigGroup group(krConfig, "Private");
    int compareMode = group.readEntry("Compare Mode", 0);
    int cmdExecMode =  group.readEntry("Command Execution Mode", 0);

    KStandardAction::home(SLOTS, SLOT(home()), actionCollection()/*, "std_home"*/)->setText(i18n("Home"));       /*->setShortcut(Qt::Key_QuoteLeft);*/

    KAction *reloadAct;
    NEW_KACTION(reloadAct, i18n("&Reload"), "view-refresh", Qt::CTRL + Qt::Key_R, SLOTS, SLOT(refresh()), "std_redisplay");

    actShowToolBar = (KToggleAction*)KStandardAction::create(KStandardAction::ShowToolbar, SLOTS, SLOT(toggleToolbar()), actionCollection()/*, "std_toolbar"*/);

    KToggleAction *toggleActToolbar;
    NEW_KTOGGLEACTION(toggleActToolbar, i18n("Show Actions Toolbar"), 0, 0, SLOTS, SLOT(toggleActionsToolbar()), "toggle actions toolbar");

    actShowStatusBar = KStandardAction::showStatusbar(SLOTS, SLOT(toggleStatusbar()), actionCollection());
    KStandardAction::quit(this, SLOT(slotClose()), actionCollection());
    KStandardAction::configureToolbars(SLOTS, SLOT(configToolbar()), actionCollection());
    KStandardAction::keyBindings(SLOTS, SLOT(configKeys()), actionCollection());

    KStandardAction::cut(SLOTS, SLOT(cut()), actionCollection())->setText(i18n("Cut to Clipboard"));
    (actCopy = KStandardAction::copy(SLOTS, SLOT(copy()), actionCollection()))->setText(i18n("Copy to Clipboard"));
    (actPaste = KStandardAction::paste(SLOTS, SLOT(paste()), actionCollection()))->setText(i18n("Paste from Clipboard"));

    // the toggle actions
    NEW_KTOGGLEACTION(actToggleFnkeys, i18n("Show &FN Keys Bar"), 0, 0, SLOTS,  SLOT(toggleFnkeys()), "toggle fn bar");

    NEW_KTOGGLEACTION(actToggleCmdline, i18n("Show &Command Line"), 0, 0, SLOTS, SLOT(toggleCmdline()), "toggle command line");

    NEW_KTOGGLEACTION(actToggleTerminal, i18n("Show Terminal &Emulator"), 0, Qt::ALT + Qt::CTRL + Qt::Key_T, SLOTS, SLOT(toggleTerminal()), "toggle terminal emulator");

    QList<KrViewInstance *> views = KrViewFactory::registeredViews();
    foreach(KrViewInstance * inst, views) {
        int id = inst->id();

        switch (id) {
        case 0:
            NEW_KACTION(actView0, i18n(inst->description().toUtf8()), 0, inst->shortcut(), SLOTS, SLOT(setView0()), "view0");
            break;
        case 1:
            NEW_KACTION(actView1, i18n(inst->description().toUtf8()), 0, inst->shortcut(), SLOTS, SLOT(setView1()), "view1");
            break;
        case 2:
            NEW_KACTION(actView2, i18n(inst->description().toUtf8()), 0, inst->shortcut(), SLOTS, SLOT(setView2()), "view2");
            break;
        case 3:
            NEW_KACTION(actView3, i18n(inst->description().toUtf8()), 0, inst->shortcut(), SLOTS, SLOT(setView3()), "view3");
            break;
        case 4:
            NEW_KACTION(actView4, i18n(inst->description().toUtf8()), 0, inst->shortcut(), SLOTS, SLOT(setView4()), "view4");
            break;
        case 5:
            NEW_KACTION(actView5, i18n(inst->description().toUtf8()), 0, inst->shortcut(), SLOTS, SLOT(setView5()), "view5");
            break;
        default:
            break;
        }
    }

    NEW_KTOGGLEACTION(actToggleHidden, i18n("Show &Hidden Files"), 0, Qt::CTRL + Qt::Key_Period, SLOTS, SLOT(toggleHidden()), "toggle hidden files");

    NEW_KTOGGLEACTION(actTogglePreviews, i18n("Show Previews"), 0, 0, SLOTS, SLOT(togglePreviews(bool)), "toggle previews");

    NEW_KACTION(actSwapPanels, i18n("S&wap Panels"), 0, Qt::CTRL + Qt::Key_U, SLOTS, SLOT(swapPanels()), "swap panels");

    NEW_KACTION(actEmptyTrash, i18n("Empty trash"), "trash-empty", 0, SLOTS, SLOT(emptyTrash()), "emptytrash");

    NEW_KACTION(actTrashBin, i18n("Trash bin"), KrTrashHandler::trashIcon(), 0, SLOTS, SLOT(trashBin()), "trashbin");

    NEW_KACTION(actSwapSides, i18n("Sw&ap Sides"), 0, Qt::CTRL + Qt::SHIFT + Qt::Key_U, SLOTS, SLOT(toggleSwapSides()), "toggle swap sides");
    actToggleHidden->setChecked(KConfigGroup(krConfig, "Look&Feel").readEntry("Show Hidden", _ShowHidden));

    // and then the DONE actions
    NEW_KACTION(actCmdlinePopup, i18n("popup cmdline"), 0, Qt::CTRL + Qt::Key_Slash, SLOTS, SLOT(cmdlinePopup()), "cmdline popup");

    /* Shortcut disabled because of the Terminal Emulator bug. */
    NEW_KACTION(actDirUp, i18n("Up"), "go-up", Qt::CTRL + Qt::Key_PageUp /*Qt::Key_Backspace*/, SLOTS, SLOT(dirUp()), "dirUp");

    KAction *tmp1, *tmp2, *tmp3;
    NEW_KACTION(tmp1, i18n("&New Text File..."), "document-new", Qt::SHIFT + Qt::Key_F4, SLOTS, SLOT(editDlg()), "edit_new_file");
    NEW_KACTION(tmp2, i18n("Start &Root Mode Krusader"), "krusader_root", Qt::ALT + Qt::SHIFT + Qt::Key_K, SLOTS, SLOT(rootKrusader()), "root krusader");
    NEW_KACTION(tmp3, i18n("F3 View Dialog"), 0, Qt::SHIFT + Qt::Key_F3, SLOTS, SLOT(viewDlg()), "F3_ViewDlg");
    NEW_KACTION(actTest, i18n("T&est Archive"), "utilities-file-archiver", Qt::ALT + Qt::SHIFT + Qt::Key_E, SLOTS, SLOT(testArchive()), "test archives");
    NEW_KACTION(actFTPNewConnect, i18n("New Net &Connection..."), "network-connect", Qt::CTRL + Qt::Key_N, SLOTS, SLOT(newFTPconnection()), "ftp new connection");
    NEW_KACTION(actProfiles, i18n("Pro&files"), "user-identity", Qt::ALT + Qt::SHIFT + Qt::Key_L, MAIN_VIEW, SLOT(profiles()), "profile");
    NEW_KACTION(actCalculate, i18n("Calculate &Occupied Space"), "kcalc", 0, SLOTS, SLOT(calcSpace()), "calculate");
    NEW_KACTION(actCreateChecksum, i18n("Create Checksum..."), "binary", 0, SLOTS, SLOT(createChecksum()), "create checksum");
    NEW_KACTION(actMatchChecksum, i18n("Verify Checksum..."), "binary", 0, SLOTS, SLOT(matchChecksum()), "match checksum");
    NEW_KACTION(actProperties, i18n("&Properties..."), 0, Qt::ALT + Qt::Key_Enter, SLOTS, SLOT(properties()), "properties");
    NEW_KACTION(actPack, i18n("Pac&k..."), "archive-insert", Qt::ALT + Qt::SHIFT + Qt::Key_P, SLOTS, SLOT(slotPack()), "pack");
    NEW_KACTION(actUnpack, i18n("&Unpack..."), "archive-extract", Qt::ALT + Qt::SHIFT + Qt::Key_U, SLOTS, SLOT(slotUnpack()), "unpack");
    NEW_KACTION(actSplit, i18n("Sp&lit File..."), "kr_split", Qt::CTRL + Qt::Key_P, SLOTS, SLOT(slotSplit()), "split");
    NEW_KACTION(actCombine, i18n("Com&bine Files..."), "kr_combine", Qt::CTRL + Qt::Key_B, SLOTS, SLOT(slotCombine()), "combine");
    NEW_KACTION(actSelect, i18n("Select &Group..."), "kr_select", Qt::CTRL + Qt::Key_Plus, SLOTS, SLOT(markGroup()), "select group");
    NEW_KACTION(actSelectAll, i18n("&Select All"), "kr_selectall", Qt::ALT + Qt::Key_Plus, SLOTS, SLOT(markAll()), "select all");
    NEW_KACTION(actUnselect, i18n("&Unselect Group..."), "kr_unselect", Qt::CTRL + Qt::Key_Minus, SLOTS, SLOT(unmarkGroup()), "unselect group");
    NEW_KACTION(actUnselectAll, i18n("U&nselect All"), "kr_unselectall", Qt::ALT + Qt::Key_Minus, SLOTS, SLOT(unmarkAll()), "unselect all");
    NEW_KACTION(actInvert, i18n("&Invert Selection"), "kr_invert", Qt::ALT + Qt::Key_Asterisk, SLOTS, SLOT(invert()), "invert");
    NEW_KACTION(actCompDirs, i18n("&Compare Directories"), "view_left_right", Qt::ALT + Qt::SHIFT + Qt::Key_C, SLOTS, SLOT(compareDirs()), "compare dirs");
    NEW_KACTION(actSelectNewerAndSingle, i18n("&Select Newer and Single"), 0, 0, SLOTS, SLOT(compareSetup()), "select_newer_and_single");
    NEW_KACTION(actSelectNewer, i18n("Select &Newer"), 0, 0, SLOTS, SLOT(compareSetup()), "select_newer");
    NEW_KACTION(actSelectSingle, i18n("Select &Single"), 0, 0, SLOTS, SLOT(compareSetup()), "select_single");
    NEW_KACTION(actSelectDifferentAndSingle, i18n("Select Different &and Single"), 0, 0, SLOTS, SLOT(compareSetup()), "select_different_and_single");
    NEW_KACTION(actSelectDifferent,  i18n("Select &Different"), 0, 0, SLOTS, SLOT(compareSetup()), "select_different");
    actSelectNewerAndSingle->setCheckable(true);
    actSelectNewer->setCheckable(true);
    actSelectSingle->setCheckable(true);
    actSelectDifferentAndSingle->setCheckable(true);
    actSelectDifferent->setCheckable(true);
    QActionGroup *selectGroup = new QActionGroup(this);
    selectGroup->setExclusive(true);
    selectGroup->addAction(actSelectNewerAndSingle);
    selectGroup->addAction(actSelectNewer);
    selectGroup->addAction(actSelectSingle);
    selectGroup->addAction(actSelectDifferentAndSingle);
    selectGroup->addAction(actSelectDifferent);
    if (compareMode < (int)(sizeof(compareArray) / sizeof(KAction **)) - 1)
        (*compareArray[ compareMode ])->setChecked(true);
    NEW_KACTION(actExecStartAndForget, i18n("Start and &Forget"), 0, 0, SLOTS, SLOT(execTypeSetup()), "exec_start_and_forget");
    NEW_KACTION(actExecCollectSeparate, i18n("Display &Separated Standard and Error Output"), 0, 0, SLOTS, SLOT(execTypeSetup()), "exec_collect_separate");
    NEW_KACTION(actExecCollectTogether, i18n("Display &Mixed Standard and Error Output"), 0, 0, SLOTS, SLOT(execTypeSetup()), "exec_collect_together");
    NEW_KACTION(actExecTerminalExternal, i18n("Start in &New Terminal"), 0, 0, SLOTS, SLOT(execTypeSetup()), "exec_terminal_external");
    NEW_KACTION(actExecTerminalEmbedded, i18n("Send to &Embedded Terminal Emulator"), 0, 0, SLOTS, SLOT(execTypeSetup()), "exec_terminal_embedded");
    actExecStartAndForget->setCheckable(true);
    actExecCollectSeparate->setCheckable(true);
    actExecCollectTogether->setCheckable(true);
    actExecTerminalExternal->setCheckable(true);
    actExecTerminalEmbedded->setCheckable(true);
    QActionGroup *actionGroup = new QActionGroup(this);
    actionGroup->setExclusive(true);
    actionGroup->addAction(actExecStartAndForget);
    actionGroup->addAction(actExecCollectSeparate);
    actionGroup->addAction(actExecCollectTogether);
    actionGroup->addAction(actExecTerminalExternal);
    actionGroup->addAction(actExecTerminalEmbedded);
    if (cmdExecMode < (int)(sizeof(execTypeArray) / sizeof(KAction **)) - 1)
        (*execTypeArray[ cmdExecMode ])->setChecked(true);


    NEW_KACTION(actHomeTerminal, i18n("Start &Terminal"), "terminal", 0, SLOTS, SLOT(homeTerminal()), "terminal@home");
    NEW_KACTION(actFTPDisconnect, i18n("Disconnect &from Net"), "network-disconnect", Qt::SHIFT + Qt::CTRL + Qt::Key_F, SLOTS, SLOT(FTPDisconnect()), "ftp disconnect");

    actMountMan = new KToolBarPopupAction(KIcon("kr_mountman"), i18n("&MountMan..."), this);
    actMountMan->setShortcut(Qt::ALT + Qt::Key_Slash);
    connect(actMountMan, SIGNAL(triggered(bool)), SLOTS, SLOT(runMountMan()));
    connect(((KToolBarPopupAction*) actMountMan) ->menu(), SIGNAL(aboutToShow()),
            mountMan, SLOT(quickList()));
    actionCollection()->addAction("mountman", actMountMan);

    NEW_KACTION(actFind, i18n("&Search..."), "system-search", Qt::CTRL + Qt::Key_S, SLOTS, SLOT(search()), "find");
    NEW_KACTION(actLocate, i18n("&Locate..."), "edit-find", Qt::SHIFT + Qt::CTRL + Qt::Key_L, SLOTS, SLOT(locate()), "locate");
    NEW_KACTION(actSyncDirs, i18n("Synchronize &Directories..."), "kr_syncdirs", Qt::CTRL + Qt::Key_Y, SLOTS, SLOT(slotSynchronizeDirs()), "sync dirs");
    NEW_KACTION(actSyncBrowse, i18n("S&ynchron Directory Changes"), "kr_syncbrowse_off", Qt::ALT + Qt::SHIFT + Qt::Key_Y, SLOTS, SLOT(slotSyncBrowse()), "sync browse");
    NEW_KACTION(actCancelRefresh, i18n("Cancel Refresh of View"), "kr_cancel_refresh", Qt::Key_Escape, SLOTS, SLOT(cancelRefresh()), "cancel refresh");
    NEW_KACTION(actDiskUsage, i18n("D&isk Usage..."), "kr_diskusage", Qt::ALT + Qt::SHIFT + Qt::Key_S, SLOTS, SLOT(slotDiskUsage()), "disk usage");
    NEW_KACTION(actQueueManager, i18n("&Queue Manager..."), "document-multiple", Qt::ALT + Qt::SHIFT + Qt::Key_Q, SLOTS, SLOT(slotQueueManager()), "queue manager");
    NEW_KACTION(actKonfigurator, i18n("Configure &Krusader..."), "configure", 0, SLOTS, SLOT(startKonfigurator()), "konfigurator");
    NEW_KACTION(actBack, i18n("Back"), "go-previous", 0, SLOTS, SLOT(back()), "back");
    NEW_KACTION(actRoot, i18n("Root"), "go-top", Qt::CTRL + Qt::Key_Backspace, SLOTS, SLOT(root()), "root");
    NEW_KACTION(actSavePosition, i18n("Save &Position"), 0, 0, krApp, SLOT(savePosition()), "save position");
    NEW_KACTION(actAllFilter, i18n("&All Files"), 0, Qt::SHIFT + Qt::Key_F10, SLOTS, SLOT(allFilter()), "all files");
    //actExecFilter = new KAction( i18n( "&Executables" ), SHIFT + Qt::Key_F11,
    //                             SLOTS, SLOT( execFilter() ), actionCollection(), "exec files" );
    NEW_KACTION(actCustomFilter, i18n("&Custom"), 0, Qt::SHIFT + Qt::Key_F12, SLOTS, SLOT(customFilter()), "custom files");
    NEW_KACTION(actCompare, i18n("Compare b&y Content..."), "kmultiple", 0, SLOTS, SLOT(compareContent()), "compare");
    NEW_KACTION(actMultiRename, i18n("Multi &Rename..."), "krename", Qt::SHIFT + Qt::Key_F9, SLOTS, SLOT(multiRename()), "multirename");

    KAction *t3, *t4, *t5, *t6, *t7, *t8, *t9, *t10, *t11, *t12, *t13, *t14, *t15, *t16;
    NEW_KACTION(t3, i18n("Right-click Menu"), 0, Qt::Key_Menu, SLOTS, SLOT(rightclickMenu()), "rightclick menu");
    NEW_KACTION(t4, i18n("Right Bookmarks"), 0, Qt::ALT + Qt::Key_Right, SLOTS, SLOT(openRightBookmarks()), "right bookmarks");
    NEW_KACTION(t5, i18n("Left Bookmarks"), 0, Qt::ALT + Qt::Key_Left, SLOTS, SLOT(openLeftBookmarks()), "left bookmarks");
    NEW_KACTION(t6, i18n("Bookmarks"), 0, Qt::CTRL + Qt::Key_D, SLOTS, SLOT(openBookmarks()), "bookmarks");
    NEW_KACTION(t7, i18n("Bookmark Current"), 0, Qt::CTRL + Qt::SHIFT + Qt::Key_D, SLOTS, SLOT(bookmarkCurrent()), "bookmark current");
    NEW_KACTION(t8, i18n("History"), 0, Qt::CTRL + Qt::Key_H, SLOTS, SLOT(openHistory()), "history");
    NEW_KACTION(t9, i18n("Sync Panels"), 0, Qt::ALT + Qt::SHIFT + Qt::Key_O, SLOTS, SLOT(syncPanels()), "sync panels");
    NEW_KACTION(t10, i18n("Left History"), 0, Qt::ALT + Qt::CTRL + Qt::Key_Left, SLOTS, SLOT(openLeftHistory()), "left history");
    NEW_KACTION(t11, i18n("Right History"), 0, Qt::ALT + Qt::CTRL + Qt::Key_Right, SLOTS, SLOT(openRightHistory()), "right history");
    NEW_KACTION(t12, i18n("Media"), 0, Qt::CTRL + Qt::Key_M, SLOTS, SLOT(openMedia()), "media");
    NEW_KACTION(t13, i18n("Left Media"), 0, Qt::CTRL + Qt::SHIFT + Qt::Key_Left, SLOTS, SLOT(openLeftMedia()), "left media");
    NEW_KACTION(t14, i18n("Right Media"), 0, Qt::CTRL + Qt::SHIFT + Qt::Key_Right, SLOTS, SLOT(openRightMedia()), "right media");
    NEW_KACTION(t15, i18n("New Symlink..."), 0, Qt::CTRL + Qt::ALT + Qt::Key_S, SLOTS, SLOT(newSymlink()), "new symlink");
    NEW_KTOGGLEACTION(t16, i18n("Toggle Popup Panel"), 0, Qt::ALT + Qt::Key_Down, SLOTS, SLOT(togglePopupPanel()), "toggle popup panel");
    NEW_KACTION(actVerticalMode, i18n("Vertical Mode"), "view-split-top-bottom", Qt::ALT + Qt::CTRL + Qt::Key_R, MAIN_VIEW, SLOT(toggleVerticalMode()), "toggle vertical mode");
    NEW_KACTION(actNewTab, i18n("New Tab"), "tab-new", Qt::ALT + Qt::CTRL + Qt::Key_N, SLOTS, SLOT(newTab()), "new tab");
    NEW_KACTION(actDupTab, i18n("Duplicate Current Tab"), "tab_duplicate", Qt::ALT + Qt::CTRL + Qt::SHIFT + Qt::Key_N, SLOTS, SLOT(duplicateTab()), "duplicate tab");
    NEW_KACTION(actCloseTab, i18n("Close Current Tab"), "tab-close", Qt::CTRL + Qt::Key_W, SLOTS, SLOT(closeTab()), "close tab");
    NEW_KACTION(actNextTab, i18n("Next Tab"), 0, Qt::SHIFT + Qt::Key_Right, SLOTS, SLOT(nextTab()), "next tab");
    NEW_KACTION(actPreviousTab, i18n("Previous Tab"), 0, Qt::SHIFT + Qt::Key_Left, SLOTS, SLOT(previousTab()), "previous tab");
    NEW_KACTION(actCloseInactiveTabs, i18n("Close Inactive Tabs"), 0, 0, SLOTS, SLOT(closeInactiveTabs()), "close inactive tabs");
    NEW_KACTION(actCloseDuplicatedTabs, i18n("Close Duplicated Tabs"), 0, 0, SLOTS, SLOT(closeDuplicatedTabs()), "close duplicated tabs");
    NEW_KACTION(actLockTab, i18n("Lock Tab"), 0, 0, SLOTS, SLOT(lockTab()), "lock tab");
#if 0
    actUserMenu = new KAction(i18n("User Menu"), ALT + Qt::Key_QuoteLeft, SLOTS,
                              SLOT(userMenu()), actionCollection(), "user menu");
#else
    actUserMenu = new KActionMenu(i18n("User&actions"), this);
    actionCollection()->addAction("useractionmenu", actUserMenu);
#endif
    NEW_KACTION(actManageUseractions, i18n("Manage User Actions..."), 0, 0, SLOTS, SLOT(manageUseractions()), "manage useractions");

    actRemoteEncoding = new KrRemoteEncodingMenu(i18n("Select Remote Charset"), "character-set", actionCollection());

    // setup the Fn keys
    NEW_KACTION(actF2, i18n("Start Terminal Here"), "terminal", Qt::Key_F2, SLOTS, SLOT(terminal()) , "F2_Terminal");
    NEW_KACTION(actF3, i18n("View File"), 0, Qt::Key_F3, SLOTS, SLOT(view()), "F3_View");
    NEW_KACTION(actF4, i18n("Edit File"), 0, Qt::Key_F4, SLOTS, SLOT(edit()) , "F4_Edit");
    NEW_KACTION(actF5, i18n("Copy..."), 0, Qt::Key_F5, SLOTS, SLOT(copyFiles()) , "F5_Copy");
    NEW_KACTION(actF6, i18n("Move..."), 0, Qt::Key_F6, SLOTS, SLOT(moveFiles()) , "F6_Move");
    NEW_KACTION(actShiftF5, i18n("Copy by queue..."), 0, Qt::SHIFT + Qt::Key_F5, SLOTS, SLOT(copyFilesByQueue()) , "F5_Copy_Queue");
    NEW_KACTION(actShiftF6, i18n("Move by queue..."), 0, Qt::SHIFT + Qt::Key_F6, SLOTS, SLOT(moveFilesByQueue()) , "F6_Move_Queue");
    NEW_KACTION(actF7, i18n("New Directory..."), "folder-new", Qt::Key_F7, SLOTS, SLOT(mkdir()) , "F7_Mkdir");
    NEW_KACTION(actF8, i18n("Delete"), "edit-delete", Qt::Key_F8, SLOTS, SLOT(deleteFiles()) , "F8_Delete");
    NEW_KACTION(actF9, i18n("Rename"), 0, Qt::Key_F9, SLOTS, SLOT(rename()) , "F9_Rename");
    NEW_KACTION(actF10, i18n("Quit"), 0, Qt::Key_F10, this, SLOT(slotClose()) , "F10_Quit");
    NEW_KACTION(actPopularUrls, i18n("Popular URLs..."), 0, Qt::CTRL + Qt::Key_Z, popularUrls, SLOT(showDialog()), "Popular_Urls");
    NEW_KACTION(actLocationBar, i18n("Go to Location Bar"), 0, Qt::CTRL + Qt::Key_L, SLOTS, SLOT(slotLocationBar()), "location_bar");
    NEW_KACTION(actJumpBack, i18n("Jump Back"), "kr_jumpback", Qt::CTRL + Qt::Key_J, SLOTS, SLOT(slotJumpBack()), "jump_back");
    NEW_KACTION(actSetJumpBack, i18n("Set Jump Back Point"), "kr_setjumpback", Qt::CTRL + Qt::SHIFT + Qt::Key_J, SLOTS, SLOT(slotSetJumpBack()), "set_jump_back");
    NEW_KACTION(actSwitchFullScreenTE, i18n("Toggle Fullwidget Terminal Emulator"), 0, Qt::CTRL + Qt::Key_F, MAIN_VIEW, SLOT(switchFullScreenTE()), "switch_fullscreen_te");
    NEW_KACTION(actZoomIn, i18n("Zoom In"), "zoom-in", 0, SLOTS, SLOT(zoomIn()), "zoom_in");
    NEW_KACTION(actZoomOut, i18n("Zoom Out"), "zoom-out", 0, SLOTS, SLOT(zoomOut()), "zoom_out");
    NEW_KACTION(actDefaultZoom, i18n("Default Zoom"), 0, 0, SLOTS, SLOT(defaultZoom()), "default_zoom");

    // and at last we can set the tool-tips
    actSelect->setToolTip(i18n("Select files using a filter"));
    actSelectAll->setToolTip(i18n("Select all files in the current directory"));
    actUnselectAll->setToolTip(i18n("Unselect all selected files"));
    actKonfigurator->setToolTip(i18n("Setup Krusader the way you like it"));
    actBack->setToolTip(i18n("Back to the place you came from"));
    actRoot->setToolTip(i18n("ROOT (/)"));
    actFind->setToolTip(i18n("Search for files"));
#endif
    KrActions::setupActions(this);
}

///////////////////////////////////////////////////////////////////////////
//////////////////// implementation of slots //////////////////////////////
///////////////////////////////////////////////////////////////////////////

void Krusader::savePosition() {
    KConfigGroup cfg(krConfig, "Private");
    cfg.writeEntry("Maximized", isMaximized());
    if (isMaximized())
        saveWindowSize(krConfig->group("Private"));
    else {
        cfg.writeEntry("Start Position", isMaximized() ? oldPos : pos());
        cfg.writeEntry("Start Size", isMaximized() ? oldSize : size());
    }
    QList<int> lst = MAIN_VIEW->horiz_splitter->sizes();
    cfg.writeEntry("Splitter Sizes", lst);
    MAIN_VIEW->left->popup->saveSizes();
    MAIN_VIEW->right->popup->saveSizes();
    if (!MAIN_VIEW->getTerminalEmulatorSplitterSizes().isEmpty())
        cfg.writeEntry("Terminal Emulator Splitter Sizes", MAIN_VIEW->getTerminalEmulatorSplitterSizes());

    // save view settings ---> fix when we have tabbed-browsing
    MAIN_VIEW->left->view->saveSettings();
    MAIN_VIEW->right->view->saveSettings();

    cfg = krConfig->group("Startup");
    cfg.writeEntry("Vertical Mode", MAIN_VIEW->isVertical());
    krConfig->sync();
}

void Krusader::saveSettings() {
    KConfigGroup cfg(krConfig, "Main Toolbar");
    toolBar() ->saveSettings(cfg);

    cfg = krConfig->group("Actions Toolbar");
    toolBar("actionsToolBar")->saveSettings(cfg);

    cfg = krConfig->group("Startup");
    MAIN_VIEW->saveSettings(cfg);

    bool rememberpos = cfg.readEntry("Remember Position", _RememberPos);
    bool uisavesettings = cfg.readEntry("UI Save Settings", _UiSave);

    // save the popup panel's page of the CURRENT tab
    cfg.writeEntry("Left Panel Popup", MAIN_VIEW->left->popup->currentPage());
    cfg.writeEntry("Right Panel Popup", MAIN_VIEW->right->popup->currentPage());

    // save size and position
    if (rememberpos || uisavesettings) {
        savePosition();
    }

    // save the gui
    if (uisavesettings) {
        cfg = krConfig->group("Startup");
        cfg.writeEntry("Show status bar", KrActions::actShowStatusBar->isChecked());
        cfg.writeEntry("Show tool bar", KrActions::actShowToolBar->isChecked());
        cfg.writeEntry("Show FN Keys", KrActions::actToggleFnkeys->isChecked());
        cfg.writeEntry("Show Cmd Line", KrActions::actToggleCmdline->isChecked());
        cfg.writeEntry("Show Terminal Emulator", KrActions::actToggleTerminal->isChecked());
        cfg.writeEntry("Vertical Mode", MAIN_VIEW->isVertical());
        cfg.writeEntry("Start To Tray", isHidden());
    }

    // save popular links
    popularUrls->save();

    krConfig->sync();
}

void Krusader::refreshView() {
    delete MAIN_VIEW;
    MAIN_VIEW = new KrusaderView(this);
    setCentralWidget(MAIN_VIEW);
    KConfigGroup cfg(krConfig, "Private");
    resize(cfg.readEntry("Start Size", _StartSize));
    move(cfg.readEntry("Start Position", _StartPosition));
    MAIN_VIEW->show();
    show();
}

void Krusader::configChanged() {
    KConfigGroup group(krConfig, "Look&Feel");
    bool minimizeToTray = group.readEntry("Minimize To Tray", _MinimizeToTray);
    bool singleInstanceMode = group.readEntry("Single Instance Mode", _SingleInstanceMode);

    if (sysTray) {
        if (!isHidden()) {
            if (singleInstanceMode && minimizeToTray)
                sysTray->show();
            else
                sysTray->hide();
        } else {
            if (minimizeToTray)
                sysTray->show();
        }
    }
}

void Krusader::slotClose() {
    directExit = true;
    close();
}

bool Krusader::queryClose() {
    if (isStarting || isExiting)
        return false;

    if (kapp->sessionSaving()) { // KDE is logging out, accept the close
        saveSettings();

        emit shutdown();

        // Removes the DBUS registration of the application. Single instance mode requires unique appid.
        // As Krusader is exiting, we release that unique appid, so new Krusader instances
        // can be started.

        QDBusConnection dbus = QDBusConnection::sessionBus();
        dbus.unregisterObject("/Instances/" + Krusader::AppName);

        KGlobal::deref(); // FIX: krusader exits at closing the viewer when minimized to tray
        sysTray->hide();
        delete sysTray;   // In KDE 4.1, KGlobal::ref() and deref() is done in KSystray constructor/destructor
        sysTray = NULL;
        KGlobal::deref(); // and close the application
        return isExiting = true;              // this will also kill the pending jobs
    }

    KConfigGroup cfg = krConfig->group("Look&Feel");
    if (!directExit && cfg.readEntry("Single Instance Mode", _SingleInstanceMode) &&
            cfg.readEntry("Minimize To Tray", _MinimizeToTray)) {
        hide();
        return false;
    }

    // the shutdown process can be cancelled. That's why
    // the directExit variable is set to normal here.
    directExit = false;

    bool quit = true;

    if (cfg.readEntry("Warn On Exit", _WarnOnExit)) {
        switch (KMessageBox::warningYesNo(this,
                                          i18n("Are you sure you want to quit?"))) {
        case KMessageBox::Yes :
            quit = true;
            break;
        case KMessageBox::No :
            quit = false;
            break;
        default:
            quit = false;
        }
    }
    if (quit) {
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
                    /*activeModal != sysTray && ==> TODO: commented since KSystemTrayIcon is no longer a QWidget  */
                    list.contains(activeModal) &&
                    !activeModal->isHidden()) {
                w = activeModal;
            } else {
                int i = 1;
                for (; i < list.count(); ++i) {
                    w = list.at(i);
                    if (!(w && (w == this || /* w==sysTray ||*/ w->isHidden() || w == menuBar())))
                        break;
                }

                if (i == list.count())
                    w = 0;
            }

            if (!w) break;
            bool hid = false;

            if (w->inherits("KDialog")) { // KDE is funny and rejects the close event for
                w->hide();                         // playing a fancy animation with the CANCEL button.
                hid = true;                        // if we hide the widget, KDialog accepts the close event
            }

            if (!w->close()) {
                if (hid)
                    w->show();

                if (w->inherits("QDialog"))
                    fprintf(stderr, "Failed to close: %s\n", w->metaObject()->className());

                return false;
            }
        }

        saveSettings();

        emit shutdown();

        isExiting = true;
        hide();        // hide

        // Removes the DBUS registration of the application. Single instance mode requires unique appid.
        // As Krusader is exiting, we release that unique appid, so new Krusader instances
        // can be started.

        QDBusConnection dbus = QDBusConnection::sessionBus();
        dbus.unregisterObject("/Instances/" + Krusader::AppName);

        KGlobal::deref(); // FIX: krusader exits at closing the viewer when minimized to tray
        sysTray->hide();
        delete sysTray;   // In KDE 4.1, KGlobal::ref() and deref() is done in KSystray constructor/destructor
        sysTray = NULL;
        KGlobal::deref(); // and close the application
        return false;  // don't let the main widget close. It stops the pendig copies!
        //FIXME: The above intention does not work (at least in KDE 4.1), because the job
        //progress window (class KWidgetJobTracker::Private::ProgressWidget)
        //is closed above among other top level windows, and when closed
        //stops the copy.
    } else
        return false;
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
    createGUI(MAIN_VIEW->terminal_dock->part());

    // this needs to be called AFTER createGUI() !!!
    updateUserActions();

    KConfigGroup cfg_toolbar(krConfig, "Main Toolbar");
    toolBar()->applySettings(cfg_toolbar);

    KConfigGroup cfg_act(krConfig->group("Actions Toolbar"));
    toolBar("actionsToolBar") ->applySettings(cfg_act);
    static_cast<KToggleAction*>(actionCollection()->action("toggle actions toolbar"))->
    setChecked(toolBar("actionsToolBar")->isVisible());

    KConfigGroup cfg(krConfig, "Startup");
    if (enforce) {
        // now, hide what need to be hidden
        if (!cfg.readEntry("Show tool bar", _ShowToolBar)) {
            toolBar() ->hide();
            KrActions::actShowToolBar->setChecked(false);
        } else {
            toolBar() ->show();
            KrActions::actShowToolBar->setChecked(true);
        }
        if (!cfg.readEntry("Show status bar", _ShowStatusBar)) {
            statusBar() ->hide();
            KrActions::actShowStatusBar->setChecked(false);
        } else {
            statusBar() ->show();
            KrActions::actShowStatusBar->setChecked(true);
        }
        if (!cfg.readEntry("Show Cmd Line", _ShowCmdline)) {
            MAIN_VIEW->cmdLine->hide();
            KrActions::actToggleCmdline->setChecked(false);
        } else {
            MAIN_VIEW->cmdLine->show();
            KrActions::actToggleCmdline->setChecked(true);
        }

        // update the Fn bar to the shortcuts selected by the user
        MAIN_VIEW->fnKeys->updateShortcuts();
        if (!cfg.readEntry("Show FN Keys", _ShowFNkeys)) {
            MAIN_VIEW->fnKeys->hide();
            KrActions::actToggleFnkeys->setChecked(false);
        } else {
            MAIN_VIEW->fnKeys->show();
            KrActions::actToggleFnkeys->setChecked(true);
        }
        // set vertical mode
        if (cfg.readEntry("Vertical Mode", false)) {
            MAIN_VIEW->toggleVerticalMode();
        }
        if (cfg.readEntry("Show Terminal Emulator", _ShowTerminalEmulator)) {
            MAIN_VIEW->slotTerminalEmulator(true);   // create konsole_part
            MAIN_VIEW->vert_splitter->setSizes(MAIN_VIEW->verticalSplitterSizes);
        } else if (KrActions::actExecTerminalEmbedded->isChecked()) {
            //create (but not show) terminal emulator,
            //if command-line commands are to be run there
            MAIN_VIEW->terminal_dock->initialise();
        }
    }
    // popular urls
    popularUrls->load();

}

// Adds one tool to the list in the supportedTools method
void Krusader::supportedTool(QStringList &tools, QString toolType,
                             QStringList names, QString confName) {
    QString foundTool = KrServices::chooseFullPathName(names, confName);
    if (! foundTool.isEmpty()) {
        tools.append(toolType);
        tools.append(foundTool);
    }
}

// return a list in the format of TOOLS,PATH. for example
// DIFF,kdiff,TERMINAL,konsole,...
//
// currently supported tools: DIFF, MAIL, RENAME
//
// to use it: QStringList lst = supportedTools();
//            int i = lst.indexOf("DIFF");
//            if (i!=-1) pathToDiff=lst[i+1];
QStringList Krusader::supportedTools() {
    QStringList tools;

    // first, a diff program: kdiff
    supportedTool(tools, "DIFF",
                  QStringList() << "kdiff3" << "kompare" << "xxdiff",
                  "diff utility");

    // a mailer: kmail or thunderbird
    supportedTool(tools, "MAIL",
                  QStringList() << "thunderbird" << "kmail",
                  "mailer");

    // rename tool: krename
    supportedTool(tools, "RENAME",
                  QStringList() << "krename",
                  "krename");

    // checksum utility
    supportedTool(tools, "MD5",
                  QStringList() << "md5deep" << "md5sum" << "sha1deep" << "sha256deep"
                  << "tigerdeep" << "whirlpooldeep" << "cfv",
                  "checksum utility");

    return tools;
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
        KMessageBox::error(krApp, i18n("Could not create a temporary directory. Handling of Archives will not be possible until this is fixed."));
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
    moveToTop();
    return true;
}

bool Krusader::isLeftActive()  {
    return MAIN_VIEW->isLeftActive();
}

bool Krusader::queryExit()
{
    krConfig->sync();
    return true;
}


#include "krusader.moc"
