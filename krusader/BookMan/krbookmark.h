#ifndef KRBOOKMARK_H
#define KRBOOKMARK_H

#include <kaction.h>
#include <kurl.h>

class KActionCollection;

class KrBookmark: public KAction {
	Q_OBJECT
public:
	KrBookmark(QString name, KURL url, KActionCollection *parent);
	// use text() and setText() to change the name of the bookmark
	// use icon() and setIcon() to change icons (by name)
	const KURL& url() const;	
	
	void setURL(const KURL& url);
	
	// special bookmarks
	static KrBookmark* devices(KActionCollection *collection);

signals:
	void activated(KrBookmark *bookmark);

protected slots:
	void activatedProxy();
	
private:
	KURL _url;
	QString _icon;
};

#endif // KRBOOKMARK_H
