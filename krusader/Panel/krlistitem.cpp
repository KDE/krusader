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

QString num2qstring(QString text){
  long num = text.replace(QRegExp(","),"").toLong();

  QString temp;
  temp.sprintf("%015ld",num);
  return temp;
}

int KRListItem::compare(QListViewItem *i,int col,bool ascending ) const {
  int asc = ( ascending ? -1 : 1 );
 
  if (text(0)== "..") return 1*asc;
	if (i->text(0)== "..") return -1*asc;

  if( text(1)== "<DIR>" ){
    if( i->text(1) != "<DIR>" ) return 1*asc;
    else return QString::compare(text(0),i->text(0));
  }

  if( i->text(1) == "<DIR>" ) return -1*asc;

  QString text0 = text(0);
  QString itext0 = i->text(0);

  krConfig->setGroup("Look&Feel");
  if(!krConfig->readBoolEntry("Case Sensative Sort",_CaseSensativeSort)) {
    text0  = text0.lower();
    itext0 = itext0.lower();
  }

  int result = 0;

  switch (col) {
  	case 0:
    	result = QString::compare(text0,itext0);
			break;
		case 1:
    	result = QString::compare(num2qstring(text(1)),num2qstring(i->text(1)));
			break;
 		case 2:
			QString  d = "" +
      		text(2)[6] + text(2)[7] + // year
					text(2)[3] + text(2)[4] + // month
					text(2)[0] + text(2)[1] + // day
          text(2)[9] + text(2)[10]+ //	hour
					text(2)[12]+ text(2)[13]; // minute
			QString  id = "" +
      		i->text(2)[6] + i->text(2)[7] + // year
					i->text(2)[3] + i->text(2)[4] + // month
					i->text(2)[0] + i->text(2)[1] + // day
          i->text(2)[9] + i->text(2)[10]+ //	hour
					i->text(2)[12]+ i->text(2)[13]; // minute
    	result = QString::compare(d,id);
			break;
  }

  if( krToggleSortByExt->isChecked() ){
    QString ext, iext;

    if( text0.find('.',2) != -1) ext = text0.mid(text0.findRev('.'));
    if( itext0.find('.',2) != -1) iext = itext0.mid(itext0.findRev('.'));
    int extResult = QString::compare(ext,iext);
    if( extResult != 0 ) return extResult;
  }
  return result;
}

KRListItem::~KRListItem(){
}

void KRListItem::paintCell(QPainter *p,const QColorGroup & cg,int column,int width,int align){
  if( column == 1 && text(1) == "<DIR>" )
    align = Qt::AlignHCenter;  // center the <DIR> text...
  QListViewItem::paintCell(p,cg,column,width,align);
}
