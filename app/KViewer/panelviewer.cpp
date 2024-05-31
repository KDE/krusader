/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "panelviewer.h"

// QtCore
#include <QDebug>
#include <QFile>
// QtWidgets
#include <QApplication>

#include <kservice_version.h>

#if KSERVICE_VERSION >= QT_VERSION_CHECK(5, 82, 0)
#define CREATE_KPART_5_82 1
#else
#define CREATE_KPART_5_82 0
#endif

#include <KLocalizedString>
#include <KParts/NavigationExtension>
#include <KParts/ReadWritePart>
#include <KSharedConfig>

#if CREATE_KPART_5_82
#include <KParts/PartLoader>
#endif

#include <KFileItem>
#include <KMessageBox>
#include <KMimeTypeTrader>
#include <KServiceTypeProfile>

#include "../defaults.h"
#include "lister.h"

#define DICTSIZE 211

PanelViewerBase::PanelViewerBase(QWidget *parent, KrViewer::Mode mode)
    : QStackedWidget(parent)
    , mimes(nullptr)
    , cpart(nullptr)
    , mode(mode)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored));

    mimes = new QHash<QString, QPointer<KParts::ReadOnlyPart>>();
    cpart = nullptr;
    // NOTE: the fallback should be never visible. The viewer is not opened without a file and the
    // tab is closed if a file cannot be opened.
    fallback = new QLabel(i18n("No file selected or selected file cannot be displayed."), this);
    fallback->setAlignment(Qt::Alignment(QFlag(Qt::AlignCenter | Qt::TextExpandTabs)));
    fallback->setWordWrap(true);
    addWidget(fallback);
    setCurrentWidget(fallback);
}

PanelViewerBase::~PanelViewerBase()
{
    // cpart->queryClose();
    closeUrl();
    QHashIterator<QString, QPointer<KParts::ReadOnlyPart>> lit(*mimes);
    while (lit.hasNext()) {
        QPointer<KParts::ReadOnlyPart> p = lit.next().value();
        if (p)
            delete p;
    }
    mimes->clear();
    delete mimes;
    delete fallback;
}

void PanelViewerBase::slotStatResult(KJob *job)
{
    if (job->error()) {
        KMessageBox::error(this, job->errorString());
        emit openUrlFinished(this, false);
    } else {
        KIO::UDSEntry entry = dynamic_cast<KIO::StatJob *>(job)->statResult();
        openFile(KFileItem(entry, curl));
    }
}

KParts::ReadOnlyPart *PanelViewerBase::getPart(const QString &mimetype)
{
    KParts::ReadOnlyPart *part = nullptr;

    if (mimes->find(mimetype) == mimes->end()) {
        part = createPart(mimetype);
        if (part)
            mimes->insert(mimetype, part);
    } else
        part = (*mimes)[mimetype];

    return part;
}

void PanelViewerBase::openUrl(const QUrl &url)
{
    closeUrl();
    curl = url;
    emit urlChanged(this, url);

    if (url.isLocalFile()) {
        if (!QFile::exists(url.path())) {
            KMessageBox::error(krMainWindow, i18n("Error at opening %1.", url.path()));
            emit openUrlFinished(this, false);
            return;
        }
        openFile(KFileItem(url));
    } else {
        KIO::StatJob *statJob = KIO::stat(url, KIO::HideProgressInfo);
        connect(statJob, &KIO::StatJob::result, this, &PanelViewerBase::slotStatResult);
    }
}

/* ----==={ PanelViewer }===---- */

PanelViewer::PanelViewer(QWidget *parent, KrViewer::Mode mode)
    : PanelViewerBase(parent, mode)
{
}

PanelViewer::~PanelViewer() = default;

KParts::ReadOnlyPart *PanelViewer::getListerPart(bool hexMode)
{
    KParts::ReadOnlyPart *part = nullptr;

    if (mimes->find(QLatin1String("krusader_lister")) == mimes->end()) {
        part = new Lister(this);
        mimes->insert(QLatin1String("krusader_lister"), part);
    } else
        part = (*mimes)[QLatin1String("krusader_lister")];

    if (part) {
        auto *lister = qobject_cast<Lister *>((KParts::ReadOnlyPart *)part);
        if (lister)
            lister->setHexMode(hexMode);
    }
    return part;
}

