#include "krbookmark.h"
#include <kactioncollection.h>

KrBookmark::KrBookmark(QString name, KURL url, KActionCollection *parent):
	KAction(name, 0, 0, 0, parent), _url(url), _group(false) {
}

const KURL& KrBookmark::url() const {
	return _url;
}

bool KrBookmark::isGroup() const {
	return _group;
}
	
void KrBookmark::setURL(const KURL& url) {
	_url = url;
}

void KrBookmark::setGroup(bool group) {
	_group = group;
}

