#ifndef KRBOOKMARK_HANDLER_H
#define KRBOOKMARK_HANDLER_H

#include <qobject.h>
#include <kpopupmenu.h>

class KActionCollection;

class KrBookmarkHandler: public QObject {
	enum Actions { NewFolder=10, BookmarkCurrent, EditBookmarks, NewLocal, NewRemote };
public:	
	KrBookmarkHandler(QWidget *parent, KPopupMenu *menu);

private:
	KPopupMenu *_menu;
	KActionCollection *_collection;
};

#endif // KRBOOKMARK_HANDLER_H
