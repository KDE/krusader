/***************************************************************************
                                 krsearchdialog.h
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

#ifndef KRSEARCHDIALOG_H
#define KRSEARCHDIALOG_H

#include <sys/types.h>
#include <time.h>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QDialog>

#include "krsearchmod.h"


class QGridLayout;
class QLabel;
class SearchResultContainer;
class KrView;
class KrViewItem;
class ProfileManager;
class FilterTabs;
class GeneralFilter;
class KTabWidget;
class KSqueezedTextLabel;
class KrSqueezedTextLabel;

class KrSearchDialog : public QDialog
{
    Q_OBJECT
public:
    KrSearchDialog(QString profile = QString(), QWidget* parent = 0);
    ~KrSearchDialog();

    void prepareGUI();

    static KrSearchDialog *SearchDialog;

public slots:
    void startSearch();
    void stopSearch();
    void feedToListBox();
    void copyToClipBoard();
    void found(QString what, QString where, KIO::filesize_t size, time_t mtime, QString perm, QString foundText);
    void closeDialog(bool isAccept = true);
    void executed(const QString &name);
    void currentChanged(KrViewItem *item);

    virtual void keyPressEvent(QKeyEvent *e);
    virtual void closeEvent(QCloseEvent *e);
    virtual void contextMenu(const QPoint &);
    virtual void resizeEvent(QResizeEvent *e);

protected slots:
    void reject();

private:
    bool gui2query();
    void editCurrent();
    void viewCurrent();
    void compareByContent();

private:
    ProfileManager *profileManager;

    FilterTabs * filterTabs;
    GeneralFilter * generalFilter;

    QPushButton* mainSearchBtn;
    QPushButton* mainStopBtn;
    QPushButton* mainCloseBtn;
    QPushButton* mainFeedToListBoxBtn;

    KTabWidget* searcherTabs;
    QLabel* foundLabel;
    KrSqueezedTextLabel *foundTextLabel;
    KSqueezedTextLabel *searchingLabel;

    SearchResultContainer *result;
    KrView *resultView;

    KRQuery *query;
    KRSearchMod *searcher;
    QStringList savedSearches;
    bool isBusy;
    bool closed;

    static QString lastSearchText;
    static int     lastSearchType;
    static bool    lastSearchForCase;
    static bool    lastRemoteContentSearch;
    static bool    lastContainsWholeWord;
    static bool    lastContainsWithCase;
    static bool    lastSearchInSubDirs;
    static bool    lastSearchInArchives;
    static bool    lastFollowSymLinks;
    static bool    lastContainsRegExp;

    int            sizeX;
    int            sizeY;
};

#endif
