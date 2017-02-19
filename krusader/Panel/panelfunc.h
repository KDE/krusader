/***************************************************************************
                                panelfunc.h
                             -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
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


#ifndef PANELFUNC_H
#define PANELFUNC_H

#include "krviewitem.h"
#include "../FileSystem/vfs.h"

// QtCore
#include <QObject>
#include <QTimer>
// QtGui
#include <QClipboard>

#include <KService/KService>

class DirHistoryQueue;
class ListPanel;

class ListPanelFunc : public QObject
{
    friend class ListPanel;
    Q_OBJECT
public slots:
    void execute(const QString&);
    void goInside(const QString&);
    void navigatorUrlChanged(const QUrl &url);
    void openUrl(const QUrl &path, const QString& nameToMakeCurrent = QString(),
                 bool manuallyEntered = false);
//     void popErronousUrl();
    void immediateOpenUrl(const QUrl &url, bool disableLock = false);
    void rename(const QString &oldname, const QString &newname);

    // actions
    void historyBackward();
    void historyForward();
    void dirUp();
    void refresh();
    void home();
    void root();
    void cdToOtherPanel();
    void FTPDisconnect();
    void newFTPconnection();
    void terminal();
    void view();
    void viewDlg();
    void edit();
    void editNew(); // create a new textfile and edit it
    void moveFilesByQueue(); // start in queue regardless of _queueMode state
    void copyFilesByQueue(); // start in queue regardless of _queueMode state
    void moveFiles(bool reverseQueueMode = false) { copyFiles(reverseQueueMode, true); }
    void copyFiles(bool reverseQueueMode = false, bool move = false);

    /*!
     * asks the user the new directory name
     */
    void mkdir();
    void deleteFiles(bool reallyDelete = false);
    // delete virtual files or directories in virtual filesystem
    void removeVirtualFiles();
    void rename();
    void krlink(bool sym = true);
    void createChecksum();
    void matchChecksum();
    void pack();
    void unpack();
    void testArchive();
    // Calculate the occupied space of an item or the currently selected item and show it in a dialog
    void calcSpace(KrViewItem *item = 0);
    void properties();
    void cut() {
        copyToClipboard(true);
    }
    void copyToClipboard(bool move = false);
    void pasteFromClipboard();
    void syncOtherPanel();

public:
    ListPanelFunc(ListPanel *parent);
    ~ListPanelFunc();

    vfs* files();  // return a pointer to the vfs

    inline vfile* getVFile(KrViewItem *item) {
        return files()->getVfile(item->name());
    }
    inline vfile* getVFile(const QString& name) {
        return files()->getVfile(name);
    }

    void refreshActions();
    void redirectLink();
    void runService(const KService &service, QList<QUrl> urls);
    void displayOpenWithDialog(QList<QUrl> urls);
    QUrl browsableArchivePath(const QString &);

    ListPanelFunc* otherFunc();
    bool atHome();
    bool ignoreVFSErrors() { return _ignoreVFSErrors; }

protected slots:
    // Load the current url from history and refresh vfs and panel to it. If this fails, try the
    // next url in history until success (last try is root)
    void doRefresh();
    void slotFileCreated(KJob *job); // a file has been created by editNewFile()
    void historyGotoPos(int pos);
    void clipboardChanged(QClipboard::Mode mode);

protected:
    QUrl cleanPath(const QUrl &url);
    bool isSyncing(const QUrl &url);
    // when externallyExecutable == true, the file can be executed or opened with other software
    void openFileNameInternal(const QString &name, bool externallyExecutable);
    void openUrlInternal(const QUrl &url, const QString& makeCurrent,
                         bool immediately, bool disableLock, bool manuallyEntered);
    void runCommand(QString cmd);

    ListPanel*           panel;     // our ListPanel
    DirHistoryQueue*     history;
    vfs*                 vfsP;      // pointer to vfs.
    QTimer               delayTimer;
    QUrl                 syncURL;
    QUrl                 fileToCreate; // file that's to be created by editNewFile()
    bool                 urlManuallyEntered;

    static QPointer<ListPanelFunc> copyToClipboardOrigin;

private:
    bool _refreshing; // ignore url changes while refreshing
    bool _ignoreVFSErrors; // ignore (repeated) errors emitted by vfs;
};

#endif
