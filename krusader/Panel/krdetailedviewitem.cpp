/***************************************************************************
                            krdetailedviewitem.cpp
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../krusader.h"
#include "../defaults.h"
#include "../kicons.h"
#include "krdetailedviewitem.h"
#include "krdetailedview.h"
#include "../VFS/krpermhandler.h"
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <kdebug.h>
#include <kmimetype.h>

KrDetailedViewItem::KrDetailedViewItem(KrDetailedView *parent, QListViewItem *after, vfile *vf):
  QObject(parent), KListViewItem(parent, after), KrViewItem(), _vf(vf), _view(parent) {
  repaintItem();
}

void KrDetailedViewItem::repaintItem() {
    if (_vf == 0L) return;
    // set text in columns, according to what columns are available
    int id = KrDetailedView::Unused;
    if ((id = _view->column(KrDetailedView::Mime)) != -1) {
      QString tmp = _vf->vfile_getMime();
      setText(id, tmp.mid(tmp.find('/')+1));
    }
    if ((id = _view->column(KrDetailedView::Size)) != -1) {
      if (_vf->vfile_isDir() && _vf->vfile_getSize() <= 0) setText(id, "<DIR>");
	    else setText(id, KRpermHandler::parseSize(_vf->vfile_getSize()));
    }

    if ((id = _view->column(KrDetailedView::DateTime)) != -1)
      setText(id, _vf->vfile_getDateTime());
    if ((id = _view->column(KrDetailedView::KrPermissions)) != -1) {
      // first, build the krusader permissions
      QString tmp;
      switch (_vf->vfile_isReadable()){
        case ALLOWED_PERM: tmp+='r'; break;
        case UNKNOWN_PERM: tmp+='?'; break;
        case NO_PERM:      tmp+='-'; break;
      }
   	  switch (_vf->vfile_isWriteable()){
        case ALLOWED_PERM: tmp+='w'; break;
        case UNKNOWN_PERM: tmp+='?'; break;
        case NO_PERM:      tmp+='-'; break;
      }
   	  switch (_vf->vfile_isExecutable()){
        case ALLOWED_PERM: tmp+='x'; break;
        case UNKNOWN_PERM: tmp+='?'; break;
        case NO_PERM:      tmp+='-'; break;
      }
      setText(id, tmp);
    }
    if ((id = _view->column(KrDetailedView::Permissions)) != -1)
      setText(id, _vf->vfile_getPerm());
    if ((id = _view->column(KrDetailedView::Owner)) != -1) {
      uid_t uid = _vf->vfile_getUid();
      QString username = QString("%1").arg((int)uid);
      struct passwd *p = getpwuid(uid);
      if (p!=NULL) username = QString(p->pw_name);
      setText(id, username);
    }
    if ((id = _view->column(KrDetailedView::Group)) != -1) {
      gid_t gid = _vf->vfile_getGid();
      QString grpname = QString("%1").arg((int)gid);
      struct group *g = getgrgid(gid);
      if (g!=NULL) grpname = QString(g->gr_name);
      setText(id, grpname);
    }
    // if we've got an extention column, clip the name accordingly
    QString name = _vf->vfile_getName(), ext = "";
    if ((id = _view->column(KrDetailedView::Extention)) != -1 && !_vf->vfile_isDir()) {
      int i;
      if ((i = name.findRev('.')) > 0) {
        ext = name.mid(i+1);
        name = name.mid(0, i);
      }
      setText(id, ext);
    }
    setText(_view->column(KrDetailedView::Name), name);
    // display an icon if needed
    if (_view->_withIcons)
      setPixmap(_view->column(KrDetailedView::Name),KrView::getIcon(_vf));
}

QString num2qstring(unsigned long num){
  QString temp;
  temp.sprintf("%015ld",num);
  return temp;
}

QString KrDetailedViewItem::name() const {
  if (_vf) return _vf->vfile_getName();
  else return text(_view->column(KrDetailedView::Name));
}

void KrDetailedViewItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align) {
  // center the <DIR> thing if needed
  if(column != _view->column(KrDetailedView::Size))
    KListViewItem::paintCell(p, cg, column, width, align);
  else if (_vf) {
    if (_vf->vfile_isDir() && _vf->vfile_getSize()<=0)
      KListViewItem::paintCell(p, cg, column, width, Qt::AlignHCenter);
    else KListViewItem::paintCell(p, cg, column, width, align); // size
  } else KListViewItem::paintCell(p, cg, column, width, Qt::AlignHCenter); // updir
}


QPixmap& KrDetailedViewItem::icon() {
  QPixmap *p;
  if (_view->_withIcons)
    p = new QPixmap(*(pixmap(_view->column(KrDetailedView::Name))));
  else p = new QPixmap(KrView::getIcon(_vf));
  return *p;
}

int KrDetailedViewItem::compare(QListViewItem *i,int col,bool ascending ) const {
  int asc = ( ascending ? -1 : 1 );
  KrViewItem *other = dynamic_cast<KrViewItem*>(i);

  if (name()== "..") return 1*asc;
  if (other->name()== "..") return -1*asc;

	// handle directory sorting
  if (isDir()){
    if (!other->isDir()) return 1*asc;
  }
	else if(other->isDir()) return -1*asc;


  QString text0 = name();
  QString itext0 = other->name();

  krConfig->setGroup("Look&Feel");
  if(!krConfig->readBoolEntry("Case Sensative Sort",_CaseSensativeSort)) {
    text0  = text0.lower();
    itext0 = itext0.lower();
  }

	//kdDebug() << "text0: "<< text0 << " ,itext0: "<<itext0 << endl;

  int result = 0;

  if (col == _view->column(KrDetailedView::Name)) {
    	result = QString::compare(text0,itext0);
  } else if (col == _view->column(KrDetailedView::Size)) {
    	result = QString::compare(num2qstring(size()),num2qstring(other->size()));
  } else if (col == _view->column(KrDetailedView::DateTime)) {
      QString dt = dateTime();
      QString dti = other->dateTime();
			QString  d = ((dt[6] < '7')? "20" : "19") +
      		dt[6] + dt[7] + // year
					dt[3] + dt[4] + // month
					dt[0] + dt[1] + // day
          dt[9] + dt[10]+ //	hour
					dt[12]+ dt[13]; // minute
			QString  id = ((dti[6] < '7')? "20" : "19")  +
      		dti[6] + dti[7] + // year
					dti[3] + dti[4] + // month
					dti[0] + dti[1] + // day
          dti[9] + dti[10]+ //	hour
					dti[12]+ dti[13]; // minute
    	result = QString::compare(d,id);
  } else {
    // Joker for extention and permissions (so far)
    result = QString::compare(text(col), i->text(col));
  }

  return result;
}

QString KrDetailedViewItem::description() const {
 	if (name()=="..") return i18n("Climb up the directory tree");
	else{
    QString text = _vf->vfile_getName();
		QString comment = KMimeType::mimeType(_vf->vfile_getMime())->comment(text, false);
 		QString myLinkDest = _vf->vfile_getSymDest();
		long long mySize = _vf->vfile_getSize();

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
     	text = QString("%1 (%2)").arg(text2).arg( KIO::convertSize( mySize ) );
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
}
