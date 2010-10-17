/***************************************************************************
                                krslots.h
                            -------------------
   copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
   email                : krusader@users.sourceforge.net
   web site             : http://krusader.sourceforge.net
---------------------------------------------------------------------------
 Description
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



#ifndef KRSLOTS_H
#define KRSLOTS_H

#include <QtCore/QObject>
#include <kprocess.h>
#include <kio/netaccess.h>
#include "krglobal.h"

class KrMainWindow;
class ListPanel;
class KrViewItem;
class KUrl;

class KrProcess: public KProcess
{
    Q_OBJECT

    QString tmp1, tmp2;

public:
    KrProcess(QString in1, QString in2) {
        tmp1 = in1;
        tmp2 = in2;
        connect(this,  SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(processHasExited()));
    }

public slots:
    void processHasExited() {
        if (!tmp1.isEmpty())
            KIO::NetAccess::removeTempFile(tmp1);
        if (!tmp2.isEmpty())
            KIO::NetAccess::removeTempFile(tmp2);
        deleteLater();
    }
};

class KRslots : public QObject
{
    Q_OBJECT

public:
    enum compareMode { full } ;

    KRslots(QObject *parent);
    ~KRslots() {}

public slots:
    void sendFileByEmail(const KUrl::List &filename);
    void compareContent();
    void compareContent(KUrl, KUrl);
    void insertFileName(bool full_path);
    void rootKrusader();
    void swapPanels();
    void toggleHidden();
    void togglePreviews(bool show);
    void toggleSwapSides();
    void configToolbar();
    void configKeys();
    void toggleToolbar();
    void toggleActionsToolbar();
    void toggleStatusbar();
    void toggleTerminal();
    void compareSetup();
    void emptyTrash();
    void trashBin();
    /** called by actExec* actions to choose the built-in command line mode */
    void execTypeSetup();
    void refresh(const KUrl& u);
    void runKonfigurator(bool firstTime = false);
    void startKonfigurator() {
        runKonfigurator(false);
    }
    void search();           // call the search module
    void locate();
    void runTerminal(const QString & dir, const QStringList & args);
    void homeTerminal();
    void sysInfo();
    void addBookmark();
    void toggleFnkeys();
    void toggleCmdline();
    void changeTrashIcon();
    void multiRename();
    void bookmarkCurrent();
    void cmdlinePopup();
    void duplicateTab();
    void newTab(const KUrl& url = KUrl());
    void newTab(KrViewItem *item);
    void lockTab();
    void closeTab();
    void nextTab();
    void previousTab();
    void closeInactiveTabs();
    void closeDuplicatedTabs();
    void slotSplit();
    void slotCombine();
    void userMenu();
    void manageUseractions();
    void slotSynchronizeDirs(QStringList selected = QStringList());
    void slotDiskUsage();
    void slotQueueManager();
    void windowActive(); // called when krusader's window becomes focused
    void windowInactive(); // called when another application steals the focus
    void jsConsole();
    void saveNewToolbarConfig();
    void zoomIn();
    void zoomOut();
    void defaultZoom();
    void showViewOptionsMenu();
    void viewSaveDefaultSettings();
    void viewApplySettingsToOthers();
    void focusPanel();

protected slots:
    void configChanged(bool isGUIRestartNeeded);

protected:
    KrMainWindow *_mainWindow;
};

#endif
