#ifndef KRBOOKMARK_H
#define KRBOOKMARK_H

#include <kaction.h>
#include <kurl.h>

class KActionCollection;

class KrBookmark: public KAction {
public:
	KrBookmark(QString name, KURL url, KActionCollection *parent);
	// use text() and setText() to change the name of the bookmark
	const KURL& url() const;
	bool isGroup() const;
	
	void setURL(const KURL& url);
	void setGroup(bool group);
	
	
private:
	KURL _url;
	bool _group;
};

#endif // KRBOOKMARK_H
