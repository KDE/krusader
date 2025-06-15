/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PANELVIEWER_H
#define PANELVIEWER_H

// QtCore
#include <QHash>
#include <QString>
#include <QUrl>
// QtWidgets
#include <QLabel>
#include <QStackedWidget>
#include <KFileItem>
#include <KParts/Part>

#include "krviewer.h"

class PanelViewerBase : public QStackedWidget
{
    Q_OBJECT

public:
    explicit PanelViewerBase(QWidget *parent, KrViewer::Mode mode = KrViewer::Default);
    ~PanelViewerBase() override;
    inline QUrl url() const
    {
        return cpart ? cpart->url() : QUrl();
    }
    inline KParts::ReadOnlyPart *part() const
    {
        return cpart;
    }
    virtual bool isModified()
    {
        return false;
    }
    virtual bool isEditor() = 0;

public slots:
    virtual bool closeUrl()
    {
        return false;
    }
    virtual bool queryClose()
    {
        return true;
    }

    void openUrl(const QUrl &url);

signals:
    void openUrlRequest(const QUrl &url);
    void urlChanged(PanelViewerBase *, const QUrl &);
    void partDestroyed(PanelViewerBase *);
    void openUrlFinished(PanelViewerBase *viewWidget, bool success);

protected slots:
    void slotCPartDestroyed()
    {
        emit partDestroyed(this);
    }
    void slotStatResult(KJob *job);

protected:
    virtual void openFile(KFileItem fi) = 0;
    virtual KParts::ReadOnlyPart *createPart(QString mimetype) = 0;
    KParts::ReadOnlyPart *getPart(const QString &mimetype);

    QHash<QString, QPointer<KParts::ReadOnlyPart>> *mimes;
    QPointer<KParts::ReadOnlyPart> cpart;

    QLabel *fallback;
    KrViewer::Mode mode;
};

class PanelViewer : public PanelViewerBase
{
    Q_OBJECT
public slots:
    bool closeUrl() override;

public:
    explicit PanelViewer(QWidget *parent, KrViewer::Mode mode = KrViewer::Default);
    ~PanelViewer() override;

    bool isEditor() override
    {
        return false;
    }

protected:
    void openFile(KFileItem fi) override;
    KParts::ReadOnlyPart *createPart(QString mimetype) override;
    KParts::ReadOnlyPart *getDefaultPart(const KFileItem &fi);
    KParts::ReadOnlyPart *getHexPart();
    KParts::ReadOnlyPart *getListerPart(bool hexMode = false);
    KParts::ReadOnlyPart *getTextPart();
};

class PanelEditor : public PanelViewerBase
{
    Q_OBJECT
public:
    bool isModified() override;
    bool isEditor() override
    {
        return true;
    }

    static void configureDeps();

public slots:
    bool closeUrl() override;
    bool queryClose() override;

public:
    explicit PanelEditor(QWidget *parent, KrViewer::Mode mode = KrViewer::Default);
    ~PanelEditor() override;

protected:
    void openFile(KFileItem fi) override;
    KParts::ReadOnlyPart *createPart(QString mimetype) override;
    static QString missingKPartMsg();
};

#endif
