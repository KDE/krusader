/*
    SPDX-FileCopyrightText: 2001 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRSLOTS_H
#define KRSLOTS_H

// QtCore
#include <QFile>
#include <QObject>
#include <QUrl>

#include <KProcess>
#include <utility>

class KrMainWindow;
class QUrl;

class KrProcess : public KProcess
{
    Q_OBJECT

    QString tmp1, tmp2;

public:
    KrProcess(QString in1, QString in2)
    {
        tmp1 = std::move(in1);
        tmp2 = std::move(in2);
        connect(this, QOverload<int, QProcess::ExitStatus>::of(&KrProcess::finished), this, &KrProcess::processHasExited);
    }

public slots:
    void processHasExited()
    {
        if (!tmp1.isEmpty())
            QFile::remove(tmp1);
        if (!tmp2.isEmpty())
            QFile::remove(tmp2);
        deleteLater();
    }
};

class KrSlots : public QObject
{
    Q_OBJECT

public:
    enum compareMode { full };

    explicit KrSlots(QObject *parent);
    ~KrSlots() override = default;

public slots:
    void sendFileByEmail(const QList<QUrl> &filename);
    void compareContent();
    void compareContent(const QUrl &, const QUrl &);
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
    void startKonfigurator()
    {
        runKonfigurator(false);
    }
    void search(); // call the search module
    void locate();
    void runTerminal(const QString &dir);
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

protected slots:
    void configChanged(bool isGUIRestartNeeded);

protected:
    KrMainWindow *_mainWindow;
};

#endif
