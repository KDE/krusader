#include "krbookmark.h"
#include "../krusader.h"
#include <kactioncollection.h>
#include <kiconloader.h>
#include <klocale.h>

KrBookmark::KrBookmark(QString name, KURL url, KActionCollection *parent):
	KAction(name, 0, 0, 0, parent), _url(url) {
	connect(this, SIGNAL(activated()), this, SLOT(activatedProxy()));
}

const KURL& KrBookmark::url() const {
	return _url;
}

void KrBookmark::setURL(const KURL& url) {
	_url = url;
}

KrBookmark* KrBookmark::devices(KActionCollection *collection) {
	KrBookmark *bm = new KrBookmark(I18N_NOOP("Devices"), "devices:/", collection);
	bm->setIconSet(krLoader->loadIcon("blockdevice", KIcon::Small));
	return bm;
}

void KrBookmark::activatedProxy() {
	emit activated(this);
}
