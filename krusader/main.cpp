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

#include <csignal>
#include <unistd.h>

// QtCore
#include <QAbstractEventDispatcher>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QEventLoop>
#include <QStandardPaths>
#include <QDir>
// QtGui
#include <QPixmap>
// QtDBus
#include <QDBusInterface>
#include <QDBusConnectionInterface>
// QtWidgets
#include <QApplication>
#include <QSplashScreen>

#include <KCoreAddons/KAboutData>
#include <KConfigCore/KSharedConfig>
#include <KI18n/KLocalizedString>
#include <KWidgetsAddons/KActionMenu>
#include <KWindowSystem/KStartupInfo>

#include "../Archive/krarchandler.h"

#include "defaults.h"
#include "krservices.h"
#include "krslots.h"
#include "krusader.h"
#include "krusaderversion.h"
#include "krusaderview.h"
#include "panelmanager.h"

static const char *description = I18N_NOOP("Krusader\nTwin-Panel File Manager by KDE");

static void sigterm_handler(int i)
{
    fprintf(stderr, "Signal: %d\n", i);

    QAbstractEventDispatcher *instance = QAbstractEventDispatcher::instance();
    if (instance)
        instance->wakeUp();
    QApplication::exit(- 15);
}

void openTabsRemote(QStringList tabs, bool left, QString appName)
{
    // make sure left or right are not relative paths
    for (int i = 0; i != tabs.count(); i++) {
        tabs[ i ] = tabs[ i ].trimmed();
        if (!tabs[ i ].startsWith('/') && tabs[ i ].indexOf(":/") < 0)
            tabs[ i ] = QDir::currentPath() + '/' + tabs[ i ];
    }

    QDBusInterface remoteApp("org.krusader", "/Instances/" + appName + (left ? "/left_manager" : "/right_manager"),
                             "org.krusader.PanelManager", QDBusConnection::sessionBus());
    QDBusReply<void> reply;
    if (remoteApp.isValid())
        reply = remoteApp.call("newTabs", tabs);

    if (!reply.isValid())
        fprintf(stderr, "DBus Error: %s, %s\n", reply.error().name().toLocal8Bit().constData(), reply.error().message().toLocal8Bit().constData());
}

//! An object that manages archives in several parts of the source code.
KRarcHandler arcHandler;

