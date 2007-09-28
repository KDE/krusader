#include "krbookmark.h"
#include "../krusader.h"
#include "../VFS/krarchandler.h"
#include <kactioncollection.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#define BM_NAME(X)		(QString("Bookmark:")+X)

static const char* NAME_DEVICES = I18N_NOOP ( "Media" );
static const char* NAME_VIRTUAL = I18N_NOOP ( "Virtual Filesystem" );
static const char* NAME_LAN = I18N_NOOP ( "Local Network" );

KrBookmark::KrBookmark ( QString name, KUrl url, KActionCollection *parent, QString icon, QString actionName ) :
		KAction ( parent ), _url ( url ), _icon(icon), _folder ( false ), _separator ( false )
{
	setText ( actionName.isNull() ? BM_NAME ( name ).toLatin1() : BM_NAME ( actionName ).toLatin1() );
	parent->addAction ( name, this );
	connect ( this, SIGNAL ( activated() ), this, SLOT ( activatedProxy() ) );

	// do we have an icon?
	if ( !icon.isEmpty() )
		setIcon ( KIcon(icon) );
	else
	{
		// what kind of a url is it?
		if ( _url.isLocalFile() )
		{
			setIcon ( KIcon("folder") );
		}
		else
		{ // is it an archive?
			if ( KRarcHandler::isArchive ( _url ) )
				setIcon ( KIcon("tar") );
			else setIcon ( KIcon("folder_html") );
		}
	}

	_children.setAutoDelete ( true );
}

KrBookmark::KrBookmark ( QString name, QString icon ) :
		KAction ( KIcon(icon), name, 0 ), _icon(icon), _folder ( true ), _separator ( false )
{
	setIcon ( KIcon(icon=="" ? "folder" : icon) );
}

KrBookmark* KrBookmark::getExistingBookmark ( QString actionName, KActionCollection *collection )
{
	return static_cast<KrBookmark*> ( collection->action ( BM_NAME ( actionName ) ) );
}

KrBookmark* KrBookmark::devices ( KActionCollection *collection )
{
	KrBookmark *bm = getExistingBookmark ( i18n ( NAME_DEVICES ), collection );
	if ( !bm )
	{
		bm = new KrBookmark ( i18n ( NAME_DEVICES ), KUrl("media:/"), collection );
		bm->setIconSet ( krLoader->loadIcon ( "blockdevice", K3Icon::Small ) );
	}
	return bm;
}

KrBookmark* KrBookmark::virt ( KActionCollection *collection )
{
	KrBookmark *bm = getExistingBookmark ( i18n ( NAME_VIRTUAL ), collection );
	if ( !bm )
	{
		bm = new KrBookmark ( i18n ( NAME_VIRTUAL ), KUrl("virt:/"), collection );
		bm->setIconSet ( krLoader->loadIcon ( "pipe", K3Icon::Small ) );
	}
	return bm;
}

KrBookmark* KrBookmark::lan ( KActionCollection *collection )
{
	KrBookmark *bm = getExistingBookmark ( i18n ( NAME_LAN ), collection );
	if ( !bm )
	{
		bm = new KrBookmark ( i18n ( NAME_LAN ), KUrl("lan:/"), collection );
		bm->setIconSet ( krLoader->loadIcon ( "network", K3Icon::Small ) );
	}
	return bm;
}

KrBookmark* KrBookmark::separator()
{
	KrBookmark *bm = new KrBookmark ( "" );
	bm->_separator = true;
	bm->_folder = false;
	return bm;
}


void KrBookmark::activatedProxy()
{
	emit activated ( url() );
}

#include "krbookmark.moc"
