#include "krbookmark.h"
#include "../krusader.h"
#include "../VFS/krarchandler.h"
#include <kactioncollection.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#define BM_NAME(X)		(QString("Bookmark:")+X)

#if KDE_IS_VERSION(3,4,0)
static const char* NAME_DEVICES = I18N_NOOP("Media");
#else
static const char* NAME_DEVICES = I18N_NOOP("Devices");
#endif
static const char* NAME_VIRTUAL = I18N_NOOP("Virtual Filesystem");
static const char* NAME_LAN = I18N_NOOP("Local Network");

KrBookmark::KrBookmark(QString name, KURL url, KActionCollection *parent, QString icon):
	KAction(name, 0, 0, 0, parent, BM_NAME(name).latin1()), 
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

KrBookmark* KrBookmark::getExistingBookmark(QString name, KActionCollection *collection) {
	return static_cast<KrBookmark*>(collection->action(BM_NAME(name).latin1()));
}

KrBookmark* KrBookmark::devices(KActionCollection *collection) {
	KrBookmark *bm = getExistingBookmark(NAME_DEVICES, collection);	
	if (!bm) {
#if KDE_IS_VERSION(3,4,0)
		bm = new KrBookmark(NAME_DEVICES, "media:/", collection);
#else
		bm = new KrBookmark(NAME_DEVICES, "devices:/", collection);
#endif
		bm->setIconSet(krLoader->loadIcon("blockdevice", KIcon::Small));
	}
	return bm;
}

KrBookmark* KrBookmark::virt(KActionCollection *collection) {
	KrBookmark *bm = getExistingBookmark(NAME_VIRTUAL, collection);	
	if (!bm) {
		bm = new KrBookmark(NAME_VIRTUAL, "virt:/", collection);
		bm->setIconSet(krLoader->loadIcon("pipe", KIcon::Small));
	}
	return bm;
}

KrBookmark* KrBookmark::lan(KActionCollection *collection) {
	KrBookmark *bm = getExistingBookmark(NAME_LAN, collection);	
	if (!bm) {
		bm = new KrBookmark(NAME_LAN, "lan:/", collection);
		bm->setIconSet(krLoader->loadIcon("network", KIcon::Small));
	}
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