int main(int argc, char *argv[])
{
// ============ begin icon-stuff ===========
// If the user has no icon specified over the commandline we set up our own.
// this is according to the users privileges. The icons are in Krusader::privIcon()

/*    bool hasIcon = false;
    int i = 0;
    char * myArgv[argc+2];*/

// if no --miniicon is given, --icon is used. So we don't need to check for --miniicon separately
/*    for (i = 0; i < argc; ++i) {
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
    }*/
// ============ end icon-stuff ===========

    // set global log message format
    qSetMessagePattern(KrServices::GLOBAL_MESSAGE_PATTERN);

    // prevent qt5-webengine crashing
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    // create the application and set application domain so that calls to i18n get strings from right place.
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("krusader");

    // ABOUT data information
#ifdef RELEASE_NAME
    QString versionName = QString("%1 \"%2\"").arg(VERSION).arg(RELEASE_NAME);
#else
    QString versionName = VERSION;
#endif

    KAboutData aboutData(QStringLiteral("krusader"),
        (geteuid() ? i18n("Krusader") : i18n("Krusader - ROOT PRIVILEGES")), versionName,
        i18n(description), KAboutLicense::GPL_V2,
        i18n("(c) 2000-2003, Shie Erlich, Rafi Yanai\n(c) 2004-2016, Krusader Krew"),
        i18n("Feedback:\nhttps://forum.kde.org/viewforum.php?f=225\n\nIRC\nserver: "
             "irc.freenode.net, channel: #krusader"),
        QStringLiteral("https://krusader.org"));

    aboutData.setOrganizationDomain(QByteArray("kde.org"));
    aboutData.setDesktopFileName(QStringLiteral("org.kde.krusader"));

    aboutData.addAuthor(i18n("Rafi Yanai"), i18n("Author (retired)"), QStringLiteral("yanai@users.sourceforge.net"));
    aboutData.addAuthor(i18n("Shie Erlich"), i18n("Author (retired)"), QStringLiteral("erlich@users.sourceforge.net"));
    aboutData.addAuthor(i18n("Jan Lepper"), i18n("Developer"), QStringLiteral("jan_lepper@gmx.de"), 0);
    aboutData.addAuthor(i18n("Andrey Matveyakin"), i18n("Developer"), QStringLiteral("a.matveyakin@gmail.com"), 0);
    aboutData.addAuthor(i18n("Simon Persson"), i18n("Developer"), QStringLiteral("simon.persson@mykolab.com"), 0);
    aboutData.addAuthor(i18n("Davide Gianforte"), i18n("Developer"), QStringLiteral("davide@gengisdave.org"), 0);
    aboutData.addAuthor(i18n("Toni Asensi Esteve"), i18n("Developer"), QStringLiteral("toni.asensi@kdemail.net"), 0);
    aboutData.addAuthor(i18n("Alexander Bikadorov"), i18n("Developer"), QStringLiteral("alex.bikadorov@kdemail.net"), 0);
    aboutData.addAuthor(i18n("Karai Csaba"), i18n("Developer (retired)"), QStringLiteral("ckarai@users.sourceforge.net"), 0);
    aboutData.addAuthor(i18n("Heiner Eichmann"), i18n("Developer (retired)"), QStringLiteral("h.eichmann@gmx.de"), 0);
    aboutData.addAuthor(i18n("Jonas Bähr"), i18n("Developer (retired)"), QStringLiteral("jonas.baehr@web.de"), 0);
    aboutData.addAuthor(i18n("Václav Jůza"), i18n("Developer (retired)"), QStringLiteral("vaclavjuza@gmail.com"), 0);
    aboutData.addAuthor(i18n("Dirk Eschler"), i18n("Webmaster (retired)"), QStringLiteral("deschler@users.sourceforge.net"), 0);
    aboutData.addAuthor(i18n("Frank Schoolmeesters"), i18n("Documentation and marketing coordinator (retired)"), QStringLiteral("frank_schoolmeesters@yahoo.com"), 0);
    aboutData.addAuthor(i18n("Richard Holt"), i18n("Documentation & Proofing (retired)"), QStringLiteral("richard.holt@gmail.com"), 0);
    aboutData.addAuthor(i18n("Matej Urbancic"), i18n("Marketing & Product Research (retired)"), QStringLiteral("matej.urban@gmail.com"), 0);
    aboutData.addCredit(i18n("kde.org"), i18n("Everyone involved in KDE"), 0, 0);
    aboutData.addCredit(i18n("l10n.kde.org"), i18n("KDE Translation Teams"), 0, 0);
    aboutData.addCredit(i18n("Jiří Paleček"), i18n("QA, bug-hunting, patches and general help"), QStringLiteral("jpalecek@web.de"), 0);
    aboutData.addCredit(i18n("Jiří Klement"), i18n("Important help in KDE 4 porting"), 0, 0);
    aboutData.addCredit(i18n("Andrew Neupokoev"), i18n("Killer Logo and Icons for Krusader (contest winner)"), QStringLiteral("doom-blue@yandex.ru"), 0);
    aboutData.addCredit(i18n("The UsefulArts Organization"), i18n("Icon for Krusader"), QStringLiteral("mail@usefularts.org"), 0);
    aboutData.addCredit(i18n("Gábor Lehel"), i18n("Viewer module for 3rd Hand"), QStringLiteral("illissius@gmail.com"), 0);
    aboutData.addCredit(i18n("Mark Eatough"), i18n("Handbook Proof-Reader"), QStringLiteral("markeatough@yahoo.com"), 0);
    aboutData.addCredit(i18n("Jan Halasa"), i18n("The old Bookmark Module"), QStringLiteral("xhalasa@fi.muni.cz"), 0);
    aboutData.addCredit(i18n("Hans Löffler"), i18n("Dir history button"), 0, 0);
    aboutData.addCredit(i18n("Szombathelyi György"), i18n("ISO KIO slave"), 0, 0);
    aboutData.addCredit(i18n("Jan Willem van de Meent (Adios)"), i18n("Icons for Krusader"), QStringLiteral("janwillem@lorentz.leidenuniv.nl"), 0);
    aboutData.addCredit(i18n("Mikolaj Machowski"), i18n("Usability and QA"), QStringLiteral("<mikmach@wp.pl>"), 0);
    aboutData.addCredit(i18n("Cristi Dumitrescu"), i18n("QA, bug-hunting, patches and general help"), QStringLiteral("cristid@chip.ro"), 0);
    aboutData.addCredit(i18n("Aurelien Gateau"), i18n("patch for KViewer"), QStringLiteral("aurelien.gateau@free.fr"), 0);
    aboutData.addCredit(i18n("Milan Brabec"), i18n("the first patch ever!"), QStringLiteral("mbrabec@volny.cz"), 0);
    aboutData.addCredit(i18n("Asim Husanovic"), i18n("Bosnian translation"), QStringLiteral("asim@megatel.ba"), 0);
    aboutData.addCredit(i18n("Doutor Zero"), i18n("Brazilian Portuguese translation"), QStringLiteral("doutor.zero@gmail.com"), 0);
    aboutData.addCredit(i18n("Milen Ivanov"), i18n("Bulgarian translation"), QStringLiteral("milen.ivanov@abv.bg"), 0);
    aboutData.addCredit(i18n("Quim Perez"), i18n("Catalan translation"), QStringLiteral("noguer@osona.com"), 0);
    aboutData.addCredit(i18n("Jinghua Luo"), i18n("Chinese Simplified translation"), QStringLiteral("luojinghua@msn.com"), 0);
    aboutData.addCredit(i18n("Mitek"), i18n("Old Czech translation"), QStringLiteral("mitek@email.cz"), 0);
    aboutData.addCredit(i18n("Martin Sixta"), i18n("Old Czech translation"), QStringLiteral("lukumo84@seznam.cz"), 0);
    aboutData.addCredit(i18n("Vaclav Jůza"), i18n("Czech translation"), QStringLiteral("VaclavJuza@gmail.com"), 0);
    aboutData.addCredit(i18n("Anders Bruun Olsen"), i18n("Old Danish translation"), QStringLiteral("anders@bruun-olsen.net"), 0);
    aboutData.addCredit(i18n("Peter H. Sorensen"), i18n("Danish translation"), QStringLiteral("peters@skydebanen.net"), 0);
    aboutData.addCredit(i18n("Frank Schoolmeesters"), i18n("Dutch translation"), QStringLiteral("frank_schoolmeesters@yahoo.com"), 0);
    aboutData.addCredit(i18n("Rene-Pierre Lehmann"), i18n("Old French translation"), QStringLiteral("ripi@lepi.org"), 0);
    aboutData.addCredit(i18n("David Guillerm"), i18n("French translation"), QStringLiteral("dguillerm@gmail.com"), 0);
    aboutData.addCredit(i18n("Christoph Thielecke"), i18n("Old German translation"), QStringLiteral("crissi99@gmx.de"), 0);
    aboutData.addCredit(i18n("Dirk Eschler"), i18n("German translation"), QStringLiteral("deschler@users.sourceforge.net"), 0);
    aboutData.addCredit(i18n("Spiros Georgaras"), i18n("Greek translation"), QStringLiteral("sngeorgaras@gmail.com"), 0);
    aboutData.addCredit(i18n("Kukk Zoltan"), i18n("Old Hungarian translation"), QStringLiteral("kukkzoli@freemail.hu"), 0);
    aboutData.addCredit(i18n("Arpad Biro"), i18n("Hungarian translation"), QStringLiteral("biro_arpad@yahoo.com"), 0);
    aboutData.addCredit(i18n("Giuseppe Bordoni"), i18n("Italian translation"), QStringLiteral("geppo@geppozone.com"), 0);
    aboutData.addCredit(i18n("Hideki Kimura"), i18n("Japanese translation"), QStringLiteral("hangyo1973@gmail.com"), 0);
    aboutData.addCredit(i18n("UTUMI Hirosi"), i18n("Old Japanese translation"), QStringLiteral("utuhiro@mx12.freecom.ne.jp"), 0);
    aboutData.addCredit(i18n("Dovydas Sankauskas"), i18n("Lithuanian translation"), QStringLiteral("laisve@gmail.com"), 0);
    aboutData.addCredit(i18n("Bruno Queiros"), i18n("Portuguese translation"), QStringLiteral("brunoqueiros@portugalmail.com"), 0);
    aboutData.addCredit(i18n("Lukasz Janyst"), i18n("Old Polish translation"), QStringLiteral("ljan@wp.pl"), 0);
    aboutData.addCredit(i18n("Pawel Salawa"), i18n("Polish translation"), QStringLiteral("boogie@myslenice.one.pl"), 0);
    aboutData.addCredit(i18n("Tomek Grzejszczyk"), i18n("Polish translation"), QStringLiteral("tgrzej@onet.eu"), 0);
    aboutData.addCredit(i18n("Dmitry A. Bugay"), i18n("Russian translation"), QStringLiteral("sam@vhnet.ru"), 0);
    aboutData.addCredit(i18n("Dmitry Chernyak"), i18n("Old Russian translation"), QStringLiteral("chernyak@mail.ru"), 0);
    aboutData.addCredit(i18n("Sasa Tomic"), i18n("Serbian translation"), QStringLiteral("stomic@gmx.net"), 0);
    aboutData.addCredit(i18n("Zdenko Podobný and Ondrej Pačay (Yogi)"), i18n("Slovak translation"), QStringLiteral("zdenop@gmail.com"), 0);
    aboutData.addCredit(i18n("Matej Urbancic"), i18n("Slovenian translation"), QStringLiteral("matej.urban@gmail.com"), 0);
    aboutData.addCredit(i18n("Rafael Munoz"), i18n("Old Spanish translation"), QStringLiteral("muror@hotpop.com"), 0);
    aboutData.addCredit(i18n("Alejandro Araiza Alvarado"), i18n("Spanish translation"), QStringLiteral("mebrelith@gmail.com"), 0);
    aboutData.addCredit(i18n("Erik Johanssen"), i18n("Old Swedish translation"), QStringLiteral("erre@telia.com"), 0);
    aboutData.addCredit(i18n("Anders Linden"), i18n("Old Swedish translation"), QStringLiteral("connyosis@gmx.net"), 0);
    aboutData.addCredit(i18n("Peter Landgren"), i18n("Swedish translation"), QStringLiteral("peter.talken@telia.com"), 0);
    aboutData.addCredit(i18n("Bekir Sonat"), i18n("Turkish translation"), QStringLiteral("bekirsonat@kde.org.tr"), 0);
    aboutData.addCredit(i18n("Ivan Petrouchtchak"), i18n("Ukrainian translation"), QStringLiteral("connyosis@gmx.net"), 0);
    aboutData.addCredit(i18n("Seongnam Jee"), i18n("Korean translation"), QStringLiteral("snjee@intellicam.com"), 0);

    // This will call QCoreApplication::setApplicationName, etc for us by using info in the KAboutData instance.
    // The only thing not called for us is setWindowIcon(), which is why we do it ourselves here.
    KAboutData::setApplicationData(aboutData);
    app.setWindowIcon(QIcon::fromTheme(Krusader::privIcon()));

    // Command line arguments ...
    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("left"), i18n("Start left panel at <path>"), QLatin1String("path")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("right"), i18n("Start right panel at <path>"), QLatin1String("path")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("profile"), i18n("Load this profile on startup"), QLatin1String("panel-profile")));
    parser.addOption(QCommandLineOption(QStringList() << "d" << QLatin1String("debug"), i18n("Enable debug output")));
    parser.addPositionalArgument(QLatin1String("url"), i18n("URL to open"));

    // check for command line arguments
    parser.process(app);
    aboutData.processCommandLine(&parser);

    // set global message handler
    KrServices::setGlobalKrMessageHandler(parser.isSet("debug"));

    KConfigGroup cfg(KSharedConfig::openConfig(), QStringLiteral("Look&Feel"));
    bool singleInstanceMode = cfg.readEntry("Single Instance Mode", _SingleInstanceMode);

    QString url;
    if(!parser.positionalArguments().isEmpty()) {
        url = parser.positionalArguments().first();
    }

    QString appName = "krusader";
    if (!singleInstanceMode)
        appName += QString("%1").arg(getpid());

    if (!QDBusConnection::sessionBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-BUS session bus.\n"
                "To start it, run:\n"
                "\teval `dbus-launch --auto-syntax`\n");
    }

    if (singleInstanceMode) {
        QDBusInterface remoteApp("org.krusader", "/Instances/" + appName,
                                "org.krusader.Instance", QDBusConnection::sessionBus());
        QDBusReply<bool> reply;
        if (remoteApp.isValid())
            reply = remoteApp.call("isRunning");

        if (!reply.isValid() && reply.error().type() != QDBusError::ServiceUnknown &&
                reply.error().type() != QDBusError::UnknownObject)
            fprintf(stderr, "DBus Error: %s, %s\n", reply.error().name().toLocal8Bit().constData(), reply.error().message().toLocal8Bit().constData());

        if (reply.isValid() && (bool)reply) {
            KStartupInfo::appStarted();
            QStringList tabs;
            if (parser.isSet("left"))
                openTabsRemote(parser.value("left").split(','), true, appName);
            if (parser.isSet("right"))
                openTabsRemote(parser.value("right").split(','), false, appName);
            if(!url.isEmpty()) {
                reply = remoteApp.call("openUrl", url);
                if (!reply.isValid())
                    fprintf(stderr, "DBus Error: %s, %s\n", reply.error().name().toLocal8Bit().constData(), reply.error().message().toLocal8Bit().constData());
            }
            return 0;
        }
    }

    // splash screen - if the user wants one
    QSplashScreen *splash = 0;
    { // don't remove bracket
        KConfigGroup cfg(KSharedConfig::openConfig(), QStringLiteral("Look&Feel"));
        if (cfg.readEntry("Show splashscreen", _ShowSplashScreen)) {
            QString splashFilename = QStandardPaths::locate(QStandardPaths::DataLocation, QStringLiteral("splash.png"));
            QPixmap pixmap(splashFilename);
            if (!pixmap.isNull()) {
                splash = new QSplashScreen(pixmap);
                splash->show();
            }
        }
    } // don't remove bracket

    Krusader::AppName = appName;
    Krusader *krusader = new Krusader(parser);

    if(!url.isEmpty())
        krusader->openUrl(url);

    QDBusConnection dbus = QDBusConnection::sessionBus();
    if (!dbus.interface()->isServiceRegistered("org.krusader") && !dbus.registerService("org.krusader")) {
        fprintf(stderr, "DBus Error: %s, %s\n", dbus.lastError().name().toLocal8Bit().constData(), dbus.lastError().message().toLocal8Bit().constData());
    }
    if (!dbus.registerObject("/Instances/" + appName, krusader, QDBusConnection::ExportScriptableSlots)) {
        fprintf(stderr, "DBus Error: %s, %s\n", dbus.lastError().name().toLocal8Bit().constData(), dbus.lastError().message().toLocal8Bit().constData());
    }
    if (!dbus.registerObject("/Instances/" + appName + "/left_manager", LEFT_MNG, QDBusConnection::ExportScriptableSlots)) {
        fprintf(stderr, "DBus Error: %s, %s\n", dbus.lastError().name().toLocal8Bit().constData(), dbus.lastError().message().toLocal8Bit().constData());
    }
    if (!dbus.registerObject("/Instances/" + appName + "/right_manager", RIGHT_MNG, QDBusConnection::ExportScriptableSlots)) {
        fprintf(stderr, "DBus Error: %s, %s\n", dbus.lastError().name().toLocal8Bit().constData(), dbus.lastError().message().toLocal8Bit().constData());
    }



    // catching SIGTERM, SIGHUP, SIGQUIT
    signal(SIGTERM, sigterm_handler);
    signal(SIGPIPE, sigterm_handler);
    signal(SIGHUP, sigterm_handler);

    QObject::connect(&app, &QGuiApplication::applicationStateChanged, SLOTS, &KRslots::applicationStateChanged);

    // hide splashscreen
    if (splash) {
        splash->finish(krusader);
        delete splash;
    }
    // let's go.
    return app.exec();

}
