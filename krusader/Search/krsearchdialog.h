/*****************************************************************************
 * Copyright (C) 2001 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2001 Rafi Yanai <krusader@users.sourceforge.net>            *
 * Copyright (C) 2004-2020 Krusader Krew [https://krusader.org]              *
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
class KrQuery;
class KrSearchMod;
class KSqueezedTextLabel;
class ProfileManager;
class SearchResultContainer;

class KrSearchDialog : public QDialog
{
    Q_OBJECT
public:
    explicit KrSearchDialog(const QString& profile = QString(), QWidget* parent = nullptr);
    ~KrSearchDialog() override;

    void prepareGUI();

    static KrSearchDialog *SearchDialog;

public slots:
    void startSearch();
    void stopSearch();
    void feedToListBox();
    void slotFound(const FileItem &file, const QString &foundText);
    void closeDialog(bool isAccept = true);
    void executed(const QString &name);
    void currentChanged(KrViewItem *item);
    void contextMenu(const QPoint &);

    void keyPressEvent(QKeyEvent *e) override;
    void closeEvent(QCloseEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

protected slots:
    void reject() override;

private slots:
    void editCurrent();
    void viewCurrent();
    void compareByContent();
    void copyToClipBoard();

private:
    bool gui2query();

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
    QFrame *foundTextFrame;

    KrQuery *query;
    KrSearchMod *searcher;
    bool isBusy;
    bool closed;

    QAction *viewAction;
    QAction *editAction;
    QAction *compareAction;
    QAction *copyAction;

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
