#include "krbookmarkhandler.h"
#include "kraddbookmarkdlg.h"
#include "../krusader.h"
#include "../krslots.h"
#include "../Dialogs/popularurls.h"
#include "../VFS/vfs.h"
#include <kiconloader.h>
#include <kmessagebox.h>
#include <q3ptrlist.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <Q3PopupMenu>
#include <QEvent>
#include <kactioncollection.h>
#include <klocale.h>
#include <kdebug.h>
#include <kbookmarkmanager.h>
#include <kstandarddirs.h>
#include <qfile.h>
#include <qcursor.h>

#define SPECIAL_BOOKMARKS	true

// ------------------------ for internal use
#define BOOKMARKS_FILE	"krusader/krbookmarks.xml"
#define CONNECT_BM(X)	{ disconnect(X, SIGNAL(activated(const KUrl&)), 0, 0); connect(X, SIGNAL(activated(const KUrl&)), this, SLOT(slotActivated(const KUrl&))); }
											
KrBookmarkHandler::KrBookmarkHandler(): QObject(0), _middleClick(false), _mainBookmarkPopup( 0 ), _specialBookmarkIDs(), _bookmarkIDTable() {
	// create our own action collection and make the shortcuts apply only to parent
	_privateCollection = new KActionCollection(krApp, "private collection");
	_collection = krApp->actionCollection();

	// create _root: father of all bookmarks. it is a dummy bookmark and never shown
	_root = new KrBookmark(i18n("Bookmarks"));
	
	_bookmarkIDTable.setAutoDelete( true );
	
	// load bookmarks 
	importFromFile();

	// hack
	manager = KBookmarkManager::managerForFile(KStandardDirs::locateLocal( "data", BOOKMARKS_FILE ), false);
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

void KrBookmarkHandler::bookmarkCurrent(KUrl url) {
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
	if( bm->isFolder() )
		clearBookmarks( bm ); // remove the child bookmarks
	removeReferences( _root, bm );
	bm->unplugAll();
	delete bm;
	
	exportToFile();
}

void KrBookmarkHandler::removeReferences( KrBookmark *root, KrBookmark *bmToRemove ) {
	int index = root->children().find( bmToRemove );
	if( index >= 0 )
		root->children().take( index );
	
	KrBookmark *bm = root->children().first();
	while (bm) {
		if (bm->isFolder())
			removeReferences(bm, bmToRemove);
		bm = root->children().next();
	}
}

void KrBookmarkHandler::exportToFileBookmark(QDomDocument &doc, QDomElement &where, KrBookmark *bm) {
	if( bm->isSeparator() ) {
		QDomElement bookmark = doc.createElement("separator");
		where.appendChild(bookmark);
	}
	else {
		QDomElement bookmark = doc.createElement("bookmark");
		// url
		bookmark.setAttribute("href", bm->url().prettyUrl());
		// icon
		bookmark.setAttribute("icon", bm->icon());
		// title
		QDomElement title = doc.createElement("title");	
		title.appendChild(doc.createTextNode(bm->text()));
		bookmark.appendChild(title);
		
		where.appendChild(bookmark);
	}
}

void KrBookmarkHandler::exportToFileFolder(QDomDocument &doc, QDomElement &parent, KrBookmark *folder) {
	for (KrBookmark *bm = folder->children().first(); bm; bm = folder->children().next()) {
		if (bm->isFolder()) {
			QDomElement newFolder = doc.createElement("folder");
			newFolder.setAttribute("icon", bm->icon());
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
	if (!doc.firstChild().isProcessingInstruction()) {
		// adding: <?xml version="1.0" encoding="UTF-8" ?> if not already present 
		QDomProcessingInstruction instr = doc.createProcessingInstruction( "xml", 
				"version=\"1.0\" encoding=\"UTF-8\" ");
		doc.insertBefore( instr, doc.firstChild() ); 
	}

	
	QString filename = KStandardDirs::locateLocal( "data", BOOKMARKS_FILE );
	QFile file(filename);
	if ( file.open( QIODevice::WriteOnly ) ) {
		Q3TextStream stream( &file );
		stream.setEncoding(stream.UnicodeUTF8);
		stream << doc.toString();
		file.close();
	} else {
		KMessageBox::error(krApp, i18n("Unable to write to %1").arg(filename), i18n("Error"));
	}
}

bool KrBookmarkHandler::importFromFileBookmark(QDomElement &e, KrBookmark *parent, QString path, QString *errorMsg) {
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
	KrBookmark *bm = KrBookmark::getExistingBookmark(path+name, _collection);
	if (!bm) {
		bm = new KrBookmark(name, vfs::fromPathOrUrl( url ), _collection, icon, path+name);
	parent->children().append(bm);
	}

	return true;
}

bool KrBookmarkHandler::importFromFileFolder(QDomNode &first, KrBookmark *parent, QString path, QString *errorMsg) {
	QString name;
	QDomNode n = first;
	while (!n.isNull()) {
		QDomElement e = n.toElement();
		if (e.tagName() == "bookmark") {
			if (!importFromFileBookmark(e, parent, path, errorMsg))
				return false;
		} else if (e.tagName() == "folder") {
			QString iconName = "";
			if (e.hasAttribute("icon")) iconName=e.attribute("icon");
			// the title is the first child of the folder
			QDomElement tmp = e.firstChild().toElement();
			if (tmp.tagName() != "title") {
				*errorMsg = i18n("missing tag ")+"title";
				return false;
			} else name = tmp.text();
			KrBookmark *folder = new KrBookmark(name, iconName);
			parent->children().append(folder);

			QDomNode nextOne = tmp.nextSibling();
			if (!importFromFileFolder(nextOne, folder, path + name + "/", errorMsg))
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
	
	QString filename = KStandardDirs::locateLocal( "data", BOOKMARKS_FILE );
	QFile file( filename );
	if ( !file.open(QIODevice::ReadOnly))
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
		errorMsg = i18n("%1 doesn't seem to be a valid Bookmarks file").arg(filename);
		goto ERROR;
	} else n = n.firstChild(); // skip the xbel part
	importFromFileFolder(n, _root, "", &errorMsg);
	goto SUCCESS;
	
ERROR:
	KMessageBox::error(krApp, i18n("Error reading bookmarks file: %1").arg(errorMsg), i18n( "Error" ));

SUCCESS:
	file.close();
}

void KrBookmarkHandler::populate(KMenu *menu) {
	_mainBookmarkPopup = menu;
	menu->clear();
	_bookmarkIDTable.clear();
	_specialBookmarkIDs.clear();
	buildMenu(_root, menu);
}

void KrBookmarkHandler::buildMenu(KrBookmark *parent, KMenu *menu) {
	static int inSecondaryMenu = 0; // used to know if we're on the top menu

	// run the loop twice, in order to put the folders on top. stupid but easy :-)
	// note: this code drops the separators put there by the user
	for (KrBookmark *bm = parent->children().first(); bm; bm = parent->children().next()) {
		if (!bm->isFolder()) continue;
		KMenu *newMenu = new KMenu(menu);
		int id = menu->insertItem(QIcon(krLoader->loadIcon(bm->icon(), K3Icon::Small)),
									bm->text(), newMenu, -1 /* dummy id */, -1 /* end of list */);
		
		if( !_bookmarkIDTable.find( menu ) )
			_bookmarkIDTable.insert( menu, new QMap<int, KrBookmark *> );
		(*_bookmarkIDTable[ menu ])[ id ] = bm;
		
		++inSecondaryMenu;
		buildMenu(bm, newMenu);
		--inSecondaryMenu;
	}
	for (KrBookmark *bm = parent->children().first(); bm; bm = parent->children().next()) {
		if (bm->isFolder()) continue;
		if (bm->isSeparator() ) {
			menu->insertSeparator();
			continue;
		}
		int itemIndex = bm->plug(menu, -1 /* end of list */);
		CONNECT_BM(bm);
		
		int id = bm->itemId( itemIndex );
		if( !_bookmarkIDTable.find( menu ) )
			_bookmarkIDTable.insert( menu, new QMap<int, KrBookmark *> );
		(*_bookmarkIDTable[ menu ])[ id ] = bm;
	}

	if (!inSecondaryMenu) {
		krConfig->setGroup( "Private" );
		bool hasPopularURLs = krConfig->readBoolEntry( "BM Popular URLs", true );
		bool hasDevices     = krConfig->readBoolEntry( "BM Devices",      true );
		bool hasLan         = krConfig->readBoolEntry( "BM Lan",          true );
		bool hasVirtualFS   = krConfig->readBoolEntry( "BM Virtual FS",   true );
		bool hasJumpback    = krConfig->readBoolEntry( "BM Jumpback",     true );
		
		int itemIndex;
		
		if( hasPopularURLs ) {
			menu->insertSeparator();
			
			// add the popular links submenu
			KMenu *newMenu = new KMenu(menu);
			itemIndex = menu->insertItem(QIcon(krLoader->loadIcon("bookmark_folder", K3Icon::Small)),
										i18n("Popular URLs"), newMenu, -1 /* dummy id */, -1 /* end of list */);
			_specialBookmarkIDs.append( itemIndex );
			// add the top 15 urls
			#define MAX 15
			KUrl::List list = krApp->popularUrls->getMostPopularUrls(MAX);
			KUrl::List::Iterator it;
			for (it = list.begin(); it != list.end(); ++it) {
				QString name;
				if ((*it).isLocalFile()) name = (*it).path();
				else name = (*it).prettyUrl();
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
		}
		
		// do we need to add special bookmarks?
		if (SPECIAL_BOOKMARKS) {
			if( hasDevices || hasLan || hasVirtualFS || hasJumpback )
				menu->insertSeparator();
			
			KrBookmark *bm;
			
			// note: special bookmarks are not kept inside the _bookmarks list and added ad-hoc
			if( hasDevices ) {
				bm = KrBookmark::devices(_collection);
				itemIndex = bm->plug(menu);
				_specialBookmarkIDs.append( bm->itemId( itemIndex ) );
				CONNECT_BM(bm);
			}
			
			if( hasLan ) {
				bm = KrBookmark::lan(_collection);
				itemIndex = bm->plug(menu);
				_specialBookmarkIDs.append( bm->itemId( itemIndex ) );
				CONNECT_BM(bm);
			}
			
			if( hasVirtualFS ) {
				bm = KrBookmark::virt(_collection);
				itemIndex = bm->plug(menu);
				_specialBookmarkIDs.append( bm->itemId( itemIndex ) );
				CONNECT_BM(bm);
			}
			
			if( hasJumpback ) {
				// add the jump-back button
				itemIndex = krJumpBack->plug(menu);
				_specialBookmarkIDs.append( krJumpBack->itemId( itemIndex ) );
				menu->insertSeparator();
				itemIndex = krSetJumpBack->plug(menu);
				_specialBookmarkIDs.append( krSetJumpBack->itemId( itemIndex ) );
			}
		} 
		
		if( !hasJumpback )
			menu->insertSeparator();
		
		itemIndex = menu->insertItem(krLoader->loadIcon("bookmark_add", K3Icon::Small),
			i18n("Bookmark Current"), BookmarkCurrent);
		_specialBookmarkIDs.append( itemIndex );
		itemIndex = menu->insertItem(krLoader->loadIcon("bookmark", K3Icon::Small),
			i18n("Manage Bookmarks"), ManageBookmarks);
		_specialBookmarkIDs.append( itemIndex );
	
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
		else {
			bm->unplugAll();
			delete bm;
		}

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
			case Qt::RightButton:
				_middleClick = false;
				if( obj->inherits( "QPopupMenu" ) ) {
					int id = static_cast<Q3PopupMenu*>(obj)->idAt( static_cast<QMouseEvent*>(ev)->pos() );
					
					if( obj == _mainBookmarkPopup && _specialBookmarkIDs.contains( id ) ) {
						rightClickOnSpecialBookmark();
						return true;
					}
					
					if( _bookmarkIDTable.find( obj ) ) {
						QMap<int, KrBookmark*> * table = _bookmarkIDTable[ obj ];
						if( table && table->count( id ) ) {
							KrBookmark *bm = (*table)[ id ];
							rightClicked( static_cast<Q3PopupMenu*>(obj), id, bm );
							return true;
						}
					}
				}
			case Qt::LeftButton:
				_middleClick = false;
				break;
			case Qt::MidButton:
				_middleClick = true;
				break;
			default:
				break;
		}
	}
	return QObject::eventFilter(obj, ev);
}

#define POPULAR_URLS_ID        100100
#define DEVICES_ID             100101
#define LAN_ID                 100103
#define VIRTUAL_FS_ID          100102
#define JUMP_BACK_ID           100104

void KrBookmarkHandler::rightClickOnSpecialBookmark() {
	krConfig->setGroup( "Private" );
	bool hasPopularURLs = krConfig->readBoolEntry( "BM Popular URLs", true );
	bool hasDevices     = krConfig->readBoolEntry( "BM Devices",      true );
	bool hasLan         = krConfig->readBoolEntry( "BM Lan",          true );
	bool hasVirtualFS   = krConfig->readBoolEntry( "BM Virtual FS",   true );
	bool hasJumpback    = krConfig->readBoolEntry( "BM Jumpback",     true );
	
	Q3PopupMenu menu( _mainBookmarkPopup );
	menu.setCaption( i18n( "Enable special bookmarks" ) );
	menu.setCheckable( true );
	
	menu.insertItem( i18n( "Popular URLs" ), POPULAR_URLS_ID );
	menu.setItemChecked( POPULAR_URLS_ID, hasPopularURLs );
	menu.insertItem( i18n( "Devices" ), DEVICES_ID );
	menu.setItemChecked( DEVICES_ID, hasDevices );
	menu.insertItem( i18n( "Local Network" ), LAN_ID );
	menu.setItemChecked( LAN_ID, hasLan );
	menu.insertItem( i18n( "Virtual Filesystem" ), VIRTUAL_FS_ID );
	menu.setItemChecked( VIRTUAL_FS_ID, hasVirtualFS );
	menu.insertItem( i18n( "Jump back" ), JUMP_BACK_ID );
	menu.setItemChecked( JUMP_BACK_ID, hasJumpback );
	
	connect( _mainBookmarkPopup, SIGNAL( highlighted( int ) ), &menu, SLOT( close() ) );
	connect( _mainBookmarkPopup, SIGNAL( activated( int ) ), &menu, SLOT( close() ) );
	
	int result = menu.exec( QCursor::pos() );
	bool doCloseMain = true;
	
	krConfig->setGroup( "Private" );
	
	switch( result ) {
	case POPULAR_URLS_ID:
		krConfig->writeEntry( "BM Popular URLs", !hasPopularURLs );
		break;
	case DEVICES_ID:
		krConfig->writeEntry( "BM Devices", !hasDevices );
		break;
	case LAN_ID:
		krConfig->writeEntry( "BM Lan", !hasLan );
		break;
	case VIRTUAL_FS_ID:
		krConfig->writeEntry( "BM Virtual FS", !hasVirtualFS );
		break;
	case JUMP_BACK_ID:
		krConfig->writeEntry( "BM Jumpback", !hasJumpback );
		break;
	default:
		doCloseMain = false;
		break;
	}
	
	menu.close();
	
	if( doCloseMain && _mainBookmarkPopup )
		_mainBookmarkPopup->close();
}

#define OPEN_ID           100200
#define OPEN_NEW_TAB_ID   100201
#define DELETE_ID         100202

void KrBookmarkHandler::rightClicked( Q3PopupMenu *menu, int /*id*/, KrBookmark * bm ) {
	Q3PopupMenu popup( _mainBookmarkPopup );
	
	popup.insertItem( krLoader->loadIcon( "fileopen", K3Icon::Panel ), i18n( "Open" ), OPEN_ID );
	popup.insertItem( krLoader->loadIcon( "tab_new", K3Icon::Panel ), i18n( "Open in a new tab" ), OPEN_NEW_TAB_ID );
	popup.insertSeparator();
	popup.insertItem( krLoader->loadIcon( "editdelete", K3Icon::Panel ), i18n( "Delete" ), DELETE_ID );
	
	connect( menu, SIGNAL( highlighted( int ) ), &popup, SLOT( close() ) );
	connect( menu, SIGNAL( activated( int ) ), &popup, SLOT( close() ) );
	
	int result = popup.exec( QCursor::pos() );
	
	popup.close();
	if( _mainBookmarkPopup && result >= OPEN_ID && result <= DELETE_ID ) {
		_mainBookmarkPopup->close();
	}
	
	switch( result ) {
	case OPEN_ID:
		SLOTS->refresh( bm->url() );
		break;
	case OPEN_NEW_TAB_ID:
		SLOTS->newTab( bm->url() );
		break;
	case DELETE_ID:
		deleteBookmark( bm );
		break;
	}
}

// used to monitor middle clicks. if mid is found, then the
// bookmark is opened in a new tab. ugly, but easier than overloading
// KAction and KActionCollection.
void KrBookmarkHandler::slotActivated(const KUrl& url) {
	if (_middleClick)
		SLOTS->newTab(url);
	else SLOTS->refresh(url);
}


#include "krbookmarkhandler.moc"
