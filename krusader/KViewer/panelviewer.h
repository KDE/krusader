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

#ifndef PANELVIEWER_H
#define PANELVIEWER_H

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtGui/QLabel>
#include <QtGui/QStackedWidget>

#include <kurl.h>
#include <kparts/part.h>
#include <kio/job.h>

#include "krviewer.h"

class KTemporaryFile;

class PanelViewerBase: public QStackedWidget
{
    Q_OBJECT

public:
    PanelViewerBase(QWidget *parent = 0);
    virtual ~PanelViewerBase();
    inline KUrl url() const {
        return curl;
    }
    inline void setUrl(KUrl url) {
        emit urlChanged(this, url); curl = url;
    }
    inline KParts::ReadOnlyPart* part() const {
        return cpart;
    }
    virtual bool isModified() {
        return false;
    }
    virtual bool isEditor() = 0;

public slots:
    virtual KParts::ReadOnlyPart* openUrl(const KUrl&, KrViewer::Mode = KrViewer::Generic) {
        return 0;
    }
    virtual bool closeUrl() {
        return false;
    }
    virtual bool queryClose() {
        return true;
    }

signals:
    void openUrlRequest(const KUrl &url);
    void urlChanged(PanelViewerBase *, const KUrl &);
    void partDestroyed(PanelViewerBase *);

protected slots:
    void slotCPartDestroyed() {
        emit partDestroyed(this);
    }

protected:
    QHash<QString, QPointer<KParts::ReadOnlyPart> > *mimes;
    QPointer<KParts::ReadOnlyPart> cpart;

    QString cmimetype;
    KUrl curl;
    QLabel *fallback;

};

class PanelViewer: public PanelViewerBase
{
    Q_OBJECT
public slots:
    KParts::ReadOnlyPart* openUrl(const KUrl &url, KrViewer::Mode mode = KrViewer::Generic);
    bool closeUrl();

public:
    PanelViewer(QWidget *parent = 0);
    ~PanelViewer();

    virtual bool isEditor() {
        return false;
    }

protected:
    KParts::ReadOnlyPart *getPart(QString mimetype);
    KParts::ReadOnlyPart*  getHexPart();
    void oldHexViewer(KTemporaryFile& tmpFile);
};

class PanelEditor: public PanelViewerBase
{
    Q_OBJECT
public:
    virtual bool isModified();
    virtual bool isEditor() {
        return true;
    }

public slots:
    KParts::ReadOnlyPart* openUrl(const KUrl &url, KrViewer::Mode mode = KrViewer::Generic);
    bool closeUrl();
    bool queryClose();
    void slotStatResult(KJob* job);

public:
    PanelEditor(QWidget *parent = 0);
    ~PanelEditor();

protected:
    KParts::ReadWritePart* getPart(QString mimetype);

    bool busy;
    KIO::UDSEntry entry;
};

#endif
