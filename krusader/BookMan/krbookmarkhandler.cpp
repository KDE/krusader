#include "krbookmarkhandler.h"
#include "kraddbookmarkdlg.h"
#include "../krusader.h"
#include "../krslots.h"
#include "../Dialogs/popularurls.h"
#include "../VFS/vfs.h"
#include <kiconloader.h>
#include <kmessagebox.h>
#include <qptrlist.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kdebug.h>
#include <kbookmarkmanager.h>
#include <kstandarddirs.h>
#include <qfile.h>

#define SPECIAL_BOOKMARKS	true

// ------------------------ for internal use
#define BOOKMARKS_FILE	"krusader/krbookmarks.xml"
#define CONNECT_BM(X)	{ disconnect(X, SIGNAL(activated(const KURL&)), 0, 0); connect(X, SIGNAL(activated(const KURL&)), this, SLOT(slotActivated(const KURL&))); }
											
KrBookmarkHandler::KrBookmarkHandler(): QObject(0), _middleClick(false) {
	// create our own action collection and make the shortcuts apply only to parent
	_privateCollection = new KActionCollection(krApp, "private collection");
	_collection = krApp->actionCollection();

	// create _root: father of all bookmarks. it is a dummy bookmark and never shown
	_root = new KrBookmark(i18n("Bookmarks"));

	// load bookmarks 
	importFromFile();

	// hack
	manager = KBookmarkManager::managerForFile(locateLocal( "data", BOOKMARKS_FILE ), false);
	connect(manager, SIGNAL(changed(const QString&, const QString& )), this, SLOT(bookmarksChanged(const QString&, const QString& )));
}

KrBookmarkHandler::~KrBookmarkHandler() {
	delete manager;
	delete _privateCollection;
}

void KrBookmarkHandler::menuOperation(int id) {
	switch (id) {
		case BookmarkCurrent:
			bookmarkCurrent(ACTIVE_PANEL->virtualPath());
			break;
		case ManageBookmarks:
			manager->slotEditBookmarks();
			break;
	}
}

void KrBookmarkHandler::bookmarkCurrent(KURL url) {
	KrAddBookmarkDlg dlg(krApp, url);
	if (dlg.exec() == KDialog::Accepted) {
		KrBookmark *bm = new KrBookmark(dlg.name(), dlg.url(), _collection);
		addBookmark(bm, dlg.folder());
	}
}

void KrBookmarkHandler::addBookmark(KrBookmark *bm, KrBookmark *folder) {
	if (folder == 0)
		folder = _root;
		
	// add to the list (bottom)
	folder->children().append(bm);

	exportToFile();
}

void KrBookmarkHandler::deleteBookmark(KrBookmark *bm) {
}

void KrBookmarkHandler::exportToFileBookmark(QDomDocument &doc, QDomElement &where, KrBookmark *bm) {
	QDomElement bookmark = doc.createElement("bookmark");
	bookmark.setAttribute("href", bm->url().url());
	// title
	QDomElement title = doc.createElement("title");
	title.appendChild(doc.createTextNode(bm->text()));
	bookmark.appendChild(title);

	where.appendChild(bookmark);
}

void KrBookmarkHandler::exportToFileFolder(QDomDocument &doc, QDomElement &parent, KrBookmark *folder) {
	for (KrBookmark *bm = folder->children().first(); bm; bm = folder->children().next()) {
		if (bm->isFolder()) {
			QDomElement newFolder = doc.createElement("folder");
			parent.appendChild(newFolder);
			QDomElement title = doc.createElement("title");
			title.appendChild(doc.createTextNode(bm->text()));
			newFolder.appendChild(title);
			exportToFileFolder(doc, newFolder, bm);
		} else {
			exportToFileBookmark(doc, parent, bm);
		}
	}
}

