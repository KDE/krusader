#include "krbookmark.h"
#include "../krusader.h"
#include "../VFS/krarchandler.h"
#include <kactioncollection.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

KrBookmark::KrBookmark(QString name, KURL url, KActionCollection *parent, QString icon):
	KAction(name, 0, 0, 0, parent, QString("Bookmark:"+name).latin1()), 
	_url(url), _folder(false), _separator(false) {
	connect(this, SIGNAL(activated()), this, SLOT(activatedProxy()));
	// do we have an icon?
	if (!icon.isEmpty())
		setIcon(icon);
	else {
		// what kind of a url is it?
		if (_url.isLocalFile()) {
			setIcon("folder");
		} else { // is it an archive?
			if (KRarcHandler::isArchive(_url))
				setIcon("tar");
			else setIcon("folder_html");
		}
	}

	_children.setAutoDelete(true);
}

KrBookmark::KrBookmark(QString name):
	KAction(name, 0, 0, 0, 0), _folder(true), _separator(false) {
	setIcon("folder");
}


KrBookmark* KrBookmark::devices(KActionCollection *collection) {
	KrBookmark *bm = new KrBookmark(I18N_NOOP("Devices"), "devices:/", collection);
	bm->setIconSet(krLoader->loadIcon("blockdevice", KIcon::Small));
	return bm;
}

KrBookmark* KrBookmark::separator() {
	KrBookmark *bm = new KrBookmark("");
	bm->_separator = true;
	return bm;
}


void KrBookmark::activatedProxy() {
	emit activated(url());
}

#include "krbookmark.moc"
