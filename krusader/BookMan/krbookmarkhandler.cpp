#include "krbookmarkhandler.h"
#include "krbookmark.h"
#include "kraddbookmarkdlg.h"
#include "../krusader.h"
#include "../krslots.h"
#include <kiconloader.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kdebug.h>

#define SPECIAL_BOOKMARKS	true

#define CONNECT_BM(X)	connect(X, SIGNAL(activated(KrBookmark *)),					\
											this, SLOT(bookmarkActivated(KrBookmark *)));

KrBookmarkHandler::KrBookmarkHandler(QWidget *parent, KPopupMenu *menu): QObject(parent), _menu(menu) {
	// create our own action collection and make the shortcuts apply only to parent
	_collection = new KActionCollection(0, parent);

	_menu->setKeyboardShortcutsEnabled(true);
	_menu->setKeyboardShortcutsExecute(true);
	connect(_menu, SIGNAL(activated(int)), this, SLOT(menuOperation(int)));
	
	// do we need to add special bookmarks?
	if (SPECIAL_BOOKMARKS) {
		_menu->insertSeparator();
		KrBookmark *bm = KrBookmark::devices(_collection);
		bm->plug(_menu);
		CONNECT_BM(bm);
		
		_menu->insertSeparator();
	}
	_menu->insertItem(krLoader->loadIcon("bookmark_add", KIcon::Small),
		i18n("Bookmark Current"), BookmarkCurrent);
	_menu->insertItem(krLoader->loadIcon("bookmark", KIcon::Small),
		i18n("Manage Bookmarks"), ManageBookmarks);
}

void KrBookmarkHandler::bookmarkActivated(KrBookmark *bm) {
	emit openUrl(bm->url());
}

void KrBookmarkHandler::menuOperation(int id) {
	switch (id) {
		case BookmarkCurrent:
			bookmarkCurrent(ACTIVE_PANEL->virtualPath);
			break;
		case ManageBookmarks:
			kdWarning() << "manage not implemented yet" << endl;
			break;
	}
}

void KrBookmarkHandler::bookmarkCurrent(KURL url) {
	KrAddBookmarkDlg dlg(krApp, url);
	if (dlg.exec() == KDialog::Accepted) {
		KrBookmark *bm = new KrBookmark(dlg.name(), dlg.url(), _collection);
		bm->plug(_menu, 0); // add on top
		CONNECT_BM(bm);
	}
}


