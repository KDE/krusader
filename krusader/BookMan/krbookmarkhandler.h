/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#ifndef KRBOOKMARKHANDLER_H
#define KRBOOKMARKHANDLER_H

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QEvent>
#include <qdom.h>
#include <qmap.h>

#include <kmenu.h>
#include <kurl.h>

#include "krbookmark.h"

class KActionCollection;
class KBookmarkManager;
class FileManagerWindow;

class KrBookmarkHandler: public QObject
{
    Q_OBJECT
    friend class KrAddBookmarkDlg;
    enum Actions { BookmarkCurrent = 0, ManageBookmarks };
public:
    KrBookmarkHandler(FileManagerWindow *mainWindow);
    ~KrBookmarkHandler();
    void populate(KMenu *menu);
    void addBookmark(KrBookmark *bm, KrBookmark *parent = 0);
    void bookmarkCurrent(KUrl url);

protected:
    void deleteBookmark(KrBookmark *bm);
    void importFromFile();
    bool importFromFileBookmark(QDomElement &e, KrBookmark *parent, QString path, QString *errorMsg);
    bool importFromFileFolder(QDomNode &first, KrBookmark *parent, QString path, QString *errorMsg);
    void exportToFile();
    void exportToFileFolder(QDomDocument &doc, QDomElement &parent, KrBookmark *folder);
    void exportToFileBookmark(QDomDocument &doc, QDomElement &where, KrBookmark *bm);
    void clearBookmarks(KrBookmark *root);
    void buildMenu(KrBookmark *parent, KMenu *menu);

    bool eventFilter(QObject *obj, QEvent *ev);

    void rightClicked(QMenu *menu, KrBookmark *bm);
    void rightClickOnSpecialBookmark();

    void removeReferences(KrBookmark *root, KrBookmark *bmToRemove);

protected slots:
    void slotBookmarkCurrent();
    void bookmarksChanged(const QString&, const QString&);
    void slotActivated(const KUrl& url);

private:
    FileManagerWindow *_mainWindow;
    KActionCollection *_collection, *_privateCollection;
    KrBookmark *_root;
    // the whole KBookmarkManager is an ugly hack. use it until we have our own
    KBookmarkManager *manager;
    bool _middleClick; // if true, the user clicked the middle button to open the bookmark

    QPointer<KMenu>            _mainBookmarkPopup; // main bookmark popup menu
    QList<QAction *>           _specialBookmarks; // the action list of the special bookmarks
};

Q_DECLARE_METATYPE(KrBookmark *)

#endif // KRBOOKMARK_HANDLER_H
