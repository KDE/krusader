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

KrDetailedViewItem::KrDetailedViewItem(KrDetailedView *parent, QListViewItem *after, vfile *vf):
  KListViewItem(parent, after), KrViewItem(),_vf(vf), _view(parent) {
  
  caseSensitiveSort = !(_view->sortMode() & KrView::IgnoreCase);
    
  nameColumn        = _view->column(KrDetailedView::Name);        // the columns are stored for faster comparation
  sizeColumn        = _view->column(KrDetailedView::Size);
  dateTimeColumn    = _view->column(KrDetailedView::DateTime);
  mimeColumn        = _view->column(KrDetailedView::Mime);
  krPermColumn      = _view->column(KrDetailedView::KrPermissions);
  permColumn        = _view->column(KrDetailedView::Permissions);
  ownerColumn       = _view->column(KrDetailedView::Owner);
  groupColumn       = _view->column(KrDetailedView::Group);
  extColumn         = _view->column(KrDetailedView::Extention);

	{
	KConfigGroupSaver saver(krConfig, "Look&Feel");
	humanReadableSize = krConfig->readBoolEntry("Human Readable Size", _HumanReadableSize);
  	}
  repaintItem();
}

void KrDetailedViewItem::repaintItem() {
    if ( !_vf ) return;
    // set text in columns, according to what columns are available
    int id = KrDetailedView::Unused;
    if ((id = mimeColumn) != -1) {
      QString tmp = _vf->vfile_getMime();
      setText(id, tmp.mid(tmp.find('/')+1));
    }
    if ((id = sizeColumn) != -1) {
      if (_vf->vfile_isDir() && _vf->vfile_getSize() <= 0) setText(id, "<DIR>");
	    else setText(id, humanReadableSize ? KIO::convertSize(_vf->vfile_getSize())+"  " :
		 						KRpermHandler::parseSize(_vf->vfile_getSize())+" ");
    }

    if ((id = dateTimeColumn) != -1)
      setText(id, dateTime());
    if ((id = krPermColumn) != -1) {
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
    if ((id = permColumn ) != -1)
      setText(id, _vf->vfile_getPerm());
    if ((id = ownerColumn) != -1) {
      setText(id, _vf->vfile_getOwner());
    }
    if ((id = groupColumn) != -1) {
      setText(id, _vf->vfile_getGroup());
    }
    // if we've got an extention column, clip the name accordingly
    QString name = _vf->vfile_getName(), ext = "";
    if ((id = extColumn) != -1 && !_vf->vfile_isDir()) {
      int i;
      if ((i = name.findRev('.')) > 0) {
        ext = name.mid(i+1);
        name = name.mid(0, i);
      }
      setText(id, ext);
    }
    setText(nameColumn, name);
    // display an icon if needed
    if (_view->_withIcons)
      setPixmap(nameColumn,KrView::getIcon(_vf));
}

QString num2qstring(KIO::filesize_t num){
  char buf[25];
  sprintf(buf,"%025llu",num);
  return QString(buf);
}

QString KrDetailedViewItem::name() const {
  if (_vf) return _vf->vfile_getName();
  else return text(nameColumn);
}

void KrDetailedViewItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align) {
  QColorGroup _cg(cg);
/**************** removed new selection mode for now *************************
  // kdelibs' paintCell //////////////////////////////////////////
  const QPixmap *pm = listView()->viewport()->backgroundPixmap();
  if (pm && !pm->isNull())
  {
        _cg.setBrush(QColorGroup::Base, QBrush(backgroundColor(), *pm));
        p->setBrushOrigin( -listView()->contentsX(), -listView()->contentsY() );
  }
  else
  if (isAlternate())
       if (listView()->viewport()->backgroundMode()==Qt::FixedColor)
            _cg.setColor(QColorGroup::Background, static_cast< KListView* >(listView())->alternateBackground());
       else
        _cg.setColor(QColorGroup::Base, static_cast< KListView* >(listView())->alternateBackground());
  // end of kdelibs' paintCell ///////////////////////////////////

#define COLOR _cg.color(QColorGroup::Link)
  // make selected items colored (wincmd style)
  if (isSelected()) {
      if (_view->getCurrentKrViewItem() == this) { // selected and current
         // for visual comfortability, don't color it red if it's
         // the only file that's selected
         if (!_view->automaticSelection())
            _cg.setColor(QColorGroup::HighlightedText, COLOR);
         else _cg.setColor(QColorGroup::HighlightedText, _cg.color(QColorGroup::Foreground));
         _cg.setColor(QColorGroup::Highlight, backgroundColor()); //?
      } else { // selected but not current
         _cg.setColor(QColorGroup::HighlightedText, COLOR);
         _cg.setColor(QColorGroup::Highlight, backgroundColor());
      }
  } else if (_view->getCurrentKrViewItem() == this) { // current but not selected
         _cg.setColor(QColorGroup::Base, backgroundColor());
  } else { // not selected

  }
*****************************************************************************/

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
    if (isSymLink())
    {
       if (_vf->vfile_getMime() == "Broken Link !" )
          foreground = KrColorCache::getColorCache().getInvalidSymlinkForegroundColor(isActive);
       else
          foreground = KrColorCache::getColorCache().getSymlinkForegroundColor(isActive);
    }
    else if (isDir())
       foreground = KrColorCache::getColorCache().getDirectoryForegroundColor(isActive);
    else if (isExecutable())
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
  if(column != _view->column(KrDetailedView::Size))
   QListViewItem::paintCell(p, _cg, column, width, align);
  else if (_vf) {
    if (_vf->vfile_isDir() && _vf->vfile_getSize()<=0)
      QListViewItem::paintCell(p, _cg, column, width, Qt::AlignHCenter);
    else QListViewItem::paintCell(p, _cg, column, width, align); // size
  } else QListViewItem::paintCell(p, _cg, column, width, Qt::AlignHCenter); // updir
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
  if ( !_vf || _view->_withIcons)
    p = new QPixmap(*(pixmap(_view->column(KrDetailedView::Name))));
  else p = new QPixmap(KrView::getIcon(_vf));
  return *p;
#endif
	if (!_vf || !_view->_withIcons)
		return QPixmap();
	else return KrView::getIcon(_vf);
}

