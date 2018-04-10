/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
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

#ifndef KRBOOKMARKHANDLER_H
#define KRBOOKMARKHANDLER_H

// QtCore
#include <QObject>
#include <QPointer>
#include <QEvent>
#include <QMap>
#include <QUrl>
// QtXml
#include <QDomEntity>
// QtWidgets
#include <QMenu>
#include <QWidgetAction>
#include <QLineEdit>

#include "krbookmark.h"

class KActionCollection;
class KBookmarkManager;
class KrMainWindow;

class KrBookmarkHandler: public QObject
{
    Q_OBJECT
    friend class KrAddBookmarkDlg;
    enum Actions { BookmarkCurrent = 0, ManageBookmarks };
public:
    explicit KrBookmarkHandler(KrMainWindow *mainWindow);
    ~KrBookmarkHandler();
    void populate(QMenu *menu);
    void addBookmark(KrBookmark *bm, KrBookmark *parent = 0);
    void bookmarkCurrent(QUrl url);

protected:
    void deleteBookmark(KrBookmark *bm);
    void importFromFile();
    bool importFromFileBookmark(QDomElement &e, KrBookmark *parent, QString path, QString *errorMsg);
    bool importFromFileFolder(QDomNode &first, KrBookmark *parent, QString path, QString *errorMsg);
    void exportToFile();
    void exportToFileFolder(QDomDocument &doc, QDomElement &parent, KrBookmark *folder);
    void exportToFileBookmark(QDomDocument &doc, QDomElement &where, KrBookmark *bm);
    void clearBookmarks(KrBookmark *root);
    void buildMenu(KrBookmark *parent, QMenu *menu, int depth = 0);

    bool eventFilter(QObject *obj, QEvent *ev);
    void rightClicked(QMenu *menu, KrBookmark *bm);
    void rightClickOnSpecialBookmark();

    void removeReferences(KrBookmark *root, KrBookmark *bmToRemove);

protected slots:
    void bookmarksChanged(const QString&, const QString&);
    void slotActivated(const QUrl &url);

private:
    KrMainWindow *_mainWindow;
    KActionCollection *_collection, *_privateCollection;
    KrBookmark *_root;
    // the whole KBookmarkManager is an ugly hack. use it until we have our own
    KBookmarkManager *manager;
    bool _middleClick; // if true, the user clicked the middle button to open the bookmark

    QPointer<QMenu>            _mainBookmarkPopup; // main bookmark popup menu
    QList<QAction *>           _specialBookmarks; // the action list of the special bookmarks

    QWidgetAction *_quickSearchAction; ///< Search bar container action
    QLineEdit *_quickSearchBar; ///< Search bar containing current query
    QMenu *_quickSearchMenu; ///< The menu where the search is performed
    QHash<QAction *, QString> _quickSearchOriginalActionTitles; ///< Saved original action text values to restore after search

    void _setQuickSearchText(const QString &text);
    QString _quickSearchText() const;
    static void _highlightAction(QAction *action, bool isMatched = true);
    void _resetActionTextAndHighlighting();
};

Q_DECLARE_METATYPE(KrBookmark *)

#endif // KRBOOKMARK_HANDLER_H
