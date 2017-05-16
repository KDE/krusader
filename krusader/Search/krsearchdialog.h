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

// QtCore
#include <QString>
#include <QStringList>
// QtWidgets
#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QTabWidget>

class FileItem;
class FilterTabs;
class GeneralFilter;
class KrSearchBar;
class KrSqueezedTextLabel;
class KrView;
class KrViewItem;
class KRQuery;
class KRSearchMod;
class KSqueezedTextLabel;
class ProfileManager;
class SearchResultContainer;

class KrSearchDialog : public QDialog
{
    Q_OBJECT
public:
    explicit KrSearchDialog(QString profile = QString(), QWidget* parent = 0);
    ~KrSearchDialog();

    void prepareGUI();

    static KrSearchDialog *SearchDialog;

public slots:
    void startSearch();
    void stopSearch();
    void feedToListBox();
    void copyToClipBoard();
    void slotFound(const FileItem &file, const QString &foundText);
    void closeDialog(bool isAccept = true);
    void executed(const QString &name);
    void currentChanged(KrViewItem *item);
    void contextMenu(const QPoint &);

    virtual void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    virtual void closeEvent(QCloseEvent *e) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;

protected slots:
    void reject();

private:
    bool gui2query();
    void editCurrent();
    void viewCurrent();
    void compareByContent();

    /**
     * Placing search query to clipboard is optional (opt-in).
     * So user has clipboard untact by default when opening found documents,
     * but can enable it persistently by checking "Query to clipboard" checkbox.
     */
    void tryPlaceSearchQueryToClipboard();

private:
    ProfileManager *profileManager;
    QCheckBox *searchTextToClipboard;

    FilterTabs * filterTabs;
    GeneralFilter * generalFilter;

    QPushButton* mainSearchBtn;
    QPushButton* mainStopBtn;
    QPushButton* mainCloseBtn;
    QPushButton* mainFeedToListBoxBtn;

    QTabWidget* searcherTabs;
    QLabel* foundLabel;
    KrSqueezedTextLabel *foundTextLabel;
    KSqueezedTextLabel *searchingLabel;

    SearchResultContainer *result;
    KrView *resultView;
    KrSearchBar *searchBar;

    KRQuery *query;
    KRSearchMod *searcher;
    bool isBusy;
    bool closed;

    static QString lastSearchText;
    static int     lastSearchType;
    static bool    lastSearchForCase;
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
