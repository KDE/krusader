#include "krbookmarkhandler.h"
#include "krbookmark.h"
#include "../krusader.h"
#include <kiconloader.h>
#include <kactioncollection.h>
#include <klocale.h>

#define SPECIAL_BOOKMARKS	true

KrBookmarkHandler::KrBookmarkHandler(QWidget *parent, KPopupMenu *menu): QObject(parent), _menu(menu) {
	// create our own action collection and make the shortcuts apply only to parent
	_collection = new KActionCollection(0, parent);
	// do we need to add special bookmarks?
	if (SPECIAL_BOOKMARKS) {
		_menu->insertSeparator();
		KrBookmark::devices(_collection)->plug(_menu);
		_menu->insertSeparator();
	}
	_menu->insertItem(krLoader->loadIcon("bookmark_add", KIcon::Small),
		i18n("Bookmark Current"), BookmarkCurrent);
	KPopupMenu *newBookmarkMenu = new KPopupMenu(_menu);
	newBookmarkMenu->insertItem(krLoader->loadIcon("hdd_mount", KIcon::Small), "Local Bookmark", NewLocal);
	newBookmarkMenu->insertItem(krLoader->loadIcon("network", KIcon::Small), "Remote Bookmark", NewRemote);
	_menu->insertItem("Add New Bookmark", newBookmarkMenu);
	_menu->insertItem(krLoader->loadIcon("bookmark", KIcon::Small),
		i18n("Edit Bookmarks"), EditBookmarks);
	_menu->insertItem(krLoader->loadIcon("folder_new", KIcon::Small),
		i18n("New Folder ..."), NewFolder);
}