KParts::ReadOnlyPart *PanelViewer::getHexPart()
{
    KParts::ReadOnlyPart *part = nullptr;

    if (KConfigGroup(krConfig, "General").readEntry("UseOktetaViewer", _UseOktetaViewer)) {
        if (mimes->find("oktetapart") == mimes->end()) {
            KPluginFactory *factory = nullptr;
            // Okteta >= 0.26 provides a desktop file, prefer that as the binary changes name
            KService::Ptr service = KService::serviceByDesktopName("oktetapart");
            if (service) {
                KPluginName name = *service.data();
                factory = KPluginFactory::loadFactory(name.name()).plugin;
            } else {
                // fallback to search for desktopfile-less old variant
                factory = KPluginFactory::loadFactory(QStringLiteral("oktetapart")).plugin;
            }
            if (factory) {
                if ((part = factory->create<KParts::ReadOnlyPart>(this, this)))
                    mimes->insert("oktetapart", part);
            }
        } else
            part = (*mimes)["oktetapart"];
    }

    return part ? part : getListerPart(true);
}

KParts::ReadOnlyPart *PanelViewer::getTextPart()
{
    KParts::ReadOnlyPart *part = getPart("text/plain");
    if (!part)
        part = getPart("all/allfiles");
    return part ? part : getListerPart();
}

KParts::ReadOnlyPart *PanelViewer::getDefaultPart(const KFileItem &fi)
{
    KConfigGroup group(krConfig, "General");
    QString modeString = group.readEntry("Default Viewer Mode", QString("generic"));

    KrViewer::Mode mode = KrViewer::Generic;

    if (modeString == "generic")
        mode = KrViewer::Generic;
    else if (modeString == "text")
        mode = KrViewer::Text;
    else if (modeString == "hex")
        mode = KrViewer::Hex;
    else if (modeString == "lister")
        mode = KrViewer::Lister;

    QMimeType mimeType = fi.determineMimeType();
    bool isBinary = false;
    if (mode == KrViewer::Generic || mode == KrViewer::Lister) {
        isBinary = !mimeType.inherits(QStringLiteral("text/plain"));
    }

    KIO::filesize_t fileSize = fi.size();
    KIO::filesize_t limit = (KIO::filesize_t)group.readEntry("Lister Limit", _ListerLimit) * 0x100000;

    QString mimetype = fi.mimetype();

    KParts::ReadOnlyPart *part = nullptr;

    switch (mode) {
    case KrViewer::Generic:
        if ((mimetype.startsWith(QLatin1String("text/")) || mimetype.startsWith(QLatin1String("all/"))) && fileSize > limit) {
            part = getListerPart(isBinary);
            break;
        } else if ((part = getPart(mimetype))) {
            break;
        }
#if __GNUC__ >= 7
        [[gnu::fallthrough]];
#endif
    case KrViewer::Text:
        part = fileSize > limit ? getListerPart(false) : getTextPart();
        break;
    case KrViewer::Lister:
        part = getListerPart(isBinary);
        break;
    case KrViewer::Hex:
        part = fileSize > limit ? getListerPart(true) : getHexPart();
        break;
    default:
        abort();
    }

    return part;
}