// export to file using the xbel standard
//
//  <xbel>
//    <bookmark href="http://developer.kde.org"><title>Developer Web Site</title></bookmark>
//    <folder folded="no">
//      <title>Title of this folder</title>
//      <bookmark icon="kde" href="http://www.kde.org"><title>KDE Web Site</title></bookmark>
//      <folder toolbar="yes">
//        <title>My own bookmarks</title>
//        <bookmark href="http://www.koffice.org"><title>KOffice Web Site</title></bookmark>
//        <separator/>
//        <bookmark href="http://www.kdevelop.org"><title>KDevelop Web Site</title></bookmark>
//      </folder>
//    </folder>
//  </xbel>
void KrBookmarkHandler::exportToFile() {
	QDomDocument doc( "xbel" );
   QDomElement root = doc.createElement( "xbel" );
   doc.appendChild( root );

	exportToFileFolder(doc, root, _root);
	QString filename = locateLocal( "data", BOOKMARKS_FILE );
	QFile file(filename);
	if ( file.open( IO_WriteOnly ) ) {
		QTextStream stream( &file );
		stream << doc.toString();
		file.close();
	} else {
		KMessageBox::error(krApp, i18n("Error"), i18n("Unable to write to ") + filename);
	}
}

bool KrBookmarkHandler::importFromFileBookmark(QDomElement &e, KrBookmark *parent, QString *errorMsg) {
	QString url, name, icon;
	// verify tag
	if (e.tagName() != "bookmark") {
		*errorMsg = e.tagName() + i18n(" instead of ")+"bookmark";
		return false;
	}
	// verify href
	if (!e.hasAttribute("href")) {
		*errorMsg = i18n("missing tag ")+ "href";
		return false;
	} else url = e.attribute("href");
	// verify title
	QDomElement te = e.firstChild().toElement();
	if (te.tagName() != "title") {
		*errorMsg = i18n("missing tag ")+"title";
		return false;
	} else name = te.text();
	// do we have an icon?
	if (e.hasAttribute("icon")) {
		icon=e.attribute("icon");
	}
	
	// ok: got name and url, let's add a bookmark
	KrBookmark *bm = KrBookmark::getExistingBookmark(name, _collection);
	if (!bm) 
		bm = new KrBookmark(name, vfs::fromPathOrURL( url ), _collection, icon);
	parent->children().append(bm);

	return true;
}

bool KrBookmarkHandler::importFromFileFolder(QDomNode &first, KrBookmark *parent, QString *errorMsg) {
	QString name;
	QDomNode n = first;
	while (!n.isNull()) {
		QDomElement e = n.toElement();
		if (e.tagName() == "bookmark") {
			if (!importFromFileBookmark(e, parent, errorMsg))
				return false;
		} else if (e.tagName() == "folder") {
			// the title is the first child of the folder
			QDomElement tmp = e.firstChild().toElement();
			if (tmp.tagName() != "title") {
				*errorMsg = i18n("missing tag ")+"title";
				return false;
			} else name = tmp.text();
			KrBookmark *folder = new KrBookmark(name);
			parent->children().append(folder);

			QDomNode nextOne = tmp.nextSibling();
			if (!importFromFileFolder(nextOne, folder, errorMsg))
				return false;
		} else if (e.tagName() == "separator") {
			parent->children().append(KrBookmark::separator());
		}
		n = n.nextSibling();
	}
	return true;
}


void KrBookmarkHandler::importFromFile() {
	clearBookmarks(_root);
	
	QString filename = locateLocal( "data", BOOKMARKS_FILE );
	QFile file( filename );
	if ( !file.open(IO_ReadOnly))
		return; // no bookmarks file

	QString errorMsg;
	QDomNode n;
	QDomElement e;
	QDomDocument doc( "xbel" );
	if ( !doc.setContent( &file, &errorMsg ) ) {
		goto ERROR;
	}
	// iterate through the document: first child should be "xbel" (skip all until we find it)
	n = doc.firstChild();	
	while (!n.isNull() && n.toElement().tagName()!="xbel")
		n = n.nextSibling();

	if (n.isNull() || n.toElement().tagName()!="xbel") {
		errorMsg = filename+i18n(" doesn't seem to be a valid Bookmarks file");
		goto ERROR;
	} else n = n.firstChild(); // skip the xbel part
	importFromFileFolder(n, _root, &errorMsg);
	goto SUCCESS;
	
ERROR:
	KMessageBox::error(krApp, "Error", i18n("Error reading bookmarks file: ")+errorMsg);

SUCCESS:
	file.close();
}

void KrBookmarkHandler::populate(KPopupMenu *menu) {
	menu->clear();
	buildMenu(_root, menu);
}

