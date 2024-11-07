/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <csignal>
#include <unistd.h>

// QtCore
#include <QAbstractEventDispatcher>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QStandardPaths>
// QtGui
#include <QPixmap>
// QtDBus
#include <QDBusConnectionInterface>
#include <QDBusInterface>
// QtWidgets
#include <QApplication>
#include <QSplashScreen>

#include <KAboutData>
#include <KCrash>
#include <KLazyLocalizedString>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KStartupInfo>

#include "../Archive/krarchandler.h"

#include "defaults.h"
#include "icon.h"
#include "krservices.h"
#include "krslots.h"
#include "krusader.h"
#include "krusaderversion.h"
#include "krusaderview.h"
#include "panelmanager.h"

static const KLazyLocalizedString description = kli18n("Krusader\nTwin-Panel File Manager by KDE");

static void handleCrash(const int signal)
{
    fprintf(stderr, "Krusader crashed with signal %d\n", signal);

    Krusader::emergencySaveSettings();
}

static void handleExitSignal(const int signal)
{
    fprintf(stderr, "Krusader received signal: %d\n", signal);

    Krusader::emergencySaveSettings();

    QAbstractEventDispatcher *instance = QAbstractEventDispatcher::instance();
    if (instance)
        instance->wakeUp();
    QApplication::exit(-15);
}

void openTabsRemote(QStringList tabs, bool left, const QString &appName)
{
    // make sure left or right are not relative paths
    for (int i = 0; i != tabs.count(); i++) {
        tabs[i] = tabs[i].trimmed();
        if (!tabs[i].startsWith('/') && tabs[i].indexOf(":/") < 0)
            tabs[i] = QDir::currentPath() + '/' + tabs[i];
    }

    QDBusInterface remoteApp("org.krusader",
                             "/Instances/" + appName + (left ? "/left_manager" : "/right_manager"),
                             "org.krusader.PanelManager",
                             QDBusConnection::sessionBus());
    QDBusReply<void> reply;
    if (remoteApp.isValid())
        reply = remoteApp.call("newTabs", tabs);

    if (!reply.isValid())
        fprintf(stderr, "DBus Error: %s, %s\n", reply.error().name().toLocal8Bit().constData(), reply.error().message().toLocal8Bit().constData());
}

