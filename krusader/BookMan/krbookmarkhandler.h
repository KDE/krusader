#ifndef KRBOOKMARK_HANDLER_H
#define KRBOOKMARK_HANDLER_H

#include <qobject.h>
#include <kpopupmenu.h>
#include <kurl.h>

class KActionCollection;
class KrBookmark;

class KrBookmarkHandler: public QObject {
	Q_OBJECT

	enum Actions { BookmarkCurrent, ManageBookmarks };
public:
	KrBookmarkHandler(QWidget *parent, KPopupMenu *menu);

protected:
	void bookmarkCurrent(KURL url);

signals:
	void openUrl(const KURL& url);

protected slots:
	void bookmarkActivated(KrBookmark *bm);
	void menuOperation(int id);
	
private:
	KPopupMenu *_menu;
	KActionCollection *_collection;
};

#endif // KRBOOKMARK_HANDLER_H
