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

#include "filemanagerwindow.h"

#include <QtCore/QCommandLineParser>
#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtGui/QShowEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QHideEvent>
#include <QtGui/QMoveEvent>
#include <QtWidgets/QAction>

// TODO KF5 - these headers are from deprecated KDE4LibsSupport : remove them
#include <KDE/KDebug>

#include <KParts/MainWindow>
#include <KConfigWidgets/KStandardAction>
#include <KConfigCore/KConfig>
#include <KConfigCore/KConfigGroup>

#include "VFS/kiojobwrapper.h"

#ifdef __KJSEMBED__
class KrJS;
#endif

class KrusaderStatus;
class KRPleaseWaitHandler;
class KSystemTrayIcon;
class UserMenu;
class UserAction;
class PopularUrls;
class QueueManager;
class ViewActions;
class ListPanelActions;
class TabActions;

class Krusader : public KParts::MainWindow, public FileManagerWindow
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.krusader.Instance")

public:
    Krusader(const QCommandLineParser &parser);
    virtual ~Krusader();

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
    // FileManagerWindow implementation
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

    void configChanged();
    /**
     * This returns a defferent icon if krusader runs with root-privileges
     * @return a character string with the specitif icon-name
     */
    static const char* privIcon();
    static QStringList supportedTools(); // find supported tools


public slots:
    void moveToTop();
    void statusBarUpdate(QString& mess);
    // in use by Krusader only
    void saveSettings();
    void savePosition();
    void updateUserActions();
    void updateGUI(bool enforce = false);
    void slotClose();
    void setDirectExit() {
        directExit = true;
    }

protected slots:
    void doOpenUrl();

protected:
    bool queryClose();
    void setupActions();
    bool versionControl();  // handle version differences in krusaderrc
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void moveEvent(QMoveEvent *);
    void resizeEvent(QResizeEvent *);

public Q_SLOTS:
    Q_SCRIPTABLE bool isRunning();
    Q_SCRIPTABLE bool isLeftActive();
    Q_SCRIPTABLE bool openUrl(QString url);

public:
    static Krusader *App;       // a kApp style pointer
    static QString   AppName;   // the name of the application
    PopularUrls *_popularUrls; // holds a sorted list of the most popular urls visited
    QueueManager *queueManager;
    // return a path to a temp dir or file we can use.
    QString getTempDir();
    QString getTempFile();

    // the internal progress bar variales + functions
    KRPleaseWaitHandler* plzWait;
    void startWaiting(QString msg = "Please Wait", int count = 0 , bool cancel = false);
    void stopWait();
    bool wasWaitingCancelled() const;

    KrusaderStatus *status;
    static UserMenu *userMenu;

#ifdef __KJSEMBED__
    static KrJS *js;
#endif

signals:
    void changeMessage(QString);
    // emitted when we are about to quit
    void shutdown();

private:
    static void supportedTool(QStringList &tools, QString toolType,
                              QStringList names, QString confName);

    KrActions *_krActions;
    ViewActions *_viewActions;
    ListPanelActions *_listPanelActions;
    TabActions *_tabActions;
    KSystemTrayIcon *sysTray;
    QPoint       oldPos;
    QSize        oldSize;
    bool         isStarting;
    bool         isExiting;
    bool         directExit;
    KrJobStarter jobStarter;
    QTimer      _openUrlTimer;
    QString     _urlToOpen;
};


// main modules
#define krApp        Krusader::App

#ifdef __KJSEMBED__
#define krJS   Krusader::App->js
#endif

#endif
