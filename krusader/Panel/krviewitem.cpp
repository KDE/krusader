/***************************************************************************
                                krviewitem.cpp
                             -------------------
    copyright            : (C) 2000-2002 by Shie Erlich & Rafi Yanai
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
#include "krviewitem.h"
#include "krdetailedview.h"
#include "../VFS/krpermhandler.h"
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

KrViewItem::KrViewItem(KrDetailedView *parent, QListViewItem *after, vfile *vf):
  KListViewItem(parent->_listview, after), _vf(vf), _view(parent) {
    // set text in columns, according to what columns are available
    int id = KrDetailedView::Unused;
    if ((id = parent->column(KrDetailedView::Mime)) != KrDetailedView::Unused) {
      QString tmp = _vf->vfile_getMime();
      setText(id, tmp.mid(tmp.find('/')+1));
    }
    if ((id = parent->column(KrDetailedView::Size)) != KrDetailedView::Unused) {
      if (_vf->vfile_isDir()) setText(id, "<DIR>");
      else setText(id, KRpermHandler::parseSize(_vf->vfile_getSize()));
    }

    if ((id = parent->column(KrDetailedView::DateTime)) != KrDetailedView::Unused)
      setText(id, _vf->vfile_getDateTime());
    if ((id = parent->column(KrDetailedView::KrPermissions)) != KrDetailedView::Unused) {
      // first, build the krusader permissions
      QString tmp;
      switch (_vf->vfile_isReadable()){
        case ALLOWED_PERM: tmp+='r'; break;
        case UNKNOWN_PERM: tmp+='?'; break;
        case NO_PERM:      tmp+='-'; break;
      }
   	  switch (vf->vfile_isWriteable()){
        case ALLOWED_PERM: tmp+='w'; break;
        case UNKNOWN_PERM: tmp+='?'; break;
        case NO_PERM:      tmp+='-'; break;
      }
   	  switch (vf->vfile_isExecutable()){
        case ALLOWED_PERM: tmp+='x'; break;
        case UNKNOWN_PERM: tmp+='?'; break;
        case NO_PERM:      tmp+='-'; break;
      }
      setText(id, tmp);
    }
    if ((id = parent->column(KrDetailedView::Permissions)) != KrDetailedView::Unused)
      setText(id, _vf->vfile_getPerm());
    if ((id = parent->column(KrDetailedView::Owner)) != KrDetailedView::Unused) {
      uid_t uid = _vf->vfile_getUid();
      QString username = QString("%1").arg((int)uid);
      struct passwd *p = getpwuid(uid);
      if (p!=NULL) username = QString(p->pw_name);
      setText(id, username);
    }
    if ((id = parent->column(KrDetailedView::Group)) != KrDetailedView::Unused) {
      gid_t gid = _vf->vfile_getGid();
      QString grpname = QString("%1").arg((int)gid);
      struct group *g = getgrgid(gid);
      if (g!=NULL) grpname = QString(g->gr_name);
      setText(id, grpname);
    }
    // if we've got an extention column, clip the name accordingly
    QString name = _vf->vfile_getName(), ext = "";
    if ((id = parent->column(KrDetailedView::Extention)) != KrDetailedView::Unused) {
      int i;
      if ((i = name.findRev('.')) >= 0) {
        ext = name.mid(i+1);
        name = name.mid(0, i);
      }
      setText(id, ext);
    }
    setText(parent->column(KrDetailedView::Name), name);
    // display an icon if needed
    if (parent->_withIcons)
      setPixmap(parent->column(KrDetailedView::Name),KrView::getIcon(_vf));

}

void KrViewItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align) {
  // center the <DIR> thing if needed
  if( (column == _view->column(KrDetailedView::Size)) && _vf->vfile_isDir() )
    align = Qt::AlignHCenter;
  KListViewItem::paintCell(p, cg, column, width, align);
}

QString KrViewItem::key(int column, bool ascending) const {
  // TODO
  return KListViewItem::key(column, ascending);
}
