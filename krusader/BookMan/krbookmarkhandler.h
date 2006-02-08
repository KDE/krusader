#ifndef KRBOOKMARK_HANDLER_H
#define KRBOOKMARK_HANDLER_H

#include "krbookmark.h"
#include <qobject.h>
#include <kpopupmenu.h>
#include <kurl.h>
#include <qptrlist.h>
#include <qdom.h>

class KActionCollection;
class KBookmarkManager;

class KrBookmarkHandler: public QObject {
	Q_OBJECT
	friend class KrAddBookmarkDlg;
	enum Actions { BookmarkCurrent=0, ManageBookmarks };
public:
	KrBookmarkHandler();
	~KrBookmarkHandler();
	void populate(KPopupMenu *menu);
	void addBookmark(KrBookmark *bm, KrBookmark *parent = 0);
	void bookmarkCurrent(KURL url);

protected:
	void deleteBookmark(KrBookmark *bm);
	void importFromFile();
	bool importFromFileBookmark(QDomElement &e, KrBookmark *parent, QString path, QString *errorMsg);
	bool importFromFileFolder(QDomNode &first, KrBookmark *parent, QString path, QString *errorMsg);
	void exportToFile();
	void exportToFileFolder(QDomDocument &doc, QDomElement &parent, KrBookmark *folder);
	void exportToFileBookmark(QDomDocument &doc, QDomElement &where, KrBookmark *bm);
	void clearBookmarks(KrBookmark *root);
	void buildMenu(KrBookmark *parent, KPopupMenu *menu);

	bool eventFilter( QObject *obj, QEvent *ev );
	
protected slots:
	void menuOperation(int id);
	void bookmarksChanged(const QString&, const QString&);
	void slotActivated(const KURL& url);

private:
	KActionCollection *_collection, *_privateCollection;
	KrBookmark *_root;
	// the whole KBookmarkManager is an ugly hack. use it until we have our own
	KBookmarkManager *manager;
	bool _middleClick; // if true, the user clicked the middle button to open the bookmark
};

#endif // KRBOOKMARK_HANDLER_H
