/***************************************************************************
                                krlistitem.cpp
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

#include "krlistitem.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../VFS/krpermhandler.h"
#include <ctype.h>
#include <qregexp.h>

QString num2qstring(long num){
  QString temp;
  temp.sprintf("%015ld",num);
  return temp;
}

QString KRListItem::key ( int column, bool ascending ) const {
    bool ext = krToggleSortByExt->isChecked();
    char max[10] = {255,255,255,255,255,255,255,255,255,0};
    char dir [7] = {255,255,255,255,255,255,0};
    char file[4] = {255,255,255,0};

    if (text(0)==".."){
      if (ascending) return "";
      else return max;
    }

    QString text0 = text(0);

    QString ext0;
    if (ext && text0.find('.') != -1)
      ext0 = text0.mid(text0.findRev('.'));

    krConfig->setGroup("Look&Feel");
	  if(!krConfig->readBoolEntry("Case Sensative Sort",_CaseSensativeSort)) {
	    text0 = text0.lower();
      ext0 = ext0.lower();
    }

    if ( text(1)=="<DIR>"){
     if(ascending) return text0;
     else return dir+text0;
    }

    if (column == 0) return file+ext0+text0;
    if (column == 1) return file+ext0+num2qstring(text(1).replace(QRegExp(","),"").toLong());
    if (column == 2) return file+ext0+KRpermHandler::date2qstring(text(2));
    return "";
}

KRListItem::~KRListItem(){
}

void KRListItem::paintCell(QPainter *p,const QColorGroup & cg,int column,int width,int align){
  if( column == 1 && text(1) == "<DIR>" )
    align = Qt::AlignHCenter;  // center the <DIR> text...
  QListViewItem::paintCell(p,cg,column,width,align);
}
