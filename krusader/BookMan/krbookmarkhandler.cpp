#include "krbookmarkhandler.h"
#include "kraddbookmarkdlg.h"
#include "../krusader.h"
#include "../krslots.h"
#include <kiconloader.h>
#include <kmessagebox.h>
#include <qptrlist.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <qfile.h>
#include <kdirwatch.h>

#define SPECIAL_BOOKMARKS	true

// ------------------------ for internal use
#define BOOKMARKS_FILE	"krusader/bookman2.xml"
#define TOP_OF_MENU		0
#define BOTTOM_OF_MENU	_menu->indexOf(Border)
#define CONNECT_BM(X)	connect(X, SIGNAL(activated(KrBookmark *)), this, SLOT(bookmarkActivated(KrBookmark *)));
#define STOPWATCH		_dirwatch->stopDirScan(_filename)
#define STARTWATCH	_dirwatch->restartDirScan(_filename)
											
KrBookmarkHandler::KrBookmarkHandler(): QObject(0) {
	// create our own action collection and make the shortcuts apply only to parent
	_collection = new KActionCollection(0, this, "bookmark collection");

	// create _root: father of all bookmarks. it is a dummy bookmark and never shown
	_root = new KrBookmark("root");

	// load bookmarks and populate the menu
	_filename = locateLocal( "data", BOOKMARKS_FILE );
	importFromFile();
}

KrBookmarkHandler::KrBookmarkHandler(QWidget *parent, KPopupMenu *menu): QObject(parent), _menu(menu) {
// create our own action collection and make the shortcuts apply only to parent
	_collection = new KActionCollection(0, parent);
	
	// create _root: father of all bookmarks. it is a dummy bookmark and never shown
	_root = new KrBookmark("root");

	// start watching the bookmarks file for updates
	_filename = locateLocal( "data", BOOKMARKS_FILE );
	_dirwatch = new KDirWatch(this);
	//_dirwatch->addFile(locateLocal("data", BOOKMARKS_FILE));
	connect(_dirwatch, SIGNAL(dirty(const QString &)), this, SLOT(bookmarksUpdated(const QString &)));
	connect(_dirwatch, SIGNAL(created(const QString &)), this, SLOT(bookmarksUpdated(const QString &)));

	// add quick navigation
	_menu->setKeyboardShortcutsEnabled(true);
	_menu->setKeyboardShortcutsExecute(true);
	connect(_menu, SIGNAL(activated(int)), this, SLOT(menuOperation(int)));
	
	// load bookmarks and populate the menu
	importFromFile();

	
	// border: a dummy item used to separate normal bookmarks from special ones,
	// operations etc. we use it later when inserting bookmarks at the bottom
	_menu->insertItem("border-dummy", Border);
	_menu->setItemVisible(Border, false);

	// do we need to add special bookmarks?
	if (SPECIAL_BOOKMARKS) {
		// note: special bookmarks are not kept inside the _bookmarks list and added ad-hoc
		_menu->insertSeparator();
		KrBookmark *bm = KrBookmark::devices(_collection);
		bm->plug(_menu);
		CONNECT_BM(bm);
	}
	_menu->insertSeparator();
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
		addBookmark(bm);
	}
}

void KrBookmarkHandler::addBookmark(KrBookmark *bm, bool saveData, KrBookmark *folder) {
	if (folder == 0)
		folder = _root;
		
	// add to the list (bottom)
	folder->children().append(bm);

	// add to menu
	bm->plug(_menu, BOTTOM_OF_MENU);
	CONNECT_BM(bm);

	if (saveData) // save
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
	// disable the dirwatch while saving the file
	STOPWATCH;

	QDomDocument doc( "xbel" );
   QDomElement root = doc.createElement( "xbel" );
   doc.appendChild( root );

	exportToFileFolder(doc, root, _root);

	QFile file(_filename);
	if ( file.open( IO_WriteOnly ) ) {
		QTextStream stream( &file );
		stream << doc.toString();
		file.close();
	} else {
		KMessageBox::error(krApp, i18n("Error"), i18n("Unable to write to ") + _filename);
	}

	// re-enable the dirwatch
	STARTWATCH;
}

bool KrBookmarkHandler::importFromFileBookmark(QDomElement &e, KrBookmark *parent, QString *errorMsg) {
	QString url, name;
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

	// ok: got name and url, let's add a bookmark
	KrBookmark *bm = new KrBookmark(name, url, _collection);
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
		}
		n = n.nextSibling();
	}
	return true;
}


void KrBookmarkHandler::importFromFile() {
	//STOPWATCH;
	QFile file( _filename );
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
		errorMsg = _filename+i18n(" doesn't seem to be a valid Bookmarks file");
		goto ERROR;
	} else n = n.firstChild(); // skip the xbel part
	importFromFileFolder(n, _root, &errorMsg);
	goto SUCCESS;
	
ERROR:
	KMessageBox::error(krApp, "Error", i18n("Error reading bookmarks file: ")+errorMsg);

SUCCESS:
	file.close();
	//buildMenu(_root, _menu);
	//STARTWATCH;
}

void KrBookmarkHandler::populate(KPopupMenu *menu) {
	menu->clear();
	buildMenu(_root, menu);
}

void KrBookmarkHandler::buildMenu(KrBookmark *parent, KPopupMenu *menu) {
	for (KrBookmark *bm = parent->children().first(); bm; bm = parent->children().next()) {
		if (bm->isFolder()) {
			KPopupMenu *newMenu = new KPopupMenu(menu);
			menu->insertItem(QIconSet(krLoader->loadIcon("bookmark_folder", KIcon::Small)),
									bm->text(), newMenu);
			buildMenu(bm, newMenu);
		} else {
kdWarning() << "Adding " << bm->text() << endl;
			bm->plug(menu); // add on top
			CONNECT_BM(bm);
		}
	}
}

void KrBookmarkHandler::bookmarksUpdated(const QString &) {
	clearBookmarks(_root);
	importFromFile();
}

void KrBookmarkHandler::clearBookmarks(KrBookmark *root) {
	KrBookmark *tmp, *bm = root->children().first();
	while (bm) {
		if (bm->isFolder())
			clearBookmarks(bm);
		tmp = bm;
		bm = root->children().next();

		// TODO: clear all subfolders too
		tmp->unplugAll();
		delete tmp;
	}
}
