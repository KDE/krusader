/***************************************************************************
                                kvfspanel.cpp
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


#include <qregexp.h>
// KDE includes
#include <kapp.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kglobalsettings.h>
// Krusader includes
#include "kvfspanel.h"
#include "kfilelist.h"
#include "krlistitem.h"
#include "../defaults.h"
#include "../krusader.h"
// Krusader includes
#include "../resources.h"
#include "../VFS/vfs.h"
#include "panelfunc.h"
#include "kfilelist.h"
#include <kdeversion.h>

void KVFSPanel::slotFocusOnMe() {
	krConfig->setGroup("Look&Feel");
	////////////////////////////////
	otherPanel->focused=false;
  QPalette q( otherPanel->status->palette() );
  q.setColor( QColorGroup::Foreground,KGlobalSettings::textColor());
  q.setColor( QColorGroup::Background,KGlobalSettings::baseColor());

  otherPanel->origin->setPalette(q);
  otherPanel->status->setPalette(q);
  otherPanel->totals->setPalette(q);
  if (otherPanel->type == "list"){
    otherPanel->bookmarkList->setBackgroundMode(PaletteBackground);
    otherPanel->bookmarkList->setPalette(q);
  }

  focused=true;
  //QPalette p( KGlobalSettings::activeTextColor(),KGlobalSettings::activeTitleColor());
  QPalette p( status->palette() );
  p.setColor( QColorGroup::Foreground,KGlobalSettings::highlightedTextColor() );
  p.setColor( QColorGroup::Background,KGlobalSettings::highlightColor() );
  status->setPalette(p);
  origin->setPalette(p);
  totals->setPalette(p);
  if (type == "list"){
    bookmarkList->setBackgroundMode(PaletteBackground);
    bookmarkList->setPalette(p);
	}
	fileList->setFocus();     // get the keyboard attention to the file list
	emit cmdLineUpdate(realPath);
	emit activePanelChanged(this);
}

QString KVFSPanel::shortPath(){
  QString s = virtualPath ,s1,s2;
	s.replace( QRegExp("\\"),"#");
	
	if ( QFontMetrics(origin->font()).width(s)+40 < origin->width() ) return s;

	s1 = s.left((s.length()/2));
	s2 = s.right((s.length()/2));
	while( QFontMetrics(origin->font()).width(s1+"..."+s2)+40 > origin->width() ){
		s1.truncate(s1.length()-1);
		s2 = s2.right(s2.length()-1);
  }
  return s1+"..."+s2;
}

// this is called if the rightclick menu is popped from the keyboard
void KVFSPanel::popRightClickMenu() {
  // find out what's the current item and it's location
  QListViewItem *item = fileList->currentItem();
  if (item == 0) return;
  QPoint loc(fileList->mapToGlobal(fileList->itemRect(item).topLeft()));
  popRightClickMenu(fileList, item, loc);
}

#include "kvfspanel.moc"

