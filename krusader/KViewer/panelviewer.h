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


class PanelViewerBase: public QStackedWidget
{
    Q_OBJECT

public:
    PanelViewerBase(QWidget *parent, KrViewer::Mode mode = KrViewer::Default);
    virtual ~PanelViewerBase();
    inline KUrl url() const {
        return curl;
    }
    inline KParts::ReadOnlyPart* part() const {
        return cpart;
    }
    virtual bool isModified() {
        return false;
    }
    virtual bool isEditor() = 0;

public slots:
    virtual bool closeUrl() {
        return false;
    }
    virtual bool queryClose() {
        return true;
    }

    void openUrl(KUrl url);

signals:
    void openUrlRequest(const KUrl &url);
    void urlChanged(PanelViewerBase *, const KUrl &);
    void partDestroyed(PanelViewerBase *);
    void openUrlFinished(PanelViewerBase *viewWidget, bool success);

protected slots:
    void slotCPartDestroyed() {
        emit partDestroyed(this);
    }
    void slotStatResult(KJob* job);

protected:
    virtual void openFile(KFileItem fi) = 0;
    virtual KParts::ReadOnlyPart* createPart(QString mimetype) = 0;
    KParts::ReadOnlyPart* getPart(QString mimetype);


    QHash<QString, QPointer<KParts::ReadOnlyPart> > *mimes;
    QPointer<KParts::ReadOnlyPart> cpart;

    KUrl curl;
    QLabel *fallback;
    KrViewer::Mode mode;
};

class PanelViewer: public PanelViewerBase
{
    Q_OBJECT
public slots:
    virtual bool closeUrl();

public:
    PanelViewer(QWidget *parent, KrViewer::Mode mode = KrViewer::Default);
    ~PanelViewer();

    virtual bool isEditor() {
        return false;
    }

protected:
    virtual void openFile(KFileItem fi);
    virtual KParts::ReadOnlyPart* createPart(QString mimetype);
    KParts::ReadOnlyPart* getDefaultPart(KFileItem fi);
    KParts::ReadOnlyPart* getHexPart();
    KParts::ReadOnlyPart* getListerPart(bool hexMode = false);
    KParts::ReadOnlyPart* getTextPart();
};

class PanelEditor: public PanelViewerBase
{
    Q_OBJECT
public:
    virtual bool isModified();
    virtual bool isEditor() {
        return true;
    }

    static void configureDeps();

public slots:
    virtual bool closeUrl();
    virtual bool queryClose();

public:
    PanelEditor(QWidget *parent, KrViewer::Mode mode = KrViewer::Default);
    ~PanelEditor();

protected:
    virtual void openFile(KFileItem fi);
    virtual KParts::ReadOnlyPart* createPart(QString mimetype);
    static QString missingKPartMsg();
};

#endif
