#ifndef KRBOOKMARK_H
#define KRBOOKMARK_H

#include <kaction.h>
#include <qptrlist.h>
#include <kurl.h>

class KActionCollection;

class KrBookmark: public KAction {
	Q_OBJECT
public:
	KrBookmark(QString name, KURL url, KActionCollection *parent, QString icon = "", QString actionName = QString::null );
	KrBookmark(QString name, QString icon = ""); // creates a folder
	// text() and setText() to change the name of the bookmark
	// icon() and setIcon() to change icons (by name)
	inline const KURL& url() const { return _url; }
	inline void setURL(const KURL& url) { _url = url; }
	inline bool isFolder() const { return _folder; }
	inline bool isSeparator() const { return _separator; }
	QPtrList<KrBookmark>& children() { return _children; }

	static KrBookmark* getExistingBookmark(QString actionName, KActionCollection *collection);	
	// ----- special bookmarks
	static KrBookmark* devices(KActionCollection *collection);
	static KrBookmark* virt(KActionCollection *collection);
	static KrBookmark* lan(KActionCollection *collection);
	static KrBookmark* separator();

signals:
	void activated(const KURL& url);

protected slots:
	void activatedProxy();
	
	
private:
	KURL _url;
	QString _icon;
	bool _folder;
	bool _separator;
	QPtrList<KrBookmark> _children;
};

#endif // KRBOOKMARK_H
