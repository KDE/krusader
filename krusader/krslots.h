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

    KRslots(QObject *parent): QObject(parent) {}
    ~KRslots() {}

public slots:
    void createChecksum();
    void matchChecksum();
    void sendFileByEmail(const KUrl::List &filename);
    void compareContent();
    void compareContent(KUrl, KUrl);
    void rightclickMenu();
    void insertFileName(bool full_path);
    void rootKrusader();
    void swapPanels();
    void setView0();
    void setView1();
    void setView2();
    void setView3();
    void setView4();
    void setView5();
    void toggleHidden();
    void togglePreviews(bool show);
    void toggleSwapSides();
    void togglePopupPanel();
    void configToolbar();
    void configKeys();
    void toggleToolbar();
    void toggleActionsToolbar();
    void toggleStatusbar();
    void toggleTerminal();
    void home();
    void root();
    void dirUp();
    void markAll();
    void unmarkAll();
    void markGroup();
    void markGroup(const QString &, bool select);
    void unmarkGroup();
    void invert();
    void compareDirs();
    void compareSetup();
    void emptyTrash();
    void trashBin();
    /** called by actExec* actions to choose the built-in command line mode */
    void execTypeSetup();
    void refresh();
    void refresh(const KUrl& u);
    void properties();
    void back();
    void slotPack();
    void slotUnpack();
    void cut();
    void copy();
    void paste();
    void testArchive();
    void calcSpace();
    void FTPDisconnect();
    void allFilter();
    void execFilter();
    void customFilter();
    void newFTPconnection();
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
    void runMountMan();
    void toggleFnkeys();
    void toggleCmdline();
    void changeTrashIcon();
    void multiRename();
    void openRightBookmarks();
    void openLeftBookmarks();
    void openBookmarks();
    void bookmarkCurrent();
    void openHistory();
    void openLeftHistory();
    void openRightHistory();
    void openMedia();
    void openLeftMedia();
    void openRightMedia();
    void syncPanels();
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
    void slotSyncBrowse();
    void slotDiskUsage();
    void slotQueueManager();
    void slotLocationBar();
    void slotJumpBack();
    void slotSetJumpBack();
    void newSymlink();
    void updatePopupPanel(KrViewItem *);
    void windowActive(); // called when krusader's window becomes focused
    void windowInactive(); // called when another application steals the focus
    // F2
    void terminal();
    // F3
    void view();
    // Shift F3
    void viewDlg();
    // F4
    void edit();
    // Shift F4
    void editDlg();
    // F5
    void copyFiles();
    // F6
    void moveFiles();
    // SHIFT + F5
    void copyFilesByQueue();
    // SHIFT + F6
    void moveFilesByQueue();
    // F7
    void mkdir();
    // F8
    void deleteFiles(bool reallyDelete = false);
    // F9
    void rename();
    void jsConsole();
    void saveNewToolbarConfig();
    void cancelRefresh();
    void zoomIn();
    void zoomOut();
    void defaultZoom();
    void showViewOptionsMenu();
};

#endif
