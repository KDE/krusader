#include "krbriefview.h"
#include "krbriefviewitem.h"
#include "../defaults.h"
#include "../kicons.h"
#include <kconfig.h>

#define PROPS	_viewProperties
#define VF	getVfile()

int KrBriefViewItem::expHeight = 0;

KrBriefViewItem::KrBriefViewItem(KrBriefView *parent, QIconViewItem *after, vfile *vf):
	KIconViewItem(parent, after), KrViewItem(vf, parent->properties()) {
	initiated = false;
	// get the expected height of an item - should be done only once
	if (expHeight == 0) {
		KConfigGroupSaver svr(krConfig, "Look&Feel");
  		expHeight = 2 + (krConfig->readEntry("Filelist Icon Size",_FilelistIconSize)).toInt();
	}

	// there's a special case, where if _vf is null, then we've got the ".." (updir) item
	// in that case, create a special vfile for that item, and delete it, if needed
	if (!_vf) {
		dummyVfile = true;
		_vf = new vfile("..", 0, "drw-r--r--", 0, false, 0, 0, QString::null, QString::null, 0);
		
		setText("..");
      if ( PROPS->displayIcons )
         setPixmap( FL_LOADICON( "up" ) );
      setSelectable( false );
		initiated = true;
	}
	
	repaintItem();
}

int KrBriefViewItem::compare(QIconViewItem *i ) const {
  bool ignoreCase = (PROPS->sortMode & KrViewProperties::IgnoreCase);

  // TODO: int asc = ( ascending ? -1 : 1 );
  int asc = -1;

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

	// localeAwareCompare doesn't handle names that start with a dot
	if (text0.startsWith(".")) {
		if (!itext0.startsWith(".")) return 1*asc;
	} else if (itext0.startsWith(".")) return -1*asc;
	if (!ignoreCase && !PROPS->localeAwareCompareIsCaseSensitive) {
		// sometimes, localeAwareCompare is not case sensative. in that case,
		// we need to fallback to a simple string compare (KDE bug #40131)
		return QString::compare(text0, itext0);
	} else return QString::localeAwareCompare(text0,itext0);
}

void KrBriefViewItem::paintItem(QPainter *p, const QColorGroup &cg) {
	if (!initiated && !dummyVfile) {
		// display an icon if needed
		if (PROPS->displayIcons)
			setPixmap(KrView::getIcon(_vf));
		
		initiated = true;
	}
	KIconViewItem::paintItem(p,cg);

#if 0 // TODO  
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
#endif
}

void KrBriefViewItem::itemHeightChanged() {
	expHeight = 0;
}
