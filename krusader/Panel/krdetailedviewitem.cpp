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
#include "../krusaderview.h"
#include "krdetailedviewitem.h"
#include "krdetailedview.h"
#include "krcolorcache.h"
#include "listpanel.h"
#include "../VFS/krpermhandler.h"
#include <sys/types.h>
#include <time.h>
#include <qpainter.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <qpalette.h>
#include <kdebug.h>
#include <kmimetype.h>

#define COLUMN(X)	static_cast<const KrDetailedViewProperties*>(_viewProperties)->	\
	column[ KrDetailedViewProperties::X ]
#define PROPS	static_cast<const KrDetailedViewProperties*>(_viewProperties)
#define PERM_BITMASK (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)
#define VF	getVfile()

#ifdef FASTER
int KrDetailedViewItem::expHeight = 0;
#endif // FASTER

KrDetailedViewItem::KrDetailedViewItem(KrDetailedView *parent, QListViewItem *after, vfile *vf):
	KListViewItem(parent, after), KrViewItem(vf, parent->properties()) {
#ifdef FASTER
	initiated = false;
	// get the expected height of an item - should be done only once
	if (expHeight == 0) {
		KConfigGroupSaver svr(krConfig, "Look&Feel");
  		expHeight = 2 + (krConfig->readEntry("Filelist Icon Size",_FilelistIconSize)).toInt();
	}

#endif	
	// there's a special case, where if _vf is null, then we've got the ".." (updir) item
	// in that case, create a special vfile for that item, and delete it, if needed
	if (!_vf) {
		dummyVfile = true;
		_vf = new vfile("..", 0, "drw-r--r--", 0, false, 0, 0, QString::null, QString::null, 0);
		
		setText(COLUMN(Name), "..");
		setText(COLUMN(Size), "<DIR>" );
      if ( PROPS->displayIcons )
         setPixmap( COLUMN(Name), FL_LOADICON( "up" ) );
      setSelectable( false );
#ifdef FASTER
		initiated = true;
#endif
	}
	
	repaintItem();
}

#ifdef FASTER
void KrDetailedViewItem::setup() {
	// idea: when not having pixmaps in the first place, the height of the item is smaller then with
	// the pixmap. when the pixmap is inserted, the item resizes, thereby making ensureItemVisible()
	// become 'confused' and stop working. therefore, we set the correct height here and avoid the issue
	KListViewItem::setup();
	setHeight(expHeight);
}
#endif