int main(int argc, char *argv[])
{
    // set global log message format
    qSetMessagePattern(KrServices::GLOBAL_MESSAGE_PATTERN);

    // prevent qt5-webengine crashing
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    // create the application and set application domain so that calls to i18n get strings from right place.
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("krusader");

    // init icon theme
    qDebug() << "System icon theme:" << QIcon::themeName();
    // [WORKAROUND] setThemeName sets user theme in QIconLoader and allows to avoid Qt issues with invalid icon caching later
    // IMPORTANT: this must be done before the first QIcon::fromTheme / QIcon::hasThemeIcon call
    QIcon::setThemeName(QIcon::themeName());

    // ABOUT data information
#ifdef RELEASE_NAME
    QString versionName = QStringLiteral("%1 \"%2\"").arg(VERSION, RELEASE_NAME);
#else
    QString versionName = VERSION;
#endif

#ifdef GIT_REVISION
    versionName += QString(" (gitrev: %1)").arg(GIT_REVISION);
#endif

    KAboutData aboutData(QStringLiteral("krusader"),
                         (geteuid() ? i18n("Krusader") : i18n("Krusader - ROOT PRIVILEGES")),
                         versionName,
                         KLocalizedString(description).toString(),
                         KAboutLicense::GPL_V2,
                         i18n("© 2000-2003 Shie Erlich, Rafi Yanai\n© 2004-2022 Krusader Krew"),
                         i18n("Feedback:\nhttps://forum.kde.org/viewforum.php?f=225"),
                         QStringLiteral("https://krusader.org"));

    aboutData.setOrganizationDomain(QByteArray("kde.org"));
    aboutData.setDesktopFileName(QStringLiteral("org.kde.krusader"));

    aboutData.addAuthor(i18n("Davide Gianforte"), i18n("Developer"), QStringLiteral("davide@gengisdave.org"), nullptr);
    aboutData.addAuthor(i18n("Toni Asensi Esteve"), i18n("Developer"), QStringLiteral("toni.asensi@kdemail.net"), nullptr);
    aboutData.addAuthor(i18n("Nikita Melnichenko"), i18n("Developer"), QStringLiteral("nikita+kde@melnichenko.name"), nullptr);
    aboutData.addAuthor(i18n("Yuri Chornoivan"), i18n("Documentation"), QStringLiteral("yurchor@ukr.net"), nullptr);
    aboutData.addAuthor(i18n("Rafi Yanai"), i18n("Author (retired)"), QStringLiteral("yanai@users.sourceforge.net"));
    aboutData.addAuthor(i18n("Shie Erlich"), i18n("Author (retired)"), QStringLiteral("erlich@users.sourceforge.net"));
    aboutData.addAuthor(i18n("Csaba Karai"), i18n("Developer (retired)"), QStringLiteral("ckarai@users.sourceforge.net"), nullptr);
    aboutData.addAuthor(i18n("Heiner Eichmann"), i18n("Developer (retired)"), QStringLiteral("h.eichmann@gmx.de"), nullptr);
    aboutData.addAuthor(i18n("Jonas Bähr"), i18n("Developer (retired)"), QStringLiteral("jonas.baehr@web.de"), nullptr);
    aboutData.addAuthor(i18n("Václav Jůza"), i18n("Developer (retired)"), QStringLiteral("vaclavjuza@gmail.com"), nullptr);
    aboutData.addAuthor(i18n("Jan Lepper"), i18n("Developer (retired)"), QStringLiteral("jan_lepper@gmx.de"), nullptr);
    aboutData.addAuthor(i18n("Andrey Matveyakin"), i18n("Developer (retired)"), QStringLiteral("a.matveyakin@gmail.com"), nullptr);
    aboutData.addAuthor(i18n("Simon Persson"), i18n("Developer (retired)"), QStringLiteral("simon.persson@mykolab.com"), nullptr);
    aboutData.addAuthor(i18n("Alexander Bikadorov"), i18n("Developer (retired)"), QStringLiteral("alex.bikadorov@kdemail.net"), nullptr);
    aboutData.addAuthor(i18n("Martin Kostolný"), i18n("Developer (retired)"), QStringLiteral("clearmartin@gmail.com"), nullptr);
    aboutData.addAuthor(i18n("Dirk Eschler"), i18n("Webmaster (retired)"), QStringLiteral("deschler@users.sourceforge.net"), nullptr);
    aboutData.addAuthor(i18n("Frank Schoolmeesters"),
                        i18n("Documentation and marketing coordinator (retired)"),
                        QStringLiteral("frank_schoolmeesters@yahoo.com"),
                        nullptr);
    aboutData.addAuthor(i18n("Richard Holt"), i18n("Documentation & Proofing (retired)"), QStringLiteral("richard.holt@gmail.com"), nullptr);
    aboutData.addAuthor(i18n("Matej Urbancic"), i18n("Marketing & Product Research (retired)"), QStringLiteral("matej.urban@gmail.com"), nullptr);
    aboutData.addCredit(i18n("kde.org"), i18n("Everyone involved in KDE"), nullptr, nullptr);
    aboutData.addCredit(i18n("l10n.kde.org"), i18n("KDE Translation Teams"), nullptr, nullptr);
    aboutData.addCredit(i18n("Jiří Paleček"), i18n("QA, bug-hunting, patches and general help"), QStringLiteral("jpalecek@web.de"), nullptr);
    aboutData.addCredit(i18n("Jiří Klement"), i18n("Important help in KDE 4 porting"), nullptr, nullptr);
    aboutData.addCredit(i18n("Andrew Neupokoev"), i18n("Killer Logo and Icons for Krusader (contest winner)"), QStringLiteral("doom-blue@yandex.ru"), nullptr);
    aboutData.addCredit(i18n("The UsefulArts Organization"), i18n("Icon for Krusader"), QStringLiteral("mail@usefularts.org"), nullptr);
    aboutData.addCredit(i18n("Gábor Lehel"), i18n("Viewer module for 3rd Hand"), QStringLiteral("illissius@gmail.com"), nullptr);
    aboutData.addCredit(i18n("Mark Eatough"), i18n("Handbook Proof-Reader"), QStringLiteral("markeatough@yahoo.com"), nullptr);
    aboutData.addCredit(i18n("Jan Halasa"), i18n("The old Bookmark Module"), QStringLiteral("xhalasa@fi.muni.cz"), nullptr);
    aboutData.addCredit(i18n("Hans Löffler"), i18n("Dir history button"), nullptr, nullptr);
    aboutData.addCredit(i18n("Szombathelyi György"), i18n("ISO KIO slave"), nullptr, nullptr);
    aboutData.addCredit(i18n("Jan Willem van de Meent (Adios)"), i18n("Icons for Krusader"), QStringLiteral("janwillem@lorentz.leidenuniv.nl"), nullptr);
    aboutData.addCredit(i18n("Mikolaj Machowski"), i18n("Usability and QA"), QStringLiteral("<mikmach@wp.pl>"), nullptr);
    aboutData.addCredit(i18n("Cristi Dumitrescu"), i18n("QA, bug-hunting, patches and general help"), QStringLiteral("cristid@chip.ro"), nullptr);
    aboutData.addCredit(i18n("Aurelien Gateau"), i18n("patch for KViewer"), QStringLiteral("aurelien.gateau@free.fr"), nullptr);
    aboutData.addCredit(i18n("Milan Brabec"), i18n("the first patch ever!"), QStringLiteral("mbrabec@volny.cz"), nullptr);
    aboutData.addCredit(i18n("Asim Husanovic"), i18n("Bosnian translation"), QStringLiteral("asim@megatel.ba"), nullptr);
    aboutData.addCredit(i18n("Doutor Zero"), i18n("Brazilian Portuguese translation"), QStringLiteral("doutor.zero@gmail.com"), nullptr);
    aboutData.addCredit(i18n("Milen Ivanov"), i18n("Bulgarian translation"), QStringLiteral("milen.ivanov@abv.bg"), nullptr);
    aboutData.addCredit(i18n("Quim Perez"), i18n("Catalan translation"), QStringLiteral("noguer@osona.com"), nullptr);
    aboutData.addCredit(i18n("Jinghua Luo"), i18n("Chinese Simplified translation"), QStringLiteral("luojinghua@msn.com"), nullptr);
    aboutData.addCredit(i18n("Mitek"), i18n("Old Czech translation"), QStringLiteral("mitek@email.cz"), nullptr);
    aboutData.addCredit(i18n("Martin Sixta"), i18n("Old Czech translation"), QStringLiteral("lukumo84@seznam.cz"), nullptr);
    aboutData.addCredit(i18n("Vaclav Jůza"), i18n("Czech translation"), QStringLiteral("VaclavJuza@gmail.com"), nullptr);
    aboutData.addCredit(i18n("Anders Bruun Olsen"), i18n("Old Danish translation"), QStringLiteral("anders@bruun-olsen.net"), nullptr);
    aboutData.addCredit(i18n("Peter H. Sorensen"), i18n("Danish translation"), QStringLiteral("peters@skydebanen.net"), nullptr);
    aboutData.addCredit(i18n("Frank Schoolmeesters"), i18n("Dutch translation"), QStringLiteral("frank_schoolmeesters@yahoo.com"), nullptr);
    aboutData.addCredit(i18n("Rene-Pierre Lehmann"), i18n("Old French translation"), QStringLiteral("ripi@lepi.org"), nullptr);
    aboutData.addCredit(i18n("David Guillerm"), i18n("French translation"), QStringLiteral("dguillerm@gmail.com"), nullptr);
    aboutData.addCredit(i18n("Christoph Thielecke"), i18n("Old German translation"), QStringLiteral("crissi99@gmx.de"), nullptr);
    aboutData.addCredit(i18n("Dirk Eschler"), i18n("German translation"), QStringLiteral("deschler@users.sourceforge.net"), nullptr);
    aboutData.addCredit(i18n("Spiros Georgaras"), i18n("Greek translation"), QStringLiteral("sngeorgaras@gmail.com"), nullptr);
    aboutData.addCredit(i18n("Kukk Zoltan"), i18n("Old Hungarian translation"), QStringLiteral("kukkzoli@freemail.hu"), nullptr);
    aboutData.addCredit(i18n("Arpad Biro"), i18n("Hungarian translation"), QStringLiteral("biro_arpad@yahoo.com"), nullptr);
    aboutData.addCredit(i18n("Giuseppe Bordoni"), i18n("Italian translation"), QStringLiteral("geppo@geppozone.com"), nullptr);
    aboutData.addCredit(i18n("Hideki Kimura"), i18n("Japanese translation"), QStringLiteral("hangyo1973@gmail.com"), nullptr);
    aboutData.addCredit(i18n("UTUMI Hirosi"), i18n("Old Japanese translation"), QStringLiteral("utuhiro@mx12.freecom.ne.jp"), nullptr);
    aboutData.addCredit(i18n("Dovydas Sankauskas"), i18n("Lithuanian translation"), QStringLiteral("laisve@gmail.com"), nullptr);
    aboutData.addCredit(i18n("Bruno Queiros"), i18n("Portuguese translation"), QStringLiteral("brunoqueiros@portugalmail.com"), nullptr);
    aboutData.addCredit(i18n("Lukasz Janyst"), i18n("Old Polish translation"), QStringLiteral("ljan@wp.pl"), nullptr);
    aboutData.addCredit(i18n("Pawel Salawa"), i18n("Polish translation"), QStringLiteral("boogie@myslenice.one.pl"), nullptr);
    aboutData.addCredit(i18n("Tomek Grzejszczyk"), i18n("Polish translation"), QStringLiteral("tgrzej@onet.eu"), nullptr);
    aboutData.addCredit(i18n("Dmitry A. Bugay"), i18n("Russian translation"), QStringLiteral("sam@vhnet.ru"), nullptr);
    aboutData.addCredit(i18n("Dmitry Chernyak"), i18n("Old Russian translation"), QStringLiteral("chernyak@mail.ru"), nullptr);
    aboutData.addCredit(i18n("Sasa Tomic"), i18n("Serbian translation"), QStringLiteral("stomic@gmx.net"), nullptr);
    aboutData.addCredit(i18n("Zdenko Podobný and Ondrej Pačay (Yogi)"), i18n("Slovak translation"), QStringLiteral("zdenop@gmail.com"), nullptr);
    aboutData.addCredit(i18n("Matej Urbancic"), i18n("Slovenian translation"), QStringLiteral("matej.urban@gmail.com"), nullptr);
    aboutData.addCredit(i18n("Rafael Munoz"), i18n("Old Spanish translation"), QStringLiteral("muror@hotpop.com"), nullptr);
    aboutData.addCredit(i18n("Alejandro Araiza Alvarado"), i18n("Spanish translation"), QStringLiteral("mebrelith@gmail.com"), nullptr);
    aboutData.addCredit(i18n("Erik Johanssen"), i18n("Old Swedish translation"), QStringLiteral("erre@telia.com"), nullptr);
    aboutData.addCredit(i18n("Anders Linden"), i18n("Old Swedish translation"), QStringLiteral("connyosis@gmx.net"), nullptr);
    aboutData.addCredit(i18n("Peter Landgren"), i18n("Swedish translation"), QStringLiteral("peter.talken@telia.com"), nullptr);
    aboutData.addCredit(i18n("Bekir Sonat"), i18n("Turkish translation"), QStringLiteral("bekirsonat@kde.org.tr"), nullptr);
    aboutData.addCredit(i18n("Ivan Petrouchtchak"), i18n("Ukrainian translation"), QStringLiteral("connyosis@gmx.net"), nullptr);
    aboutData.addCredit(i18n("Seongnam Jee"), i18n("Korean translation"), QStringLiteral("snjee@intellicam.com"), nullptr);

    // This will call QCoreApplication::setApplicationName, etc for us by using info in the KAboutData instance.
    // The only thing not called for us is setWindowIcon(), which is why we do it ourselves here.
    KAboutData::setApplicationData(aboutData);
    app.setWindowIcon(Icon(Krusader::appIconName()));

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

    // handle crashes
    KCrash::initialize();
    KCrash::setEmergencySaveFunction(handleCrash);

    // set global message handler
    KrServices::setGlobalKrMessageHandler(parser.isSet("debug"));

    KConfigGroup cfg(KSharedConfig::openConfig(), QStringLiteral("Look&Feel"));
    bool singleInstanceMode = cfg.readEntry("Single Instance Mode", _SingleInstanceMode);

    QString url;
    if (!parser.positionalArguments().isEmpty()) {
        url = parser.positionalArguments().first();
    }

    QString appName = "krusader";
    if (!singleInstanceMode)
        appName += QString("%1").arg(getpid());

    if (!QDBusConnection::sessionBus().isConnected()) {
        fprintf(stderr,
                "Cannot connect to the D-BUS session bus.\n"
                "To start it, run:\n"
                "\teval `dbus-launch --auto-syntax`\n");
    }

    if (singleInstanceMode) {
        QDBusInterface remoteApp("org.krusader", "/Instances/" + appName, "org.krusader.Instance", QDBusConnection::sessionBus());
        QDBusReply<bool> reply;
        if (remoteApp.isValid())
            reply = remoteApp.call("isRunning");

        if (!reply.isValid() && reply.error().type() != QDBusError::ServiceUnknown && reply.error().type() != QDBusError::UnknownObject)
            fprintf(stderr, "DBus Error: %s, %s\n", reply.error().name().toLocal8Bit().constData(), reply.error().message().toLocal8Bit().constData());

        if (reply.isValid() && (bool)reply) {
            KStartupInfo::appStarted();
            if (parser.isSet("left"))
                openTabsRemote(parser.value("left").split(','), true, appName);
            if (parser.isSet("right"))
                openTabsRemote(parser.value("right").split(','), false, appName);
            if (!url.isEmpty()) {
                reply = remoteApp.call("openUrl", url);
                if (!reply.isValid())
                    fprintf(stderr, "DBus Error: %s, %s\n", reply.error().name().toLocal8Bit().constData(), reply.error().message().toLocal8Bit().constData());
            }
            return 0;
        }
    }

    // splash screen - if the user wants one
    QSplashScreen *splash = nullptr;
    { // don't remove bracket
        KConfigGroup cfg(KSharedConfig::openConfig(), QStringLiteral("Look&Feel"));
        if (cfg.readEntry("Show splashscreen", _ShowSplashScreen)) {
            QString splashFilename = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("splash.png"));
            QPixmap pixmap(splashFilename);
            if (!pixmap.isNull()) {
                splash = new QSplashScreen(pixmap);
                splash->show();
            }
        }
    } // don't remove bracket

    Krusader::AppName = appName;
    auto *krusader = new Krusader(parser);

    if (!url.isEmpty())
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

    // catching SIGTERM, SIGHUP, SIGINT
    std::signal(SIGTERM, handleExitSignal);
    std::signal(SIGHUP, handleExitSignal);
    std::signal(SIGINT, handleExitSignal);

    QObject::connect(&app, &QGuiApplication::applicationStateChanged, SLOTS, &KrSlots::applicationStateChanged);

    // hide splashscreen
    if (splash) {
        splash->finish(krusader);
        delete splash;
    }
    // let's go.
    return app.exec();
}