void KrBookmarkHandler::buildMenu(KrBookmark *parent, KPopupMenu *menu) {
	static int inSecondaryMenu = 0; // used to know if we're on the top menu
	int floc=0, bloc=0;
	
	if (!inSecondaryMenu) {
		// do we need to add special bookmarks?
		if (SPECIAL_BOOKMARKS) {
			// note: special bookmarks are not kept inside the _bookmarks list and added ad-hoc
			KrBookmark *bm = KrBookmark::devices(_collection);
			bm->plug(menu);
			CONNECT_BM(bm);
			bm = KrBookmark::lan(_collection);
			bm->plug(menu);
			CONNECT_BM(bm);
			bm = KrBookmark::virt(_collection);
			bm->plug(menu);
			CONNECT_BM(bm);
			floc = bloc = 3; // 3 bookmarks
		} 
		// add the popular links submenu
		KPopupMenu *newMenu = new KPopupMenu(menu);
		menu->insertItem(QIconSet(krLoader->loadIcon("bookmark_folder", KIcon::Small)),
									i18n("Popular URLs"), newMenu, -1 /* dummy id */, floc);
		// add the top 15 urls
		#define MAX 15
		KURL::List list = krApp->popularUrls->getMostPopularUrls(MAX);
		KURL::List::Iterator it;
		for (it = list.begin(); it != list.end(); ++it) {
			QString name;
			if ((*it).isLocalFile()) name = (*it).path();
			else name = (*it).prettyURL();
			// note: these bookmark are put into the private collection
			// as to not spam the general collection
			KrBookmark *bm = KrBookmark::getExistingBookmark(name, _privateCollection);
			if (!bm)
				bm = new KrBookmark(name, *it, _privateCollection);
			bm->plug(newMenu);
			CONNECT_BM(bm);
		}
		newMenu->insertSeparator();
		krPopularUrls->plug(newMenu);
		newMenu->installEventFilter(this);
		
		// finished with popular links
		menu->insertSeparator();
		floc += 2; bloc +=2; // 1 group + separator
	}

	for (KrBookmark *bm = parent->children().first(); bm; bm = parent->children().next()) {
		if (bm->isSeparator()) { // separator
			menu->insertSeparator(bloc++);
		} else if (bm->isFolder()) {
			KPopupMenu *newMenu = new KPopupMenu(menu);
			// add folders above bookmarks
			menu->insertItem(QIconSet(krLoader->loadIcon("bookmark_folder", KIcon::Small)),
									bm->text(), newMenu, -1 /* dummy id */, floc++);
			++bloc; // stuffed a folder in the middle
			++inSecondaryMenu;
			buildMenu(bm, newMenu);
			--inSecondaryMenu;
		} else { // ordinary bookmark
			bm->plug(menu, bloc++);
			CONNECT_BM(bm);
		}
	}

	if (!inSecondaryMenu) {
		menu->insertSeparator();
		menu->insertItem(krLoader->loadIcon("bookmark_add", KIcon::Small),
			i18n("Bookmark Current"), BookmarkCurrent);
		menu->insertItem(krLoader->loadIcon("bookmark", KIcon::Small),
			i18n("Manage Bookmarks"), ManageBookmarks);
	
		// make sure the menu is connected to us
		disconnect(menu, SIGNAL(activated(int)), 0, 0);
		connect(menu, SIGNAL(activated(int)), this, SLOT(menuOperation(int)));
	}

	menu->installEventFilter(this);
}


void KrBookmarkHandler::clearBookmarks(KrBookmark *root) {
	KrBookmark *bm = root->children().first();
	while (bm) {	
		if (bm->isFolder())
			clearBookmarks(bm);
		else bm->unplugAll();

		bm = root->children().next();
	}
	root->children().clear();
}

void KrBookmarkHandler::bookmarksChanged(const QString&, const QString&) {
	importFromFile();
}

bool KrBookmarkHandler::eventFilter( QObject *obj, QEvent *ev ) {
	if (ev->type() == QEvent::MouseButtonRelease) {
		switch (static_cast<QMouseEvent*>(ev)->button()) {
			case LeftButton:
			case RightButton:
				_middleClick = false;
				break;
			case MidButton:
				_middleClick = true;
				break;
			default:
				break;
		}
	}
	return QObject::eventFilter(obj, ev);
}

// used to monitor middle clicks. if mid is found, then the
// bookmark is opened in a new tab. ugly, but easier than overloading
// KAction and KActionCollection.
void KrBookmarkHandler::slotActivated(const KURL& url) {
	if (_middleClick)
		SLOTS->newTab(url);
	else SLOTS->refresh(url);
}


#include "krbookmarkhandler.moc"
