#include "krbookmarkhandler.h"
#include "kraddbookmarkdlg.h"
#include "../krusader.h"
#include "../krslots.h"
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <qfile.h>


#define SPECIAL_BOOKMARKS	true

#define BOOKMARKS_FILE	"krusader/bookman2.xml"
#define TOP_OF_MENU		0
#define BOTTOM_OF_MENU	_menu->indexOf(Border)
#define CONNECT_BM(X)	connect(X, SIGNAL(activated(KrBookmark *)),					\
											this, SLOT(bookmarkActivated(KrBookmark *)));

KrBookmarkHandler::KrBookmarkHandler(QWidget *parent, KPopupMenu *menu): QObject(parent), _menu(menu) {
	// create our own action collection and make the shortcuts apply only to parent
	_collection = new KActionCollection(0, parent);

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

void KrBookmarkHandler::addBookmark(KrBookmark *bm) {
	// add to the list (bottom)
	_bookmarks.append(bm);

	// add to menu
	bm->plug(_menu, BOTTOM_OF_MENU); // add on top
	CONNECT_BM(bm);

	// save
	exportToFile();
}

void KrBookmarkHandler::deleteBookmark(KrBookmark *bm) {
}

void KrBookmarkHandler::exportToFileBookmark(QDomDocument &doc, QDomElement &where, KrBookmark *bm) {
	QDomElement bookmark = doc.createElement("bookmark");
	bookmark.setAttribute("href", bm->url().url());
	where.appendChild(bookmark);
	// title
	QDomElement title = doc.createElement("title");
	title.appendChild(doc.createTextNode(bm->text()));
	bookmark.appendChild(title);
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
	// use the local file
	QString filename = locateLocal( "data", BOOKMARKS_FILE );

	QDomDocument doc( "xbel" );
   QDomElement root = doc.createElement( "xbel" );
   doc.appendChild( root );

	// iterate through the bookmark list
	for (KrBookmark *bm = _bookmarks.first(); bm; bm = _bookmarks.next()) {
		// TODO: support for folder
		exportToFileBookmark(doc, root, bm);
	}

	QFile file(filename);
	if ( file.open( IO_WriteOnly ) ) {
		QTextStream stream( &file );
		stream << doc.toString();
		file.close();
	} else {
		KMessageBox::error(krApp, i18n("Error"), i18n("Unable to write to ") + filename);
	}
}

bool KrBookmarkHandler::importFromFileBookmark(QDomElement &e, QString *errorMsg) {
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
	if (e.firstChild().toElement().tagName() != "title") {
		*errorMsg = i18n("missing tag ")+"title";
		return false;
	} else name = e.firstChild().toElement().toText().nodeValue();
	if (e.firstChild().toElement().isText()) {
		kdWarning() <<"text" << endl;
		QDomText d = e.firstChild().toElement().toText();
		kdWarning() << d.nodeName() << ":" << d.nodeValue() << endl;
	} else kdWarning() << "not text" << endl; 

	
	kdWarning() << name << ";" << url << endl;



	return true;
}

void KrBookmarkHandler::importFromFile() {
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
	// iterate through the document: first child should be "xbel"
	n = doc.firstChild();
	if (n.isNull() || n.toElement().tagName()!="xbel") {
		errorMsg = filename+i18n(" doesn't seem to be a valid Bookmarks file");
		goto ERROR;
	} else n = n.firstChild(); // skip the 'xbel' root
	
	while (!n.isNull()) {
		e = n.toElement();
		if (!importFromFileBookmark(e, &errorMsg))
			goto ERROR;
		n = n.nextSibling();
	}

	
	goto SUCCESS;
	
ERROR:
	KMessageBox::error(krApp, "Error", i18n("Error reading bookmarks file: ")+errorMsg);
SUCCESS:
	file.close();
}


#if 0
 QDomNode n = docElem.firstChild();
    while( !n.isNull() ) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if( !e.isNull() ) {
            cout << e.tagName() << endl; // the node really is an element.
        }
        n = n.nextSibling();
    }
#endif