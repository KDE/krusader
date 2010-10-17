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
#include "panelmanager.h"

// define the static members
Krusader *Krusader::App = 0;
QString   Krusader::AppName;
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
        _listPanelActions(0), status(0), sysTray(0), isStarting(true),
        isExiting(false), directExit(false)
{

    setAttribute(Qt::WA_DeleteOnClose);
    // parse command line arguments
    KCmdLineArgs * args = KCmdLineArgs::parsedArgs();

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
    connect(KrGlobal::mountMan, SIGNAL(refreshPanel(const KUrl &)), SLOTS, SLOT(refresh(const KUrl &)));

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

// <patch> Moving from Pixmap actions to generic filenames - thanks to Carsten Pfeiffer
void Krusader::setupActions() {
    KAction * tab = new KAction("Tab-Switch panel", this);
    tab->setShortcut(Qt::Key_Tab);
    connect(tab, SIGNAL(triggered(bool)), MAIN_VIEW, SLOT(panelSwitch()));
    actionCollection()->addAction("tab", tab);
    KrActions::setupActions(this);
    _listPanelActions = new ListPanelActions(this, this);
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
    MAIN_VIEW->left->saveSettings(cfg);
    MAIN_VIEW->right->saveSettings(cfg);

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

            if (w->inherits("KDialog") && !w->inherits("Konfigurator")) {
                // KDE is funny and rejects the close event for
                // playing a fancy animation with the CANCEL button.
                // if we hide the widget, KDialog accepts the close event
                // dont't hide Konfigurator - see Konfigurator::closeEvent()
                w->hide();
                hid = true;
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

void Krusader::openUrl(QString url)
{
    int tab = ACTIVE_MNG->findTab(url);
    if(tab > 0)
        ACTIVE_MNG->setActiveTab(tab);
    else if((tab = OTHER_MNG->findTab(url)) > 0)
        OTHER_MNG->setActiveTab(tab);
    else
        ACTIVE_MNG->slotNewTab(url);
}

bool Krusader::queryExit()
{
    krConfig->sync();
    return true;
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
    return MAIN_VIEW->leftMng;
}

AbstractPanelManager *Krusader::rightManager()
{
    return MAIN_VIEW->rightMng;
}

ListPanelActions *Krusader::listPanelActions()
{
    return _listPanelActions;
}

#include "krusader.moc"
