/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PANELFUNC_H
#define PANELFUNC_H

// QtCore
#include <QObject>
#include <QTimer>
#include <QUrl>
// QtGui
#include <QClipboard>

#include <KIO/Global>
#include <KJob>
#include <KService>

#include "../FileSystem/sizecalculator.h"

class DirHistoryQueue;
class FileItem;
class FileSystem;
class KrViewItem;
class ListPanel;

class ListPanelFunc : public QObject
{
    friend class ListPanel;
    Q_OBJECT
public slots:
    void execute(const QString &);
    void goInside(const QString &);
    void openUrl(const QUrl &url, const QString &nameToMakeCurrent = QString(), bool manuallyEntered = false);
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
    void editFile(const QUrl &filePath = QUrl());
    /** Ask for a filename; if it doesn't exist, create it; edit it */
    void askEditFile();
    void moveFilesDelayed()
    {
        moveFiles(true);
    }
    void copyFilesDelayed()
    {
        copyFiles(true);
    }
    void moveFiles(bool enqueue = false)
    {
        copyFiles(enqueue, true);
    }
    void copyFiles(bool enqueue = false, bool move = false);

    /*!
     * asks the user the new directory name
     */
    void mkdir();
    /** Delete or move selected files to trash - depending on user setting. */
    void defaultDeleteFiles()
    {
        defaultOrAlternativeDeleteFiles(false);
    }
    /** Delete or move selected files to trash - inverting the user setting. */
    void alternativeDeleteFiles()
    {
        defaultOrAlternativeDeleteFiles(true);
    }
    /** Delete virtual files or directories in virtual filesystem. */
    void removeVirtualFiles();
    void rename();
    void krlink(bool sym = true);
    void createChecksum();
    void matchChecksum();
    void pack();
    void unpack();
    void testArchive();
    /** Calculate the occupied space of the currently selected items and show a dialog. */
    void calcSpace();
    /** Calculate the occupied space of the current item without dialog. */
    void quickCalcSpace();
    void properties();
    void cut()
    {
        copyToClipboard(true);
    }
    void copyToClipboard(bool move = false);
    void pasteFromClipboard();
    void syncOtherPanel();
    /** Disable refresh if panel is not visible. */
    void setPaused(bool paused);

public:
    explicit ListPanelFunc(ListPanel *parent);
    ~ListPanelFunc() override;

    FileSystem *files(); // return a pointer to the filesystem
    QUrl virtualDirectory(); // return the current URL (simulated when panel is paused)

    FileItem *getFileItem(KrViewItem *item);
    FileItem *getFileItem(const QString &name);

    void refreshActions();
    void redirectLink();
    void runService(const KService &service, const QList<QUrl> &urls);
    void displayOpenWithDialog(const QList<QUrl> &urls);
    QUrl browsableArchivePath(const QString &);
    void deleteFiles(bool moveToTrash);

    ListPanelFunc *otherFunc();
    bool atHome();

    /** Ask user for confirmation before deleting files. Returns only confirmed files.*/
    static QList<QUrl> confirmDeletion(const QList<QUrl> &urls, bool moveToTrash, bool virtualFS, bool showPath);

protected slots:
    // Load the current url from history and refresh filesystem and panel to it
    void doRefresh();
    void slotFileCreated(KJob *job, const QUrl filePath); // a file has been created by askEditFile() or slotStatEdit()
    void historyGotoPos(int pos);
    void clipboardChanged(QClipboard::Mode mode);
    // Update the directory size in view
    void slotSizeCalculated(const QUrl &url, KIO::filesize_t size);
    // Get some information about a file that is going to be edited
    // and perform some steps before the edition
    void slotStatEdit(KJob *job);

protected:
    QUrl cleanPath(const QUrl &url);
    bool isSyncing(const QUrl &url);
    // when externallyExecutable == true, the file can be executed or opened with other software
    void openFileNameInternal(const QString &name, bool externallyExecutable);
    void openUrlInternal(const QUrl &url, const QString &nameToMakeCurrent, bool manuallyEntered);
    void runCommand(const QString &cmd);

    ListPanel *panel; // our ListPanel
    DirHistoryQueue *history;
    FileSystem *fileSystemP; // pointer to fileSystem.
    QTimer delayTimer;
    QUrl syncURL;
    bool urlManuallyEntered;

    static QPointer<ListPanelFunc> copyToClipboardOrigin;

private:
    void defaultOrAlternativeDeleteFiles(bool invert);
    bool getSelectedFiles(QStringList &args);
    SizeCalculator *createAndConnectSizeCalculator(const QList<QUrl> &urls);
    bool _isPaused; // do not refresh while panel is not visible
    bool _refreshAfterPaused; // refresh after not paused anymore
    QPointer<SizeCalculator> _quickSizeCalculator;
};

#endif
