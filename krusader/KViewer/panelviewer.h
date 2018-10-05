/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#ifndef PANELVIEWER_H
#define PANELVIEWER_H

// QtCore
#include <QHash>
#include <QString>
#include <QUrl>
// QtWidgets
#include <QStackedWidget>
#include <QLabel>

# include <KParts/Part>

#include "krviewer.h"


class PanelViewerBase: public QStackedWidget
{
    Q_OBJECT

public:
    explicit PanelViewerBase(QWidget *parent, KrViewer::Mode mode = KrViewer::Default);
    ~PanelViewerBase() override;
    inline QUrl url() const {
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

    void openUrl(const QUrl& url);

signals:
    void openUrlRequest(const QUrl &url);
    void urlChanged(PanelViewerBase *, const QUrl &);
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
    KParts::ReadOnlyPart* getPart(const QString& mimetype);


    QHash<QString, QPointer<KParts::ReadOnlyPart> > *mimes;
    QPointer<KParts::ReadOnlyPart> cpart;

    QUrl curl;
    QLabel *fallback;
    KrViewer::Mode mode;
};

class PanelViewer: public PanelViewerBase
{
    Q_OBJECT
public slots:
    bool closeUrl() Q_DECL_OVERRIDE;

public:
    explicit PanelViewer(QWidget *parent, KrViewer::Mode mode = KrViewer::Default);
    ~PanelViewer() override;

    bool isEditor() Q_DECL_OVERRIDE {
        return false;
    }

protected:
    void openFile(KFileItem fi) Q_DECL_OVERRIDE;
    KParts::ReadOnlyPart* createPart(QString mimetype) Q_DECL_OVERRIDE;
    KParts::ReadOnlyPart* getDefaultPart(const KFileItem& fi);
    KParts::ReadOnlyPart* getHexPart();
    KParts::ReadOnlyPart* getListerPart(bool hexMode = false);
    KParts::ReadOnlyPart* getTextPart();
};

class PanelEditor: public PanelViewerBase
{
    Q_OBJECT
public:
    bool isModified() Q_DECL_OVERRIDE;
    bool isEditor() Q_DECL_OVERRIDE {
        return true;
    }

    static void configureDeps();

public slots:
    bool closeUrl() Q_DECL_OVERRIDE;
    bool queryClose() Q_DECL_OVERRIDE;

public:
    explicit PanelEditor(QWidget *parent, KrViewer::Mode mode = KrViewer::Default);
    ~PanelEditor() override;

protected:
    void openFile(KFileItem fi) Q_DECL_OVERRIDE;
    KParts::ReadOnlyPart* createPart(QString mimetype) Q_DECL_OVERRIDE;
    static QString missingKPartMsg();
};

#endif
