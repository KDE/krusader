/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#include "panelviewer.h"

#include <QtCore/QFile>
#include <QApplication>

#include <kparts/browserextension.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <ktemporaryfile.h>
#include <klocale.h>
#include <klibloader.h>
#include <kservicetypeprofile.h>
#include <kmimetypetrader.h>
#include <kdebug.h>
#include <kfileitem.h>
#include <kio/netaccess.h>
#include <kde_file.h>

#include "lister.h"
#include "../defaults.h"

#define DICTSIZE 211

PanelViewerBase::PanelViewerBase(QWidget *parent, KrViewer::Mode mode) :
        QStackedWidget(parent), mimes(0), cpart(0), mode(mode)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored));

    mimes = new QHash<QString, QPointer<KParts::ReadOnlyPart> >();
    cpart = 0;
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
    QHashIterator< QString, QPointer<KParts::ReadOnlyPart> > lit(*mimes);
    while (lit.hasNext()) {
        QPointer<KParts::ReadOnlyPart> p = lit.next().value();
        if (p)
            delete p;
    }
    mimes->clear();
    delete mimes;
    delete fallback;
}

void PanelViewerBase::slotStatResult(KJob* job)
{
    if (job->error()) {
        KMessageBox::error(this, job->errorString());
        emit openUrlFinished(this, false);
    } else {
        KIO::UDSEntry entry = static_cast<KIO::StatJob*>(job)->statResult();
        openFile(KFileItem(entry, curl));
    }
}

KParts::ReadOnlyPart* PanelViewerBase::getPart(QString mimetype)
{
    KParts::ReadOnlyPart* part = 0;

    if (mimes->find(mimetype) == mimes->end()) {
        part = createPart(mimetype);
        if(part)
            mimes->insert(mimetype, part);
    } else
        part = (*mimes)[mimetype];

    return part;
}

void PanelViewerBase::openUrl(KUrl url)
{
    closeUrl();
    curl = url;
    emit urlChanged(this, url);

    if (url.isLocalFile())
        openFile(KFileItem(KFileItem::Unknown, KFileItem::Unknown, url));
    else {
        KIO::StatJob* statJob = KIO::stat(url, KIO::HideProgressInfo);
        connect(statJob, SIGNAL(result(KJob*)), this, SLOT(slotStatResult(KJob*)));
    }
}


/* ----==={ PanelViewer }===---- */

PanelViewer::PanelViewer(QWidget *parent, KrViewer::Mode mode) :
        PanelViewerBase(parent, mode)
{
}

PanelViewer::~PanelViewer()
{
}

KParts::ReadOnlyPart* PanelViewer::getListerPart(bool hexMode)
{
    KParts::ReadOnlyPart* part = 0;

    if (mimes->find(QLatin1String("krusader_lister")) == mimes->end()) {
        part = new Lister(this);
        mimes->insert(QLatin1String("krusader_lister"), part);
    } else
        part = (*mimes)[ QLatin1String("krusader_lister")];

    if (part) {
        Lister *lister = dynamic_cast<Lister *>((KParts::ReadOnlyPart *)part);
        if (lister)
            lister->setHexMode(hexMode);
    }
    return part;
}

KParts::ReadOnlyPart* PanelViewer::getHexPart()
{
    KParts::ReadOnlyPart* part = 0;

    if (KConfigGroup(krConfig, "General").readEntry("UseOktetaViewer", _UseOktetaViewer)) {
        if (mimes->find("oktetapart") == mimes->end()) {
            KPluginLoader loader("oktetapart");
            if (KPluginFactory *factory = loader.factory()) {
                if ((part = factory->create<KParts::ReadOnlyPart>(this, this)))
                    mimes->insert("oktetapart", part);
            }
        } else
            part = (*mimes)["oktetapart"];
    }

    return part ? part : getListerPart(true);
}

KParts::ReadOnlyPart* PanelViewer::getTextPart()
{
    KParts::ReadOnlyPart* part = getPart("text/plain");
    if(!part)
        part = getPart("all/allfiles");
    return part ? part : getListerPart();
}

KParts::ReadOnlyPart* PanelViewer::getDefaultPart(KFileItem fi)
{
    KConfigGroup group(krConfig, "General");
    QString modeString = group.readEntry("Default Viewer Mode", QString("generic"));

    KrViewer::Mode mode = KrViewer::Generic;

    if (modeString == "generic") mode = KrViewer::Generic;
    else if (modeString == "text") mode = KrViewer::Text;
    else if (modeString == "hex") mode = KrViewer::Hex;
    else if (modeString == "lister") mode = KrViewer::Lister;

    bool isBinary = false;
    // FIXME isBinaryData() only works on local files
    if (fi.isLocalFile())
        isBinary  = KMimeType::isBinaryData(fi.localPath());

    KIO::filesize_t fileSize = fi.size();
    KIO::filesize_t limit = (KIO::filesize_t)group.readEntry("Lister Limit", _ListerLimit) * 0x100000;

    QString mimetype = fi.mimetype();

    KParts::ReadOnlyPart* part = 0;

    switch(mode) {
    case KrViewer::Generic:
        if((mimetype.startsWith(QLatin1String("text/")) ||
            mimetype.startsWith(QLatin1String("all/"))) &&
                fileSize > limit) {
            part = getListerPart(isBinary);
            break;
        } else if((part = getPart(mimetype)))
            break;
    case KrViewer::Text:
        if (fileSize > limit)
            part =  getListerPart(isBinary);
        else if (isBinary)
            part = getHexPart();
        else
            part = getTextPart();
        break;
    case KrViewer::Lister:
        part = getListerPart(isBinary);
        break;
    case KrViewer::Hex:
        if (fileSize > limit)
            part = getListerPart(true);
        else
            part = getHexPart();
        break;
    default:
        abort();
    }

    return part;
}

