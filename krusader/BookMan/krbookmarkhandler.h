#ifndef KRBOOKMARK_HANDLER_H
#define KRBOOKMARK_HANDLER_H

#include "krbookmark.h"
#include <qobject.h>
#include <qpointer.h>
#include <QEvent>
#include <kmenu.h>
#include <kurl.h>
#include <q3ptrdict.h>
#include <qdom.h>
#include <qmap.h>

class KActionCollection;
class KBookmarkManager;

class KrBookmarkHandler: public QObject {
	Q_OBJECT
	friend class KrAddBookmarkDlg;
	enum Actions { BookmarkCurrent=0, ManageBookmarks };
public:
	KrBookmarkHandler();
	~KrBookmarkHandler();
	void populate(KMenu *menu);
	void addBookmark(KrBookmark *bm, KrBookmark *parent = 0);
	void bookmarkCurrent(KUrl url);

protected:
	void deleteBookmark(KrBookmark *bm);
	void importFromFile();
	bool importFromFileBookmark(QDomElement &e, KrBookmark *parent, QString path, QString *errorMsg);
	bool importFromFileFolder(QDomNode &first, KrBookmark *parent, QString path, QString *errorMsg);
	void exportToFile();
	void exportToFileFolder(QDomDocument &doc, QDomElement &parent, KrBookmark *folder);
	void exportToFileBookmark(QDomDocument &doc, QDomElement &where, KrBookmark *bm);
	void clearBookmarks(KrBookmark *root);
	void buildMenu(KrBookmark *parent, KMenu *menu);

	bool eventFilter( QObject *obj, QEvent *ev );
	
	void rightClicked( QMenu *menu, KrBookmark *bm );
	void rightClickOnSpecialBookmark();
	
	void removeReferences( KrBookmark *root, KrBookmark *bmToRemove );
	
protected slots:
	void slotBookmarkCurrent();
	void bookmarksChanged(const QString&, const QString&);
	void slotActivated(const KUrl& url);

private:
	KActionCollection *_collection, *_privateCollection;
	KrBookmark *_root;
	// the whole KBookmarkManager is an ugly hack. use it until we have our own
	KBookmarkManager *manager;
	bool _middleClick; // if true, the user clicked the middle button to open the bookmark
	
	QPointer<KMenu>            _mainBookmarkPopup; // main bookmark popup menu
	QList<QAction *>           _specialBookmarks; // the action list of the special bookmarks
};

Q_DECLARE_METATYPE(KrBookmark *);

#endif // KRBOOKMARK_HANDLER_H