void PanelViewer::openFile(KFileItem fi)
{
    switch (mode) {
    case KrViewer::Generic:
        cpart = getPart(fi.mimetype());
        break;
    case KrViewer::Text:
        cpart = getTextPart();
        break;
    case KrViewer::Lister:
        cpart = getListerPart();
        break;
    case KrViewer::Hex:
        cpart = getHexPart();
        break;
    case KrViewer::Default:
        cpart = getDefaultPart(fi);
        break;
    default:
        abort();
    }

    if (cpart) {
        addWidget(cpart->widget());
        setCurrentWidget(cpart->widget());

        if (cpart->inherits("KParts::ReadWritePart"))
            dynamic_cast<KParts::ReadWritePart *>(cpart.data())->setReadWrite(false);
        KParts::OpenUrlArguments args;
        args.setReload(true);
        cpart->setArguments(args);

        connect(cpart.data(), &KParts::ReadOnlyPart::canceled, this, [=]() {
            qDebug() << "openFile canceled: '" << curl << "'";
        });

        auto cPartCompleted = [=]() {
            connect(cpart.data(), &KParts::ReadOnlyPart::destroyed, this, &PanelViewer::slotCPartDestroyed);
            emit openUrlFinished(this, true);
            qDebug() << "openFile completed: '" << curl << "'";
        };
        connect(cpart.data(), QOverload<>::of(&KParts::ReadOnlyPart::completed), this, cPartCompleted);
#if KSERVICE_VERSION >= QT_VERSION_CHECK(5, 81, 0)
        connect(cpart.data(), &KParts::ReadOnlyPart::completedWithPendingAction, this, cPartCompleted);
#else
        connect(cpart.data(), QOverload<bool>::of(&KParts::ReadOnlyPart::completed), this, cPartCompleted);
#endif

        // Note: Don't rely on return value of openUrl as the call is async in general
        cpart->openUrl(curl);

        return;
    }

    setCurrentWidget(fallback);
    emit openUrlFinished(this, false);
}

bool PanelViewer::closeUrl()
{
    setCurrentWidget(fallback);
    if (cpart && cpart->closeUrl()) {
        cpart = nullptr;
        return true;
    }
    return false;
}

#if CREATE_KPART_5_82

template<typename T>
static T *createKPartForMimeType(QString mimetype, QWidget *parentWidget)
{
    auto metaDataList = KParts::PartLoader::partsForMimeType(mimetype);
    if (metaDataList.isEmpty())
        return nullptr;

    KPluginFactory *pluginFactory = KPluginFactory::loadFactory(metaDataList.first()).plugin;

    if (!pluginFactory)
        return nullptr;

    return pluginFactory->create<T>(parentWidget, parentWidget);
}

#endif // CREATE_KPART_5_82

KParts::ReadOnlyPart *PanelViewer::createPart(QString mimetype)
{
    KParts::ReadOnlyPart *part = nullptr;

#if CREATE_KPART_5_82
    part = createKPartForMimeType<KParts::ReadOnlyPart>(mimetype, this);
#else
    KService::Ptr ptr = KMimeTypeTrader::self()->preferredService(mimetype, "KParts/ReadOnlyPart");
    if (ptr) {
        QVariantList args;
        QVariant argsProp = ptr->property("X-KDE-BrowserView-Args");
        if (argsProp.isValid())
            args << argsProp;
        QVariant prop = ptr->property("X-KDE-BrowserView-AllowAsDefault");
        if (!prop.isValid() || prop.toBool()) // defaults to true
            part = ptr->createInstance<KParts::ReadOnlyPart>(this, this, args);
    }
#endif

    if (part) {
        KParts::NavigationExtension *ext = KParts::NavigationExtension::childObject(part);
        if (ext) {
            connect(ext, &KParts::NavigationExtension::openUrlRequestDelayed, this, &PanelViewer::openUrl);
            connect(ext, &KParts::NavigationExtension::openUrlRequestDelayed, this, &PanelViewer::openUrlRequest);
        }
    }
    return part;
}

/* ----==={ PanelEditor }===---- */

PanelEditor::PanelEditor(QWidget *parent, KrViewer::Mode mode)
    : PanelViewerBase(parent, mode)
{
}

PanelEditor::~PanelEditor() = default;

void PanelEditor::configureDeps()
{
    bool foundPlugin = false;

#if CREATE_KPART_5_82
    foundPlugin = !KParts::PartLoader::partsForMimeType("text/plain").isEmpty();
#else
    KService::Ptr ptr = KMimeTypeTrader::self()->preferredService("text/plain", "KParts/ReadWritePart");
    if (!ptr)
        ptr = KMimeTypeTrader::self()->preferredService("all/allfiles", "KParts/ReadWritePart");
    foundPlugin = (ptr != nullptr);
#endif

    if (!foundPlugin)
        KMessageBox::error(nullptr, missingKPartMsg(), i18n("Missing Plugin"), KMessageBox::AllowLink);
}

