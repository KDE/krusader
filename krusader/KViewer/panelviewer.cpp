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

PanelViewerBase::PanelViewerBase(QWidget *parent) :
        QStackedWidget(parent), mimes(0), cpart(0)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored));

    mimes = new QHash<QString, QPointer<KParts::ReadOnlyPart> >();
    cpart = 0;
    fallback = new QLabel(i18n("No file selected or selected file can't be displayed."), this);
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

KFileItem PanelViewerBase::readFileInfo(const KUrl & url)
{
    if (url.isLocalFile())
        return KFileItem(KFileItem::Unknown, KFileItem::Unknown, url);

    KIO::StatJob* statJob = KIO::stat(url, KIO::HideProgressInfo);
    connect(statJob, SIGNAL(result(KJob*)), this, SLOT(slotStatResult(KJob*)));
    busy = true;
    while (busy) qApp->processEvents();
    if (entry.count() != 0) {
        return KFileItem(entry, url);
    }
    return KFileItem();
}

void PanelViewerBase::slotStatResult(KJob* job)
{
    if (!job || job->error()) entry = KIO::UDSEntry();
    else entry = static_cast<KIO::StatJob*>(job)->statResult();
    busy = false;
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


/* ----==={ PanelViewer }===---- */

PanelViewer::PanelViewer(QWidget *parent) :
        PanelViewerBase(parent)
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

    if (mimes->find(QLatin1String("libkhexedit2part")) == mimes->end()) {
        KPluginFactory* factory = KLibLoader::self()->factory("libkhexedit2part");
        if (factory) {
            // Create the part
            part = (KParts::ReadOnlyPart *) factory->create(this, "KParts::ReadOnlyPart");
            mimes->insert(QLatin1String("libkhexedit2part"), part);
        }
    } else
        part = (*mimes)[ QLatin1String("libkhexedit2part")];

    if(!part)
        part = getListerPart(true);

    return part;
}

KParts::ReadOnlyPart* PanelViewer::openUrl(const KUrl &url, KrViewer::Mode mode)
{
    emit urlChanged(this, url);
    closeUrl();
    curl = url;

    KIO::filesize_t fileSize = readFileInfo(curl).size();
    KConfigGroup group(krConfig, "General");
    KIO::filesize_t limit = (KIO::filesize_t)group.readEntry("Lister Limit", _ListerLimit);
    limit *= 0x100000;

    KMimeType::Ptr mt = KMimeType::findByUrl(curl);
    cmimetype = mt ? mt->name() : QString();

    switch(mode) {
    case KrViewer::Generic:
        {

            if (fileSize > limit && (cmimetype.startsWith(QLatin1String("text/")) ||
                                    cmimetype.startsWith(QLatin1String("all/"))))
                cpart = getListerPart();
            else {
                // KDE 4 HACK : START
                // KDE 4 crashes at viewing directories
                if (cmimetype == "inode/directory")
                    return 0;
                // KDE 4 HACK : END
                cpart = getPart(cmimetype);
            }
        }
        if(cpart)
            break;
    case KrViewer::Text:
        if(fileSize > limit || mt->isBinaryData(curl.path())) // FIXME isBinaryData() only works on local files
            cpart = getHexPart();
        else {
            cpart = getPart("text/plain");
            if(!cpart)
                cpart = getPart("all/allfiles");
        }
        break;
    case KrViewer::Lister:
        cpart = getListerPart(mt->isBinaryData(curl.path())); // FIXME isBinaryData() only works on local files
        break;
    case KrViewer::Hex:
        cpart = getHexPart();
        break;
    default:
        abort();
    }

    if(cpart) {
        addWidget(cpart->widget());
        setCurrentWidget(cpart->widget());

        KParts::OpenUrlArguments args;
        args.setReload(true);
        cpart->setArguments(args);
        if (cpart->openUrl(curl)) {
            connect(cpart, SIGNAL(destroyed()), this, SLOT(slotCPartDestroyed()));
            return cpart;
        }
    }

    setCurrentWidget(fallback);
    return 0;
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
    KPluginFactory *factory = 0;
    KService::Ptr ptr = KMimeTypeTrader::self()->preferredService(mimetype, "KParts/ReadOnlyPart");
    if (ptr) {
        QStringList args;
        QVariant argsProp = ptr->property("X-KDE-BrowserView-Args");
        if (argsProp.isValid()) {
            QString argStr = argsProp.toString();
            args = argStr.split(' ');
        }
        QVariant prop = ptr->property("X-KDE-BrowserView-AllowAsDefault");
        if (!prop.isValid() || prop.toBool()) {   // defaults to true
            factory = KLibLoader::self() ->factory(ptr->library().toLatin1());
            if (factory) {
                if (ptr->serviceTypes().contains("Browser/View"))
                    part = static_cast<KParts::ReadOnlyPart *>(factory->create(this,
                            QString("Browser/View").toLatin1(), args));
                if (!part)
                    part = static_cast<KParts::ReadOnlyPart *>(factory->create(this,
                            QString("KParts::ReadOnlyPart").toLatin1(), args));
            }
        }
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

PanelEditor::PanelEditor(QWidget *parent) :
        PanelViewerBase(parent)
{
}

PanelEditor::~PanelEditor()
{
}

KParts::ReadOnlyPart* PanelEditor::openUrl(const KUrl &url, KrViewer::Mode mode)
{
    emit urlChanged(this, url);
    closeUrl();
    curl = url;

    KIO::filesize_t fileSize = readFileInfo(curl).size();
    KConfigGroup group(krConfig, "General");
    KIO::filesize_t limit = (KIO::filesize_t)group.readEntry("Lister Limit", _ListerLimit);

    if (mode == KrViewer::Generic) {
        KMimeType::Ptr mt = KMimeType::findByUrl(curl);
        cmimetype = mt ? mt->name() : QString();

        cpart = getPart(cmimetype);
    }

    if(fileSize > limit * 0x100000) {
        if(!cpart || cmimetype.startsWith(QLatin1String("text/")) ||
                cmimetype.startsWith(QLatin1String("all/"))) {
            if(KMessageBox::Cancel == KMessageBox::warningContinueCancel(this,
                  i18n("%1 is bigger than %2 MB" , curl.pathOrUrl(), limit))) {
                setCurrentWidget(fallback);
                return 0;
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
            return cpart;
        }

    }

    setCurrentWidget(fallback);
    return 0;
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
    KPluginFactory *factory = 0;
    KService::Ptr ptr = KMimeTypeTrader::self()->preferredService(mimetype, "KParts/ReadWritePart");
    if (ptr) {
        QStringList args;
        QVariant argsProp = ptr->property("X-KDE-BrowserView-Args");
        if (argsProp.isValid()) {
            QString argStr = argsProp.toString();
            args = argStr.split(' ');
        }
        QVariant prop = ptr->property("X-KDE-BrowserView-AllowAsDefault");
        if (!prop.isValid() || prop.toBool()) {  // defaults to true
            factory = KLibLoader::self() ->factory(ptr->library().toLatin1());
            if (factory) {
                part = static_cast<KParts::ReadWritePart *>(factory->create(this,
                        QString("KParts::ReadWritePart").toLatin1(), args));
            }
        }
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
    return static_cast<KParts::ReadWritePart *>((KParts::ReadOnlyPart *)cpart)->isModified();
}

#include "panelviewer.moc"
