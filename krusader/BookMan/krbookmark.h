#ifndef KRBOOKMARK_H
#define KRBOOKMARK_H

#include <kaction.h>
#include <qptrlist.h>
#include <kurl.h>

class KActionCollection;

class KrBookmark: public KAction {
	Q_OBJECT
public:
	KrBookmark(QString name, KURL url, KActionCollection *parent);
	KrBookmark(QString name); // creates a folder
	// text() and setText() to change the name of the bookmark
	// icon() and setIcon() to change icons (by name)
	const KURL& url() const { return _url; }
	void setURL(const KURL& url) { _url = url; }
	bool isFolder() const { return _folder; }
	QPtrList<KrBookmark>& children() { return _children; }
	// ----- special bookmarks
	static KrBookmark* devices(KActionCollection *collection);

signals:
	void activated(const KURL& url);

protected slots:
	void activatedProxy();
	
	
private:
	KURL _url;
	QString _icon;
	bool _folder;
	QPtrList<KrBookmark> _children;
};

#endif // KRBOOKMARK_H