QString PanelEditor::missingKPartMsg()
{
    return i18nc("missing kpart - arg1 is a URL",
                 "<b>No text editor plugin available.</b><br/>"
                 "Internal editor will not work without this.<br/>"
                 "You can fix this by installing Kate:<br/>%1",
                 QString("<a href='%1'>%1</a>").arg("https://kde.org/applications/utilities/org.kde.kate"));
}

void PanelEditor::openFile(KFileItem fi)
{
    KIO::filesize_t fileSize = fi.size();
    KConfigGroup group(krConfig, "General");
    KIO::filesize_t limitMB = (KIO::filesize_t)group.readEntry("Lister Limit", _ListerLimit);
    QString mimetype = fi.mimetype();

    if (mode == KrViewer::Generic)
        cpart = getPart(mimetype);

    if (fileSize > limitMB * 0x100000) {
        if (!cpart || mimetype.startsWith(QLatin1String("text/")) || mimetype.startsWith(QLatin1String("all/"))) {
            if (KMessageBox::Cancel
                == KMessageBox::warningContinueCancel(this, i18n("%1 is bigger than %2 MB", curl.toDisplayString(QUrl::PreferLocalFile), limitMB))) {
                setCurrentWidget(fallback);
                emit openUrlFinished(this, false);
                return;
            }
        }
    }

    if (!cpart)
        cpart = getPart("text/plain");
    if (!cpart)
        cpart = getPart("all/allfiles");

    if (cpart) {
        addWidget(cpart->widget());
        setCurrentWidget(cpart->widget());

        KParts::OpenUrlArguments args;
        args.setReload(true);
        cpart->setArguments(args);
        if (cpart->openUrl(curl)) {
            connect(cpart.data(), &KParts::ReadOnlyPart::destroyed, this, &PanelEditor::slotCPartDestroyed);
            emit openUrlFinished(this, true);
            return;
        } // else: don't show error message - assume this has been done by the editor part
    } else
        KMessageBox::error(this, missingKPartMsg(), i18n("Cannot edit %1", curl.toDisplayString(QUrl::PreferLocalFile)), KMessageBox::AllowLink);

    setCurrentWidget(fallback);
    emit openUrlFinished(this, false);
}

bool PanelEditor::queryClose()
{
    if (!cpart)
        return true;
    return dynamic_cast<KParts::ReadWritePart *>((KParts::ReadOnlyPart *)cpart)->queryClose();
}

bool PanelEditor::closeUrl()
{
    if (!cpart)
        return false;

    dynamic_cast<KParts::ReadWritePart *>((KParts::ReadOnlyPart *)cpart)->closeUrl(false);

    setCurrentWidget(fallback);
    cpart = nullptr;
    return true;
}

KParts::ReadOnlyPart *PanelEditor::createPart(QString mimetype)
{
    KParts::ReadWritePart *part = nullptr;

#if CREATE_KPART_5_82
    part = createKPartForMimeType<KParts::ReadWritePart>(mimetype, this);
#else
    KService::Ptr ptr = KMimeTypeTrader::self()->preferredService(mimetype, "KParts/ReadWritePart");
    if (ptr) {
        QVariantList args;
        QVariant argsProp = ptr->property("X-KDE-BrowserView-Args");
        if (argsProp.isValid())
            args << argsProp;
        QVariant prop = ptr->property("X-KDE-BrowserView-AllowAsDefault");
        if (!prop.isValid() || prop.toBool()) // defaults to true
            part = ptr->createInstance<KParts::ReadWritePart>(this, this, args);
    }
#endif

    if (part) {
        KParts::NavigationExtension *ext = KParts::NavigationExtension::childObject(part);
        if (ext) {
            connect(ext, &KParts::NavigationExtension::openUrlRequestDelayed, this, &PanelEditor::openUrl);
            connect(ext, &KParts::NavigationExtension::openUrlRequestDelayed, this, &PanelEditor::openUrlRequest);
        }
    }
    return part;
}

bool PanelEditor::isModified()
{
    if (cpart)
        return dynamic_cast<KParts::ReadWritePart *>((KParts::ReadOnlyPart *)cpart)->isModified();
    else
        return false;
}
