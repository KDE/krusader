#include "krbookmark.h"
#include "../krusader.h"
#include "../VFS/krarchandler.h"
#include <kactioncollection.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

KrBookmark::KrBookmark(QString name, KURL url, KActionCollection *parent):
	KAction(name, 0, 0, 0, parent), _url(url), _folder(false) {
	connect(this, SIGNAL(activated()), this, SLOT(activatedProxy()));

	// what kind of a url is it?
	if (_url.isLocalFile()) {
		setIcon("folder");
	} else { // is it an archive?
		if (KRarcHandler::isArchive(_url))
			setIcon("tar");
		else setIcon("folder_html");
	}

	_children.setAutoDelete(true);
}

KrBookmark::KrBookmark(QString name):
	KAction(name, 0, 0, 0, 0), _folder(true) {
	setIcon("folder");
}


KrBookmark* KrBookmark::devices(KActionCollection *collection) {
	KrBookmark *bm = new KrBookmark(I18N_NOOP("Devices"), "devices:/", collection);
	bm->setIconSet(krLoader->loadIcon("blockdevice", KIcon::Small));
	return bm;
}

void KrBookmark::activatedProxy() {
	emit activated(url());
}
