/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DISKUSAGE_H
#define DISKUSAGE_H

// QtCore
#include <QEvent>
#include <QHash>
#include <QStack>
#include <QTimer>
#include <QUrl>
// QtGui
#include <QKeyEvent>
#include <QPixmap>
#include <QResizeEvent>
// QtWidgets
#include <QDialog>
#include <QLabel>
#include <QScrollArea>
#include <QStackedWidget>

#include <KSqueezedTextLabel>

#include "filelightParts/fileTree.h"

#define VIEW_LINES 0
#define VIEW_DETAILED 1
#define VIEW_FILELIGHT 2
#define VIEW_LOADER 3

typedef QHash<QString, void *> Properties;

class DUListView;
class DULines;
class DUFilelight;
class QMenu;
class LoaderWidget;
class FileItem;
class FileSystem;

class DiskUsage : public QStackedWidget
{
    Q_OBJECT

public:
    explicit DiskUsage(QString confGroup, QWidget *parent = nullptr);
    ~DiskUsage();

    void load(const QUrl &dirName);
    void close();
    void stopLoad();
    bool isLoading()
    {
        return loading;
    }

    void setView(int view);
    int getActiveView()
    {
        return activeView;
    }

    Directory *getDirectory(QString path);
    File *getFile(const QString &path);

    QString getConfigGroup()
    {
        return configGroup;
    }

    void *getProperty(File *, const QString &);
    void addProperty(File *, const QString &, void *);
    void removeProperty(File *, const QString &);

    int exclude(File *file, bool calcPercents = true, int depth = 0);
    void includeAll();

    int del(File *file, bool calcPercents = true, int depth = 0);

    QString getToolTip(File *);

    void rightClickMenu(const QPoint &, File *, QMenu * = nullptr, const QString & = QString());

    void changeDirectory(Directory *dir);

    Directory *getCurrentDir();
    File *getCurrentFile();

    QPixmap getIcon(const QString &mime);

    QUrl getBaseURL()
    {
        return baseURL;
    }

public slots:
    void dirUp();
    void clear();

signals:
    void enteringDirectory(Directory *);
    void clearing();
    void changed(File *);
    void changeFinished();
    void deleted(File *);
    void deleteFinished();
    void status(QString);
    void viewChanged(int);
    void loadFinished(bool);
    void newSearch();

protected slots:
    void slotLoadDirectory();

protected:
    QHash<QString, Directory *> contentMap;
    QHash<File *, Properties *> propertyMap;

    Directory *currentDirectory;
    KIO::filesize_t currentSize;

    virtual void keyPressEvent(QKeyEvent *) override;
    virtual bool event(QEvent *) override;

    int calculateSizes(Directory *dir = nullptr, bool emitSig = false, int depth = 0);
    int calculatePercents(bool emitSig = false, Directory *dir = nullptr, int depth = 0);
    int include(Directory *dir, int depth = 0);
    void createStatus();
    void executeAction(int, File * = nullptr);

    QUrl baseURL; //< the base URL of loading

    DUListView *listView;
    DULines *lineView;
    DUFilelight *filelightView;
    LoaderWidget *loaderView;

    Directory *root;

    int activeView;

    QString configGroup;

    bool first;
    bool loading;
    bool abortLoading;
    bool clearAfterAbort;
    bool deleting;

    QStack<QString> directoryStack;
    QStack<Directory *> parentStack;

    FileSystem *searchFileSystem;
    FileItem *currentFileItem;
    QList<FileItem *> fileItems;
    Directory *currentParent;
    QString dirToCheck;

    int fileNum;
    int dirNum;
    int viewBeforeLoad;

    QTimer loadingTimer;
};

class LoaderWidget : public QScrollArea
{
    Q_OBJECT

public:
    explicit LoaderWidget(QWidget *parent = nullptr);

    void init();
    void setCurrentURL(const QUrl &url);
    void setValues(int fileNum, int dirNum, KIO::filesize_t total);
    bool wasCancelled()
    {
        return cancelled;
    }

public slots:
    void slotCancelled();

protected:
    QLabel *totalSize;
    QLabel *files;
    QLabel *directories;

    KSqueezedTextLabel *searchedDirectory;
    QWidget *widget;

    bool cancelled;
};

#endif /* __DISK_USAGE_GUI_H__ */