void PanelViewer::openFile(KFileItem fi)
{
    switch(mode) {
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

    if(cpart) {
        addWidget(cpart->widget());
        setCurrentWidget(cpart->widget());

        if (cpart->inherits("KParts::ReadWritePart"))
            static_cast<KParts::ReadWritePart*>(cpart.data())->setReadWrite(false);
        KParts::OpenUrlArguments args;
        args.setReload(true);
        cpart->setArguments(args);
        if (cpart->openUrl(curl)) {
            connect(cpart, SIGNAL(destroyed()), this, SLOT(slotCPartDestroyed()));
            emit openUrlFinished(this, true);
            return;
        }
    }

    setCurrentWidget(fallback);
    emit openUrlFinished(this, false);
}

bool PanelViewer::closeUrl()
{
    setCurrentWidget(fallback);
    if (cpart && cpart->closeUrl()) {
        cpart = 0;
        return true;
    }
    return false;
}

KParts::ReadOnlyPart* PanelViewer::createPart(QString mimetype)
{
    KParts::ReadOnlyPart * part = 0;
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
    if (part) {
        KParts::BrowserExtension * ext = KParts::BrowserExtension::childObject(part);
        if (ext) {
            connect(ext, SIGNAL(openUrlRequestDelayed(const KUrl &, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&)), this, SLOT(openUrl(const KUrl &)));
            connect(ext, SIGNAL(openUrlRequestDelayed(const KUrl &, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&)), this, SIGNAL(openUrlRequest(const KUrl &)));
        }
    }
    return part;
}


/* ----==={ PanelEditor }===---- */

PanelEditor::PanelEditor(QWidget *parent, KrViewer::Mode mode) :
        PanelViewerBase(parent, mode)
{
}

PanelEditor::~PanelEditor()
{
}

void PanelEditor::configureDeps()
{
    KService::Ptr ptr = KMimeTypeTrader::self()->preferredService("text/plain", "KParts/ReadWritePart");
    if (!ptr)
        ptr = KMimeTypeTrader::self()->preferredService("all/allfiles", "KParts/ReadWritePart");
    if (!ptr)
        KMessageBox::sorry(0, missingKPartMsg(), i18n("Missing Plugin"), KMessageBox::AllowLink);

}

QString PanelEditor::missingKPartMsg()
{
    return i18nc("missing kpart - arg1 is a URL",
                 "<b>No text editor plugin available.</b><br/>"
                    "Internal editor will not work without this.<br/>"
                    "You can fix this by installing Kate:<br/>%1",
                 QString("<a href='%1'>%1</a>").arg(
                    "http://www.kde.org/applications/utilities/kate"));
}

void PanelEditor::openFile(KFileItem fi)
{
    KIO::filesize_t fileSize = fi.size();
    KConfigGroup group(krConfig, "General");
    KIO::filesize_t limitMB = (KIO::filesize_t)group.readEntry("Lister Limit", _ListerLimit);
    QString mimetype = fi.mimetype();

    if (mode == KrViewer::Generic)
        cpart = getPart(mimetype);

    if(fileSize > limitMB * 0x100000) {
        if(!cpart || mimetype.startsWith(QLatin1String("text/")) ||
                mimetype.startsWith(QLatin1String("all/"))) {
            if(KMessageBox::Cancel == KMessageBox::warningContinueCancel(this,
                  i18n("%1 is bigger than %2 MB" , curl.pathOrUrl(), limitMB))) {
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
            connect(cpart, SIGNAL(destroyed()), this, SLOT(slotCPartDestroyed()));
            emit openUrlFinished(this, true);
            return;
        } // else: don't show error message - assume this has been done by the editor part
    } else
        KMessageBox::sorry(this, missingKPartMsg(), i18n("Cannot edit %1", curl.pathOrUrl()),
                           KMessageBox::AllowLink);

    setCurrentWidget(fallback);
    emit openUrlFinished(this, false);
}

bool PanelEditor::queryClose()
{
    if (!cpart) return true;
    return static_cast<KParts::ReadWritePart *>((KParts::ReadOnlyPart *)cpart)->queryClose();
}

bool PanelEditor::closeUrl()
{
    if (!cpart) return false;

    static_cast<KParts::ReadWritePart *>((KParts::ReadOnlyPart *)cpart)->closeUrl(false);

    setCurrentWidget(fallback);
    cpart = 0;
    return true;
}

KParts::ReadOnlyPart* PanelEditor::createPart(QString mimetype)
{
    KParts::ReadWritePart * part = 0L;
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
    if (part) {
        KParts::BrowserExtension * ext = KParts::BrowserExtension::childObject(part);
        if (ext) {
            connect(ext, SIGNAL(openUrlRequestDelayed(const KUrl &, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&)), this, SLOT(openUrl(const KUrl &)));
            connect(ext, SIGNAL(openUrlRequestDelayed(const KUrl &, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&)), this, SIGNAL(openUrlRequest(const KUrl &)));
        }
    }
    return part;
}

bool PanelEditor::isModified()
{
    if (cpart)
        return static_cast<KParts::ReadWritePart *>((KParts::ReadOnlyPart *)cpart)->isModified();
    else
        return false;
}

#include "panelviewer.moc"