void KrDetailedViewItem::repaintItem() {
    if ( dummyVfile ) return;
    QString tmp;
    // set text in columns, according to what columns are available
    int id = KrDetailedViewProperties::Unused;
    if ((id = COLUMN(Mime)) != -1) {
      tmp = KMimeType::mimeType(_vf->vfile_getMime())->comment();
      setText( id, tmp );
    }
    if ((id = COLUMN(Size)) != -1) {
      if (_vf->vfile_isDir() && _vf->vfile_getSize() <= 0) setText(id, "<DIR>");
	    else setText(id, PROPS->humanReadableSize ? KIO::convertSize(_vf->vfile_getSize())+"  " :
		 						KRpermHandler::parseSize(_vf->vfile_getSize())+" ");
    }

    if ((id = COLUMN(DateTime)) != -1)
      setText(id, dateTime());
    if ((id = COLUMN(KrPermissions)) != -1) {
      // first, build the krusader permissions
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
    if ((id = COLUMN(Permissions) ) != -1) {
		if (PROPS->numericPermissions) {
      	setText(id, tmp.sprintf("%.4o", _vf->vfile_getMode() & PERM_BITMASK));
		} else setText(id, _vf->vfile_getPerm());
	 }
    if ((id = COLUMN(Owner)) != -1) {
      setText(id, _vf->vfile_getOwner());
    }
    if ((id = COLUMN(Group)) != -1) {
      setText(id, _vf->vfile_getGroup());
    }
    // if we've got an extention column, clip the name accordingly
    QString name = _vf->vfile_getName(), ext = "";
    if ((id = COLUMN(Extention)) != -1 && !_vf->vfile_isDir()) {
      int i;
      if ((i = name.findRev('.')) > 0) {
        ext = name.mid(i+1);
        name = name.mid(0, i);
      }
      setText(id, ext);
    }
    setText(COLUMN(Name), name);
#ifndef FASTER
    // display an icon if needed
    if (PROPS->displayIcons)
      setPixmap(COLUMN(Name),KrView::getIcon(_vf));
#endif
}

QString num2qstring(KIO::filesize_t num){
  char buf[25];
  sprintf(buf,"%025llu",num);
  return QString(buf);
}

void KrDetailedViewItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align) {
#ifdef FASTER
	if (!initiated && !dummyVfile) {
		// display an icon if needed
		if (PROPS->displayIcons)
			setPixmap(COLUMN(Name),KrView::getIcon(_vf));
		
		initiated = true;
	}
#endif
  
  QColorGroup _cg(cg);

   // This is ugly! I had to dublicate KListViewItem::paintCell() code, as the
   // KListViewItem::paintCell() overwrites my color settings. So KrDetailedViewItem::paintCell
   // must dublicate the KListViewItem::paintCell() code, do the required color settings
   // and call QListViewItem::paintCell() afterwards (the base class of KListViewItem).
   // This tabooed in the object oriented heaven, but necessary here. Blame the KDE team for
   // this really poor paintCell implementation!
   
   const QPixmap *pm = listView()->viewport()->backgroundPixmap();
   if (pm && !pm->isNull())
   {
         _cg.setBrush(QColorGroup::Base, QBrush(backgroundColor(), *pm));
         p->setBrushOrigin( -listView()->contentsX(), -listView()->contentsY() );
   }
   else if (isAlternate())
        if (listView()->viewport()->backgroundMode()==Qt::FixedColor)
             _cg.setColor(QColorGroup::Background, static_cast< KListView* >(listView())->alternateBackground());
       else
             _cg.setColor(QColorGroup::Base, static_cast< KListView* >(listView())->alternateBackground());

  // end of uglyness

  
  // begin of custom color calculation
  krConfig->setGroup("Colors");
  
  // if KDE deafault: do not touch color group!
  if (!KrColorCache::getColorCache().isKDEDefault())
  {
    bool markCurrentAlways = KrColorCache::getColorCache().isShowCurrentItemAlways();
    bool isActive = (dynamic_cast<KrView *>(listView()) == ACTIVE_PANEL->view);
    bool isCurrent = listView()->currentItem() == this;
    
    // First calculate fore- and background.
    QColor background = isAlternate()?KrColorCache::getColorCache().getAlternateBackgroundColor(isActive):KrColorCache::getColorCache().getBackgroundColor(isActive);
    QColor foreground;
    if (VF->vfile_isSymLink())
    {
       if (_vf->vfile_getMime() == "Broken Link !" )
          foreground = KrColorCache::getColorCache().getInvalidSymlinkForegroundColor(isActive);
       else
          foreground = KrColorCache::getColorCache().getSymlinkForegroundColor(isActive);
    }
    else if (VF->vfile_isDir())
       foreground = KrColorCache::getColorCache().getDirectoryForegroundColor(isActive);
    else if (VF->vfile_isExecutable())
       foreground = KrColorCache::getColorCache().getExecutableForegroundColor(isActive);
    else
       foreground = KrColorCache::getColorCache().getForegroundColor(isActive);
       
    // set the background color
    _cg.setColor(QColorGroup::Base, background);
    _cg.setColor(QColorGroup::Background, background);
      
    // set the foreground color
    _cg.setColor(QColorGroup::Text, foreground);

    // now the color of a marked item
    QColor markedBackground = isAlternate()?KrColorCache::getColorCache().getAlternateMarkedBackgroundColor(isActive):KrColorCache::getColorCache().getMarkedBackgroundColor(isActive);
    QColor markedForeground = KrColorCache::getColorCache().getMarkedForegroundColor(isActive);
    if (!markedForeground.isValid()) // transparent
       // choose fore- or background, depending on its contrast compared to markedBackground
       markedForeground = setColorIfContrastIsSufficient(markedBackground, foreground, background);

      // set it in the color group (different group color than normal foreground!)
    _cg.setColor(QColorGroup::HighlightedText, markedForeground);
    _cg.setColor(QColorGroup::Highlight, markedBackground);

    // In case the current item is a selected one, set the fore- and background colors for the contrast calculation below
    if (isSelected())
    {
        background = markedBackground;
        foreground = markedForeground;
    }

    // finally the current item
    if (isCurrent && (markCurrentAlways || isActive))
    // if this is the current item AND the panels has tho focus OR the current should be marked always
    {
       QColor currentBackground = KrColorCache::getColorCache().getCurrentBackgroundColor(isActive);
       if (!currentBackground.isValid()) // transparent
          currentBackground = background;
       
       // set the background
       _cg.setColor(QColorGroup::Highlight, currentBackground);
       _cg.setColor(QColorGroup::Base, currentBackground);
       _cg.setColor(QColorGroup::Background, currentBackground);

       QColor color;
       if (isSelected()) 
          color = KrColorCache::getColorCache().getCurrentMarkedForegroundColor(isActive);
       if (!color.isValid()) // not used
       {
          color = KrColorCache::getColorCache().getCurrentForegroundColor(isActive);
          if (!color.isValid()) // transparent
            // choose fore- or background, depending on its contrast compared to markedBackground
            color = setColorIfContrastIsSufficient(currentBackground, foreground, background);
       }
       
       // set the foreground
       _cg.setColor(QColorGroup::Text, color);
       _cg.setColor(QColorGroup::HighlightedText, color);
    }
  }

	// center the <DIR> thing if needed
	if(column != COLUMN(Size))
		QListViewItem::paintCell(p, _cg, column, width, align);
	else {
  		if (dummyVfile) {
			QListViewItem::paintCell(p, _cg, column, width, Qt::AlignHCenter); // updir
  		} else {
    		if (_vf->vfile_isDir() && _vf->vfile_getSize()<=0)
      		QListViewItem::paintCell(p, _cg, column, width, Qt::AlignHCenter);
    		else QListViewItem::paintCell(p, _cg, column, width, align); // size
  		}
	}
}

