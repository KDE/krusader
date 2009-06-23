/***************************************************************************
                                main.cpp
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

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <QtCore/QEventLoop>
#include <QtCore/QAbstractEventDispatcher>
#include <QtGui/QPixmap>
#include <QtDBus/QtDBus>

#include <kde_file.h>
#include <KCmdLineArgs>
#include <KAboutData>
#include <KActionMenu>
#include <KLocale>
#include <KStandardDirs>
#include <KSplashScreen>
#include <KStartupInfo>

#include "krusader.h"
#include "krusaderversion.h"
#include "krslots.h"
#include "krusaderapp.h"
#include "defaults.h"
#include "Panel/krviewfactory.h"

static const char *description = "Krusader\nTwin-Panel File Manager for KDE";

static void sigterm_handler(int i)
{
    fprintf(stderr, "Signal: %d\n", i);

    QAbstractEventDispatcher *instance = QAbstractEventDispatcher::instance();
    if (instance)
        instance->wakeUp();
    KApplication::exit(- 15);
}

int main(int argc, char *argv[])
{
// ============ begin icon-stuff ===========
// If the user has no icon specified over the commandline we set up uor own.
// this is according to the users privileges. The icons are in Krusader::privIcon()
    KrViewFactory::init();


    bool hasIcon = false;
    int i = 0;
    char * myArgv[argc+2];

// if no --miniicon is given, --icon is used. So we don't need to check for --miniicon separately
    for (i = 0; i < argc; ++i) {
        if (strstr(argv[ i ], "-icon") != 0)   // important: just one dash because both can appear!
            hasIcon = true;
    }

    static const char* const icon_text = "--icon";
    const char* icon_name = Krusader::privIcon();
    char addedParams[strlen(icon_text)+strlen(icon_name)+2];

    if (! hasIcon) {
        for (i = 0; i < argc; ++i)
            myArgv[ i ] = argv[ i ];

        strcpy(addedParams, icon_text);
        strcpy(addedParams + strlen(icon_text) + 1, icon_name);

        myArgv[ argc ] = addedParams;
        myArgv[ ++argc ] = addedParams + strlen(icon_text) + 1;
        myArgv[ ++argc ] = 0;

        argv = myArgv;
    }
// ============ end icon-stuff ===========

    // ABOUT data information
#ifdef RELEASE_NAME
    QString versionName = QString("%1 \"%2\"").arg(VERSION).arg(RELEASE_NAME);
#else
    QString versionName = VERSION;
#endif

    KAboutData aboutData("krusader", 0, (geteuid() ? ki18n("Krusader") : ki18n("Krusader - ROOT PRIVILEGES")),
                         versionName.toLocal8Bit(), ki18n(description), KAboutData::License_GPL_V2,
                         ki18n("(c) 2000-2003, Shie Erlich, Rafi Yanai\n(c) 2004-2009, Krusader Krew"),
                         ki18n("Feedback:\nhttp://www.krusader.org/phpBB/\n\nIRC\nserver: irc.freenode.net, channel: #krusader"),
                         "http://www.krusader.org");

    aboutData.addAuthor(ki18n("Rafi Yanai"), ki18n("Author"), "yanai@users.sourceforge.net");
    aboutData.addAuthor(ki18n("Shie Erlich"), ki18n("Author"), "erlich@users.sourceforge.net");
    aboutData.addAuthor(ki18n("Karai Csaba"), ki18n("Developer"), "ckarai@users.sourceforge.net", 0);
    aboutData.addAuthor(ki18n("Heiner Eichmann"), ki18n("Developer"), "h.eichmann@gmx.de", 0);
    aboutData.addAuthor(ki18n("Jonas Bähr"), ki18n("Developer"), "jonas.baehr@web.de", 0);
    aboutData.addAuthor(ki18n("Václav Jůza"), ki18n("Developer"), "vaclavjuza@gmail.com", 0);
    aboutData.addAuthor(ki18n("Dirk Eschler"), ki18n("Webmaster and i18n coordinator"), "deschler@users.sourceforge.net", 0);
    aboutData.addAuthor(ki18n("Frank Schoolmeesters"), ki18n("Documentation and marketing coordinator"), "frank_schoolmeesters@yahoo.com", 0);
    aboutData.addAuthor(ki18n("Richard Holt"), ki18n("Documentation & Proofing"), "richard.holt@gmail.com", 0);
    aboutData.addAuthor(ki18n("Matej Urbancic"), ki18n("Marketing & Product Research"), "matej.urban@gmail.com", 0);
    aboutData.addCredit(ki18n("Václav Jůza"), ki18n("QA, bug-hunting, patches and general help"), "vaclavjuza@gmail.com", 0);
    aboutData.addCredit(ki18n("Jiří Paleček"), ki18n("QA, bug-hunting, patches and general help"), "jpalecek@web.de", 0);
    aboutData.addCredit(ki18n("Jiří Klement"), ki18n("Important help in KDE 4 porting"), 0, 0);
    aboutData.addCredit(ki18n("Andrew Neupokoev"), ki18n("Killer Logo and Icons for Krusader (contest winner)"), "doom-blue@yandex.ru", 0);
    aboutData.addCredit(ki18n("The UsefulArts Organization"), ki18n("Icon for krusader"), "mail@usefularts.rg", 0);
    aboutData.addCredit(ki18n("Gábor Lehel"), ki18n("Viewer module for 3rd Hand"), "illissius@gmail.com", 0);
    aboutData.addCredit(ki18n("Mark Eatough"), ki18n("Handbook Proof-Reader"), "markeatough@yahoo.com", 0);
    aboutData.addCredit(ki18n("Jan Halasa"), ki18n("The old Bookmark Module"), "xhalasa@fi.muni.cz", 0);
    aboutData.addCredit(ki18n("Hans Löffler"), ki18n("Dir history button"), 0, 0);
    aboutData.addCredit(ki18n("Szombathelyi György"), ki18n("ISO KIO slave"), 0, 0);
    aboutData.addCredit(ki18n("Jan Willem van de Meent (Adios)"), ki18n("Icons for Krusader"), "janwillem@lorentz.leidenuniv.nl", 0);
    aboutData.addCredit(ki18n("Mikolaj Machowski"), ki18n("Usability and QA"), "<mikmach@wp.pl>", 0);
    aboutData.addCredit(ki18n("Cristi Dumitrescu"), ki18n("QA, bug-hunting, patches and general help"), "cristid@chip.ro", 0);
    aboutData.addCredit(ki18n("Aurelien Gateau"), ki18n("patch for KViewer"), "aurelien.gateau@free.fr", 0);
    aboutData.addCredit(ki18n("Milan Brabec"), ki18n("the first patch ever !"), "mbrabec@volny.cz", 0);
    aboutData.addCredit(ki18n("Asim Husanovic"), ki18n("Bosnian translation"), "asim@megatel.ba", 0);
    aboutData.addCredit(ki18n("Doutor Zero"), ki18n("Brazilian Portuguese translation"), "doutor.zero@gmail.com", 0);
    aboutData.addCredit(ki18n("Milen Ivanov"), ki18n("Bulgarian translation"), "milen.ivanov@abv.bg", 0);
    aboutData.addCredit(ki18n("Quim Perez"), ki18n("Catalan translation"), "noguer@osona.com", 0);
    aboutData.addCredit(ki18n("Jinghua Luo"), ki18n("Chinese Simplified translation"), "luojinghua@msn.com", 0);
    aboutData.addCredit(ki18n("Mitek"), ki18n("Old Czech translation"), "mitek@email.cz", 0);
    aboutData.addCredit(ki18n("Martin Sixta"), ki18n("Old Czech translation"), "lukumo84@seznam.cz", 0);
    aboutData.addCredit(ki18n("Vaclav Jůza"), ki18n("Czech translation"), "VaclavJuza@gmail.com", 0);
    aboutData.addCredit(ki18n("Anders Bruun Olsen"), ki18n("Old Danish translation"), "anders@bruun-olsen.net", 0);
    aboutData.addCredit(ki18n("Peter H. Sorensen"), ki18n("Danish translation"), "peters@skydebanen.net", 0);
    aboutData.addCredit(ki18n("Frank Schoolmeesters"), ki18n("Dutch translation"), "frank_schoolmeesters@yahoo.com", 0);
    aboutData.addCredit(ki18n("Rene-Pierre Lehmann"), ki18n("Old French translation"), "ripi@lepi.org", 0);
    aboutData.addCredit(ki18n("David Guillerm"), ki18n("French translation"), "dguillerm@gmail.com", 0);
    aboutData.addCredit(ki18n("Christoph Thielecke"), ki18n("Old German translation"), "crissi99@gmx.de", 0);
    aboutData.addCredit(ki18n("Dirk Eschler"), ki18n("German translation"), "deschler@users.sourceforge.net", 0);
    aboutData.addCredit(ki18n("Spiros Georgaras"), ki18n("Greek translation"), "sngeorgaras@gmail.com", 0);
    aboutData.addCredit(ki18n("Kukk Zoltan"), ki18n("Old Hungarian translation"), "kukkzoli@freemail.hu", 0);
    aboutData.addCredit(ki18n("Arpad Biro"), ki18n("Hungarian translation"), "biro_arpad@yahoo.com", 0);
    aboutData.addCredit(ki18n("Giuseppe Bordoni"), ki18n("Italian translation"), "geppo@geppozone.com", 0);
    aboutData.addCredit(ki18n("Hideki Kimura"), ki18n("Japanese translation"), "hangyo1973@gmail.com", 0);
    aboutData.addCredit(ki18n("UTUMI Hirosi"), ki18n("Old Japanese translation"), "utuhiro@mx12.freecom.ne.jp", 0);
    aboutData.addCredit(ki18n("Dovydas Sankauskas"), ki18n("Lithuanian translation"), "laisve@gmail.com", 0);
    aboutData.addCredit(ki18n("Bruno Queiros"), ki18n("Portuguese translation"), "brunoqueiros@portugalmail.com", 0);
    aboutData.addCredit(ki18n("Lukasz Janyst"), ki18n("Old Polish translation"), "ljan@wp.pl", 0);
    aboutData.addCredit(ki18n("Pawel Salawa"), ki18n("Polish translation"), "boogie@myslenice.one.pl", 0);
    aboutData.addCredit(ki18n("Tomek Grzejszczyk"), ki18n("Polish translation"), "tgrzej@onet.eu", 0);
    aboutData.addCredit(ki18n("Dmitry A. Bugay"), ki18n("Russian translation"), "sam@vhnet.ru", 0);
    aboutData.addCredit(ki18n("Dmitry Chernyak"), ki18n("Old Russian translation"), "chernyak@mail.ru", 0);
    aboutData.addCredit(ki18n("Sasa Tomic"), ki18n("Serbian translation"), "stomic@gmx.net", 0);
    aboutData.addCredit(ki18n("Zdenko Podobný and Ondrej Pačay (Yogi)"), ki18n("Slovak translation"), "zdenop@gmail.com", 0);
    aboutData.addCredit(ki18n("Matej Urbancic"), ki18n("Slovenian translation"), "matej.urban@gmail.com", 0);
    aboutData.addCredit(ki18n("Rafael Munoz"), ki18n("Old Spanish translation"), "muror@hotpop.com", 0);
    aboutData.addCredit(ki18n("Alejandro Araiza Alvarado"), ki18n("Spanish translation"), "mebrelith@gmail.com", 0);
    aboutData.addCredit(ki18n("Erik Johanssen"), ki18n("Old Swedish translation"), "erre@telia.com", 0);
    aboutData.addCredit(ki18n("Anders Linden"), ki18n("Old Swedish translation"), "connyosis@gmx.net", 0);
    aboutData.addCredit(ki18n("Peter Landgren"), ki18n("Swedish translation"), "peter.talken@telia.com", 0);
    aboutData.addCredit(ki18n("Bekir Sonat"), ki18n("Turkish translation"), "bekirsonat@kde.org.tr", 0);
    aboutData.addCredit(ki18n("Ivan Petrouchtchak"), ki18n("Ukrainian translation"), "connyosis@gmx.net", 0);
    aboutData.addCredit(ki18n("Seongnam Jee"), ki18n("Korean translation"), "snjee@intellicam.com", 0);

    // Command line arguments ...
    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("left <path>", ki18n("Start left panel at <path>"));
    options.add("right <path>", ki18n("Start right panel at <path>"));
    options.add("profile <panel-profile>", ki18n("Load this profile on startup"));

    KCmdLineArgs::addCmdLineOptions(options);   // Add our own options.

    // check for command line arguments

    // create the application
    KrusaderApp app;

    KConfigGroup cfg(KGlobal::config().data(), "Look&Feel");
    bool singleInstanceMode = cfg.readEntry("Single Instance Mode", _SingleInstanceMode);

    QString appName = "krusader";
    if (!singleInstanceMode)
        appName += QString("%1").arg(getpid());

    if (!QDBusConnection::sessionBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-BUS session bus.\n"
                "To start it, run:\n"
                "\teval `dbus-launch --auto-syntax`\n");
    }

    QDBusInterface remoteApp("org.krusader", "/Instances/" + appName,
                             "org.krusader.Instance", QDBusConnection::sessionBus());
    QDBusReply<bool> reply;
    if (remoteApp.isValid())
        reply = remoteApp.call("isRunning");

    if (!reply.isValid() && reply.error().type() != QDBusError::ServiceUnknown &&
            reply.error().type() != QDBusError::UnknownObject)
        fprintf(stderr, "DBus Error: %s, %s\n", reply.error().name().toLocal8Bit().constData(), reply.error().message().toLocal8Bit().constData());

    if (reply.isValid() && (bool)reply) {
        fprintf(stderr, "%s", i18n("Application already running!\n").toLocal8Bit().data());
        KStartupInfo::appStarted();
        return 1;
    }

    // splash screen - if the user wants one
    KSplashScreen *splash = 0;
    { // don't remove bracket
        KConfigGroup cfg(KGlobal::config().data(), "Look&Feel");
        if (cfg.readEntry("Show splashscreen", _ShowSplashScreen)) {
            QString splashFilename = KStandardDirs::locate("data", "krusader/splash.png");
            QPixmap pixmap(splashFilename);
            if (!pixmap.isNull()) {
                splash = new KSplashScreen(pixmap);
                splash->show();
            }
        }
    } // don't remove bracket

    Krusader::AppName = appName;
    Krusader krusader;

    QDBusConnection dbus = QDBusConnection::sessionBus();
    if (!dbus.interface()->isServiceRegistered("org.krusader") && !dbus.registerService("org.krusader")) {
        fprintf(stderr, "DBus Error: %s, %s\n", dbus.lastError().name().toLocal8Bit().constData(), dbus.lastError().message().toLocal8Bit().constData());
    }
    if (!dbus.registerObject("/Instances/" + appName, &krusader, QDBusConnection::ExportScriptableSlots)) {
        fprintf(stderr, "DBus Error: %s, %s\n", dbus.lastError().name().toLocal8Bit().constData(), dbus.lastError().message().toLocal8Bit().constData());
    }

    // catching SIGTERM, SIGHUP, SIGQUIT
    KDE_signal(SIGTERM, sigterm_handler);
    KDE_signal(SIGPIPE, sigterm_handler);
    KDE_signal(SIGHUP, sigterm_handler);

    // make sure we receive X's focus in/out events
    QObject::connect(&app, SIGNAL(windowActive()), krusader.slot, SLOT(windowActive()));
    QObject::connect(&app, SIGNAL(windowInactive()), krusader.slot, SLOT(windowInactive()));

    // hide splashscreen
    if (splash) {
        splash->finish(&krusader);
        delete splash;
    }

    // let's go.
    return app.exec();
}
