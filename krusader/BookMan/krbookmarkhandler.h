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

	enum Actions { Border, BookmarkCurrent, ManageBookmarks };
public:
	KrBookmarkHandler(QWidget *parent, KPopupMenu *menu);

protected:
	void bookmarkCurrent(KURL url);
	void addBookmark(KrBookmark *bm);
	void deleteBookmark(KrBookmark *bm);
	void importFromFile();
	bool importFromFileBookmark(QDomElement &e, QString *errorMsg);
	void exportToFile();
	void exportToFileBookmark(QDomDocument &doc, QDomElement &where, KrBookmark *bm);
	
signals:
	void openUrl(const KURL& url);

protected slots:
	void bookmarkActivated(KrBookmark *bm);
	void menuOperation(int id);
	
private:
	KPopupMenu *_menu;
	KActionCollection *_collection;
	QPtrList<KrBookmark> _bookmarks;
};

#endif // KRBOOKMARK_HANDLER_H
