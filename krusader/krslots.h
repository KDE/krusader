/*****************************************************************************
 * Copyright (C) 2001 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2001 Rafi Yanai <krusader@users.sourceforge.net>            *
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
        connect(this, QOverload<int, QProcess::ExitStatus>::of(&KrProcess::finished), this, &KrProcess::processHasExited);
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
    ~KRslots() override {}

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
