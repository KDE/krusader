/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRBOOKMARKHANDLER_H
#define KRBOOKMARKHANDLER_H

// QtCore
#include <QEvent>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QUrl>
// QtXml
#include <QDomEntity>
// QtWidgets
#include <QLineEdit>
#include <QMenu>
#include <QWidgetAction>

#include "krbookmark.h"

class KActionCollection;
class KBookmarkManager;
class KBookmarkMenu;
class KrMainWindow;

class KrBookmarkHandler : public QObject
{
    Q_OBJECT
    friend class KrAddBookmarkDlg;
    enum Actions { BookmarkCurrent = 0, ManageBookmarks };

public:
    explicit KrBookmarkHandler(KrMainWindow *mainWindow);
    ~KrBookmarkHandler() override;
    void populate(QMenu *menu);
    void addBookmark(KrBookmark *bm, KrBookmark *parent = nullptr);
    void bookmarkCurrent(QUrl url);

protected:
    void deleteBookmark(KrBookmark *bm);
    void importFromFile();
    bool importFromFileBookmark(QDomElement &e, KrBookmark *parent, const QString &path, QString *errorMsg);
    bool importFromFileFolder(QDomNode &first, KrBookmark *parent, const QString &path, QString *errorMsg);
    void exportToFile();
    void exportToFileFolder(QDomDocument &doc, QDomElement &parent, KrBookmark *folder);
    void exportToFileBookmark(QDomDocument &doc, QDomElement &where, KrBookmark *bm);
    void clearBookmarks(KrBookmark *root, bool removeBookmarks = true);
    void buildMenu(KrBookmark *parent, QMenu *menu, int depth = 0);

    bool eventFilter(QObject *obj, QEvent *ev) override;
    void rightClicked(QMenu *menu, KrBookmark *bm);
    void rightClickOnSpecialBookmark();

    void removeReferences(KrBookmark *root, KrBookmark *bmToRemove);

protected slots:
    void bookmarksChanged(const QString &);
    void slotActivated(const QUrl &url);

private:
    KrMainWindow *_mainWindow;
    KActionCollection *_collection, *_privateCollection;
    KrBookmark *_root;
    // the whole KBookmarkManager is an ugly hack. use it until we have our own
    KBookmarkManager *manager;
    KBookmarkMenu *bookmarksMenu;
    bool _middleClick; // if true, the user clicked the middle button to open the bookmark

    QPointer<QMenu> _mainBookmarkPopup; // main bookmark popup menu
    QList<QAction *> _specialBookmarks; // the action list of the special bookmarks

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
