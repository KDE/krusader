#ifndef KRBOOKMARK_HANDLER_H
#define KRBOOKMARK_HANDLER_H

#include "krbookmark.h"
#include <qobject.h>
#include <kpopupmenu.h>
#include <kurl.h>
#include <qptrlist.h>
#include <qdom.h>

class KActionCollection;

class KrBookmarkHandler: public QObject {
	Q_OBJECT
	friend class KrAddBookmarkDlg;
	enum Actions { Border, BookmarkCurrent, ManageBookmarks };
public:
	KrBookmarkHandler();
	void populate(KPopupMenu *menu);

protected:
	void bookmarkCurrent(KURL url);
	void addBookmark(KrBookmark *bm, bool saveData = true, KrBookmark *parent = 0);
	void deleteBookmark(KrBookmark *bm);
	void importFromFile();
	bool importFromFileBookmark(QDomElement &e, KrBookmark *parent, QString *errorMsg);
	bool importFromFileFolder(QDomNode &first, KrBookmark *parent, QString *errorMsg);
	void exportToFile();
	void exportToFileFolder(QDomDocument &doc, QDomElement &parent, KrBookmark *folder);
	void exportToFileBookmark(QDomDocument &doc, QDomElement &where, KrBookmark *bm);
	void clearBookmarks(KrBookmark *root);
	void buildMenu(KrBookmark *parent, KPopupMenu *menu);
	
protected slots:
	void menuOperation(int id);

private:
	KActionCollection *_collection;
	KrBookmark *_root;
};

#endif // KRBOOKMARK_HANDLER_H
