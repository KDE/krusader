#ifndef KRBOOKMARK_HANDLER_H
#define KRBOOKMARK_HANDLER_H

#include "krbookmark.h"
#include <qobject.h>
#include <kpopupmenu.h>
#include <kurl.h>
#include <qptrlist.h>
#include <qdom.h>

class KActionCollection;
class KDirWatch;

class KrBookmarkHandler: public QObject {
	Q_OBJECT

	enum Actions { Border, BookmarkCurrent, ManageBookmarks };
public:
	KrBookmarkHandler(QObject *parent);
	KrBookmarkHandler(QWidget *parent, KPopupMenu *menu);

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
	
signals:
	void openUrl(const KURL& url);

protected slots:
	void bookmarkActivated(KrBookmark *bm);
	void menuOperation(int id);
	void bookmarksUpdated(const QString &);
	
private:
	KPopupMenu *_menu;
	KActionCollection *_collection;
	KrBookmark *_root;
	KDirWatch *_dirwatch;
	QString _filename;
};

#endif // KRBOOKMARK_HANDLER_H
