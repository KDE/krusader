#ifndef KRBOOKMARK_H
#define KRBOOKMARK_H

#include <kaction.h>
#include <qlist.h>
#include <kurl.h>

class KActionCollection;

class KrBookmark: public KAction {
	Q_OBJECT
public:
	KrBookmark(QString name, KUrl url, KActionCollection *parent, QString icon = "", QString actionName = QString() );
	KrBookmark(QString name, QString icon = ""); // creates a folder
	~KrBookmark();
	// text() and setText() to change the name of the bookmark
	// icon() and setIcon() to change icons
	inline const QString& iconName() const { return _icon; }
	inline const KUrl& url() const { return _url; }
	inline void setURL(const KUrl& url) { _url = url; }
	inline bool isFolder() const { return _folder; }
	inline bool isSeparator() const { return _separator; }
	QList<KrBookmark *>& children() { return _children; }

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
	bool _autoDelete;
	QList<KrBookmark *> _children;
};

#endif // KRBOOKMARK_H
