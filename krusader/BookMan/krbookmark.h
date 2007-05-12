#ifndef KRBOOKMARK_H
#define KRBOOKMARK_H

#include <kaction.h>
#include <q3ptrlist.h>
#include <kurl.h>

class KActionCollection;

class KrBookmark: public KAction {
	Q_OBJECT
public:
	KrBookmark(QString name, KUrl url, KActionCollection *parent, QString icon = "", QString actionName = QString::null );
	KrBookmark(QString name, QString icon = ""); // creates a folder
	// text() and setText() to change the name of the bookmark
	// icon() and setIcon() to change icons (by name)
	inline const KUrl& url() const { return _url; }
	inline void setURL(const KUrl& url) { _url = url; }
	inline bool isFolder() const { return _folder; }
	inline bool isSeparator() const { return _separator; }
	Q3PtrList<KrBookmark>& children() { return _children; }

	static KrBookmark* getExistingBookmark(QString actionName, KActionCollection *collection);	
	// ----- special bookmarks
	static KrBookmark* devices(KActionCollection *collection);
	static KrBookmark* virt(KActionCollection *collection);
	static KrBookmark* lan(KActionCollection *collection);
	static KrBookmark* separator();

signals:
	void activated(const KUrl& url);

protected slots:
	void activatedProxy();
	
	
private:
	KUrl _url;
	QString _icon;
	bool _folder;
	bool _separator;
	Q3PtrList<KrBookmark> _children;
};

#endif // KRBOOKMARK_H
