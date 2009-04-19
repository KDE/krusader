#include "krbookmark.h"
#include "../krusader.h"
#include "../VFS/krarchandler.h"
#include <kactioncollection.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include "../krtrashhandler.h"

#define BM_NAME(X)		(QString("Bookmark:")+X)

static const char* NAME_TRASH = I18N_NOOP ( "Trash bin" );
static const char* NAME_VIRTUAL = I18N_NOOP ( "Virtual Filesystem" );
static const char* NAME_LAN = I18N_NOOP ( "Local Network" );

KrBookmark::KrBookmark ( QString name, KUrl url, KActionCollection *parent, QString icon, QString actionName ) :
		KAction ( parent ), _url ( url ), _icon(icon), _folder ( false ), _separator ( false ), _autoDelete( true )
{
	QString actName = actionName.isNull() ? BM_NAME ( name ) : BM_NAME ( actionName );
	setText ( name );
	parent->addAction ( actName, this );
	connect ( this, SIGNAL ( triggered() ), this, SLOT ( activatedProxy() ) );

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
}

KrBookmark::KrBookmark ( QString name, QString icon ) :
		KAction ( KIcon(icon), name, 0 ), _icon(icon), _folder ( true ), _separator ( false ), _autoDelete( false )
{
	setIcon ( KIcon(icon=="" ? "folder" : icon) );
}

KrBookmark::~KrBookmark()
{
  if( _autoDelete )
  {
    QListIterator<KrBookmark *> it(_children);
    while (it.hasNext())
      delete it.next();
    _children.clear();
  }
}

KrBookmark* KrBookmark::getExistingBookmark ( QString actionName, KActionCollection *collection )
{
	return static_cast<KrBookmark*> ( collection->action ( BM_NAME ( actionName ) ) );
}

KrBookmark* KrBookmark::trash ( KActionCollection *collection )
{
	KrBookmark *bm = getExistingBookmark ( i18n ( NAME_TRASH ), collection );
	if ( !bm )
		bm = new KrBookmark ( i18n ( NAME_TRASH ), KUrl("trash:/"), collection );
	
	bm->setIcon ( krLoader->loadIcon ( KrTrashHandler::trashIcon(), KIconLoader::Small ) );
	return bm;
}

KrBookmark* KrBookmark::virt ( KActionCollection *collection )
{
	KrBookmark *bm = getExistingBookmark ( i18n ( NAME_VIRTUAL ), collection );
	if ( !bm )
	{
		bm = new KrBookmark ( i18n ( NAME_VIRTUAL ), KUrl("virt:/"), collection );
		bm->setIcon ( krLoader->loadIcon ( "pipe", KIconLoader::Small ) );
	}
	return bm;
}

KrBookmark* KrBookmark::lan ( KActionCollection *collection )
{
	KrBookmark *bm = getExistingBookmark ( i18n ( NAME_LAN ), collection );
	if ( !bm )
	{
		bm = new KrBookmark ( i18n ( NAME_LAN ), KUrl("remote:/"), collection );
		bm->setIcon ( krLoader->loadIcon ( "network-wired", KIconLoader::Small ) );
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