const QColor & KrDetailedViewItem::setColorIfContrastIsSufficient(const QColor & background, const QColor & color1, const QColor & color2)
{
   #define sqr(x) ((x)*(x))
   int contrast = sqr(color1.red() - background.red()) + sqr(color1.green() - background.green()) + sqr(color1.blue() - background.blue());

   // if the contrast between background and color1 is too small, take color2 instead.
   if (contrast < 1000)
      return color2;
   return color1;
}

QPixmap KrDetailedViewItem::icon() {
#if 0  
  QPixmap *p;

  // This is bad - very bad. the function must return a valid reference,
  // This is an interface flow - shie please fix it with a function that return QPixmap*
  // this way we can return 0 - and do our error checking...
  
  // shie answers: why? what's the difference? if we return an empty pixmap, others can use it as it
  // is, without worrying or needing to do error checking. empty pixmap displays nothing
#endif
	if (dummyVfile || !PROPS->displayIcons)
		return QPixmap();
	else return KrView::getIcon(_vf);
}

int KrDetailedViewItem::compare(QListViewItem *i,int col,bool ascending ) const {
  bool ignoreCase = (PROPS->sortMode & KrViewProperties::IgnoreCase);
  int asc = ( ascending ? -1 : 1 );
  KrViewItem *other = dynamic_cast<KrViewItem*>(i);

  // handle directory sorting
  if (VF->vfile_isDir()){
    if (!other->VF->vfile_isDir()) return 1*asc;
  } else if(other->VF->vfile_isDir()) return -1*asc;

  QString text0 = name();
  if (text0 == "..") return 1*asc;
  
  QString itext0 = other->name();
  if (itext0 == "..") return -1*asc;

  if( ignoreCase )
  {
    text0  = text0.lower();
    itext0 = itext0.lower();
  }

  if (col == COLUMN(Name) ) {
      // localeAwareCompare doesn't handle names that start with a dot
		if (text0.startsWith(".")) {
			if (!itext0.startsWith(".")) return 1*asc;
		} else if (itext0.startsWith(".")) return -1*asc;
		if (!ignoreCase && !PROPS->localeAwareCompareIsCaseSensitive) {
			// sometimes, localeAwareCompare is not case sensative. in that case,
			// we need to fallback to a simple string compare (KDE bug #40131)
			return QString::compare(text0, itext0);
		} else return QString::localeAwareCompare(text0,itext0);
  } else if (col == COLUMN(Size) ) {
      return QString::compare(num2qstring(VF->vfile_getSize()),num2qstring(other->VF->vfile_getSize()));
  } else if (col == COLUMN(DateTime) ) {
      return (VF->vfile_getTime_t() > other->VF->vfile_getTime_t() ? 1 : -1);
  } else if (col == COLUMN(Permissions) && PROPS->numericPermissions) {
		return ((text(col).toLong() > i->text(col).toLong()) ? 1 : -1);
  } else {
      QString e1 = (!ignoreCase ? text(col) : text(col).lower());
      QString e2 = (!ignoreCase ? i->text(col) : i->text(col).lower());
		if (!ignoreCase && !PROPS->localeAwareCompareIsCaseSensitive) {
			// sometimes, localeAwareCompare is not case sensative. in that case,
			// we need to fallback to a simple string compare (KDE bug #40131)
			return QString::compare(e1, e2);
		} else return QString::localeAwareCompare(e1, e2);
  }
}

QString KrDetailedViewItem::dateTime() const {
   // convert the time_t to struct tm
   time_t time = VF->vfile_getTime_t();
   struct tm* t=localtime((time_t *)&time);

   QDateTime tmp(QDate(t->tm_year+1900, t->tm_mon+1, t->tm_mday), QTime(t->tm_hour, t->tm_min));
   return KGlobal::locale()->formatDateTime(tmp);
}

void KrDetailedViewItem::itemHeightChanged() {
#ifdef FASTER
	expHeight = 0;
#endif
}
