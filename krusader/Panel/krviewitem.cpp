#include "krviewitem.h"
#include "../VFS/krpermhandler.h"
#include <klocale.h>
#include <kmimetype.h>

#define PROPS	static_cast<const KrViewProperties*>(_viewProperties)

QString KrViewItem::description() const {
	if (dummyVfile) return i18n("Climb up the directory tree");
	// else is implied
	QString text = _vf->vfile_getName();
	QString comment = KMimeType::mimeType(_vf->vfile_getMime())->comment(text, false);
	QString myLinkDest = _vf->vfile_getSymDest();
	KIO::filesize_t mySize = _vf->vfile_getSize();
	
	QString text2 = text.copy();
	mode_t m_fileMode = _vf->vfile_getMode();
	
	if (_vf->vfile_isSymLink() ){
		QString tmp;
		if ( comment.isEmpty() )	tmp = i18n ( "Symbolic Link" ) ;
		else if( _vf->vfile_getMime() == "Broken Link !" ) tmp = i18n("(broken link !)");
		else tmp = i18n("%1 (Link)").arg(comment);
	
		text += "->";
	text += myLinkDest;
	text += "  ";
	text += tmp;
	} else if ( S_ISREG( m_fileMode ) ){
	text = QString("%1 (%2)").arg(text2).arg( PROPS->humanReadableSize ?
		KRpermHandler::parseSize(_vf->vfile_getSize()) : KIO::convertSize( mySize ) );
	text += "  ";
	text += comment;
	} else if ( S_ISDIR ( m_fileMode ) ){
	text += "/  ";
		text += comment;
	} else {
	text += "  ";
	text += comment;
	}
	return text;
}

