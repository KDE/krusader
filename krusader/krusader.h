/***************************************************************************
                                krusader.h
                           -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
 The main application ! what's more to say ?
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRUSADER_H
#define KRUSADER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "krmainwindow.h"

// QtCore
#include <QCommandLineParser>
#include <QEvent>
#include <QPointer>
#include <QStringList>
#include <QTimer>
// QtGui
#include <QHideEvent>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QShowEvent>
// QtWidgets
#include <QAction>

#include <KConfigCore/KConfig>
#include <KConfigCore/KConfigGroup>
#include <KConfigWidgets/KStandardAction>
#include <KNotifications/KStatusNotifierItem>
#include <KParts/MainWindow>

#ifdef __KJSEMBED__
class KrJS;
#endif

class KStartupInfoData;
class KStartupInfoId;

class KrusaderStatus;
class KRPleaseWaitHandler;
class PopularUrls;
class ViewActions;
class ListPanelActions;
class TabActions;
class KrView;

/**
 * @brief The main window of this file manager
 */
class Krusader : public KParts::MainWindow, public KrMainWindow
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.krusader.Instance")

public:
    explicit Krusader(const QCommandLineParser &parser);
    virtual ~Krusader();

    void setTray(bool forceCreation = false);

    // KrMainWindow implementation
    virtual QWidget *widget() Q_DECL_OVERRIDE {
        return this;
    }
    virtual KrView *activeView() Q_DECL_OVERRIDE;
    ViewActions *viewActions() {
        return _viewActions;
    }
    virtual KActionCollection *actions() {
        return actionCollection();
    }
    virtual AbstractPanelManager *activeManager() Q_DECL_OVERRIDE;
    virtual AbstractPanelManager *leftManager() Q_DECL_OVERRIDE;
    virtual AbstractPanelManager *rightManager() Q_DECL_OVERRIDE;
    virtual PopularUrls *popularUrls() Q_DECL_OVERRIDE {
        return _popularUrls;
    }
    virtual KrActions *krActions() Q_DECL_OVERRIDE {
        return _krActions;
    }
    virtual ListPanelActions *listPanelActions() Q_DECL_OVERRIDE {
        return _listPanelActions;
    }
    virtual TabActions *tabActions() Q_DECL_OVERRIDE {
        return _tabActions;
    }
    virtual void plugActionList(const char *name, QList<QAction*> &list) Q_DECL_OVERRIDE {
        KParts::MainWindow::plugActionList(name, list);
    }
    /**
     * This returns a defferent icon if krusader runs with root-privileges
     * @return a character string with the specitif icon-name
     */
    static const char* privIcon();

public slots:
    void quit();
    void moveToTop();
    void statusBarUpdate(const QString& mess);
    // in use by Krusader only
    void saveSettings();
    void savePosition();
    void updateUserActions();

protected slots:
    void doOpenUrl();
    void slotGotNewStartup(const KStartupInfoId &id, const KStartupInfoData &data);
    void slotGotRemoveStartup(const KStartupInfoId &id, const KStartupInfoData &data);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    bool queryClose() Q_DECL_OVERRIDE;
    void setupActions();
    bool versionControl();  // handle version differences in krusaderrc
    bool event(QEvent *) Q_DECL_OVERRIDE;

public Q_SLOTS:
    Q_SCRIPTABLE bool isRunning();
    Q_SCRIPTABLE bool isLeftActive();
    Q_SCRIPTABLE bool openUrl(QString url);

public:
    static Krusader *App;       // a kApp style pointer
    static QString   AppName;   // the name of the application
    PopularUrls *_popularUrls; // holds a sorted list of the most popular urls visited

    // the internal progress bar variales + functions
    KRPleaseWaitHandler* plzWait;
    void startWaiting(QString msg = "Please Wait", int count = 0 , bool cancel = false);
    void stopWait();
    bool wasWaitingCancelled() const;

#ifdef __KJSEMBED__
    static KrJS *js;
#endif

signals:
    void changeMessage(QString);
    // emitted when we are about to quit
    void shutdown();

private:
    void acceptClose();

private:
    KrActions *_krActions;
    ViewActions *_viewActions;
    ListPanelActions *_listPanelActions;
    TabActions *_tabActions;
    QPointer<KStatusNotifierItem> sysTray;
    bool         isStarting;
    bool         isExiting;
    QTimer      _openUrlTimer;
    QString     _urlToOpen;
    bool        _quit;
};


// main modules
#define krApp        Krusader::App

#ifdef __KJSEMBED__
#define krJS   Krusader::App->js
#endif

#endif