int KrDetailedViewItem::compare(QListViewItem *i,int col,bool ascending ) const {
  int asc = ( ascending ? -1 : 1 );
  KrViewItem *other = dynamic_cast<KrViewItem*>(i);

  // handle directory sorting
  if (isDir()){
    if (!other->isDir()) return 1*asc;
  } else if(other->isDir()) return -1*asc;

  QString text0 = name();
  if (text0 == "..") return 1*asc;
  
  QString itext0 = other->name();
  if (itext0 == "..") return -1*asc;

  if( !caseSensitiveSort )
  {
    text0  = text0.lower();
    itext0 = itext0.lower();
  }

  //kdDebug() << "text0: "<< text0 << " ,itext0: "<<itext0 << endl;

  if (col == nameColumn ) {
      // localeAwareCompare doesn't handle names that start with a dot
		if (text0.startsWith(".")) {
			if (!itext0.startsWith(".")) return 1*asc;
		} else if (itext0.startsWith(".")) return -1*asc;
		return QString::localeAwareCompare(text0,itext0);
  } else if (col == sizeColumn ) {
      return QString::compare(num2qstring(size()),num2qstring(other->size()));
  } else if (col == dateTimeColumn ) {
      return (getTime_t() > other->getTime_t() ? 1 : -1);
  } else {
      QString e1 = (caseSensitiveSort ? text(col) : text(col).lower());
      QString e2 = (caseSensitiveSort ? i->text(col) : i->text(col).lower());
      return QString::localeAwareCompare(e1, e2);
  }
}

QString KrDetailedViewItem::description() const {
 	if (name()=="..") return i18n("Climb up the directory tree");
	else if(_vf){
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
     	text = QString("%1 (%2)").arg(text2).arg( humanReadableSize ?
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

  return "";
}

QString KrDetailedViewItem::dateTime() const {
   // convert the time_t to struct tm
   time_t time = getTime_t();
   struct tm* t=localtime((time_t *)&time);

   QDateTime tmp(QDate(t->tm_year+1900, t->tm_mon+1, t->tm_mday), QTime(t->tm_hour, t->tm_min));
   return KGlobal::locale()->formatDateTime(tmp);
}

