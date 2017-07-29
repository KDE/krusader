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

// QtCore
#include <QFile>
#include <QObject>
#include <QUrl>

#include <KCoreAddons/KProcess>

class KrMainWindow;
class QUrl;

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
            QFile::remove(tmp1);
        if (!tmp2.isEmpty())
            QFile::remove(tmp2);
        deleteLater();
    }
};

class KRslots : public QObject
{
    Q_OBJECT

public:
    enum compareMode { full };

    explicit KRslots(QObject *parent);
    ~KRslots() {}

public slots:
    void sendFileByEmail(const QList<QUrl> &filename);
    void compareContent();
    void compareContent(QUrl, QUrl);
    void insertFileName(bool fullPath);
    void rootKrusader();
    void swapPanels();
    void showHiddenFiles(bool show);
    void toggleSwapSides();
    void updateStatusbarVisibility();
    void toggleTerminal();
    void compareSetup();
    void emptyTrash();
    void trashPopupMenu();
    /** called by actExec* actions to choose the built-in command line mode */
    void execTypeSetup();
    void refresh(const QUrl &u);
    void runKonfigurator(bool firstTime = false);
    void startKonfigurator() {
        runKonfigurator(false);
    }
    void search();           // call the search module
    void locate();
    void runTerminal(const QString & dir);
    void homeTerminal();
    void addBookmark();
    void toggleFnkeys();
    void toggleCmdline();
    void multiRename();
    void cmdlinePopup();
    void slotSplit();
    void slotCombine();
    void manageUseractions();
#ifdef SYNCHRONIZER_ENABLED
    void slotSynchronizeDirs(QStringList selected = QStringList());
#endif
    void slotDiskUsage();
    void applicationStateChanged();
    void jsConsole();

protected slots:
    void configChanged(bool isGUIRestartNeeded);

protected:
    KrMainWindow *_mainWindow;
};

#endif
