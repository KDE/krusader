/***************************************************************************
                              krdetailedview.cpp
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
#include "krdetailedview.h"
#include "../kicons.h"
#include "../VFS/krpermhandler.h"
#include <kconfigbase.h>
#include <qlayout.h>
#include <qdir.h>

//////////////////////////////////////////////////////////////////////////
//  The following is KrDetailedView's settings in KConfig:
// Group name: KrDetailedView
//
// Show Status
#define _ShowStatus     true
// Show Totals
#define _ShowTotals     true
// With Icons
#define _WithIcons      true
// Ext Column
#define _ExtColumn      false
// Mime Column
#define _MimeColumn     false
// Size Column
#define _SizeColumn     true
// DateTime Column
#define _DateTimeColumn true
// Perm Column
#define _PermColumn     false
// KrPerm Column
#define _KrPermColumn   true
// Owner Column
#define _OwnerColumn    false
// Group Column
#define _GroupColumn    false
//////////////////////////////////////////////////////////////////////////

QString KrDetailedView::ColumnName[] = { i18n("Name"), i18n("Ext"), i18n("Type"),
  i18n("Size"), i18n("Modified"), i18n("Permissions"), i18n("Permission*"),
  i18n("Owner"), i18n("Group") };

KrDetailedView::KrDetailedView(QWidget *parent, KConfig *cfg = krConfig, const char *name ):
  KrView(parent, cfg, name) {
  // setup the default sort and filter
  _filter = KrView::All;
  _sortMode = static_cast<SortSpec>(KrView::Name | KrView::Descending |
                                    KrView::DirsFirst | KrView::IgnoreCase);

  // first, clear the columns list. it will be updated by addColumn()
  for (int i=0; i<MAX_COLUMNS; i++) _columns[i] = Unused;

  // we need a layout, some labels and a listview
  QBoxLayout *layout = new QVBoxLayout(this);
  _status = new KSqueezedTextLabel(this);
  _listview = new KListView(this);
  _totals = new KSqueezedTextLabel(this);
  layout->addWidget(_status);
  layout->addWidget(_listview);
  layout->addWidget(_totals);

  // add whatever columns are needed to the listview
  KConfigGroupSaver grpSvr(_config, GROUP);
  _withIcons = _config->readBoolEntry("With Icons", _WithIcons); // we we display icons ?
  addColumn(Name);  // we always have a name
  if (_config->readBoolEntry("Ext Column", _ExtColumn)) addColumn(Extention);
  if (_config->readBoolEntry("Mime Column", _MimeColumn)) addColumn(Mime);
  if (_config->readBoolEntry("Size Column", _SizeColumn)) addColumn(Size);
  if (_config->readBoolEntry("DateTime Column", _DateTimeColumn)) addColumn(DateTime);
  if (_config->readBoolEntry("Perm Column", _PermColumn)) addColumn(Permissions);
  if (_config->readBoolEntry("KrPerm Column", _KrPermColumn)) addColumn(KrPermissions);
  if (_config->readBoolEntry("Owner Column", _OwnerColumn)) addColumn(Owner);
  if (_config->readBoolEntry("Group Column", _GroupColumn)) addColumn(Group);

  // determine basic settings for the listview
  _listview->setAcceptDrops(true); _listview->setDragEnabled(true);
  _listview->setDropVisualizer(true); _listview->setDropHighlighter(true);
  _listview->setSelectionModeExt(KListView::FileManager);


  // show what parts requested
  if (!_config->readBoolEntry("Show Status", _ShowStatus)) _status->hide();
  if (!_config->readBoolEntry("Show Totals", _ShowTotals)) _totals->hide();
}

KrDetailedView::~KrDetailedView(){
}

void KrDetailedView::addColumn(ColumnType type) {
  int i;
  for (i=0; i<MAX_COLUMNS; i++)
    if (_columns[i] == Unused) break;
  if (i>=MAX_COLUMNS) perror("KrDetailedView::addColumn() - too many columns");
  // add the new type to the column handler
  _columns[i] = type;
  _listview->addColumn(ColumnName[type]);
}

/**
 * returns the number of column which holds values of type 'type'.
 * if such values are not presented in the view, -1 is returned.
 */
int KrDetailedView::column(ColumnType type) {
  for (int i=0; i<MAX_COLUMNS; i++)
    if (_columns[i] == type) return i;
  return -1;
}

void KrDetailedView::addItems(vfs *v, bool addUpDir) {
	QListViewItem *item = _listview->firstChild();
	QListViewItem *currentItem = item;
	QString size, name;
	
  // add the up-dir arrow if needed
  if (addUpDir) {
    KListViewItem *item=new KListViewItem(_listview);
    item->setText(column(Name), "..");
    item->setText(column(Size), "<DIR>");
		item->setPixmap(column(Name),FL_LOADICON("up"));
	  item->setSelectable(false);
  }

	for( vfile* vf=v->vfs_getFirstFile(); vf != 0 ; vf=v->vfs_getNextFile() ){
		size =  KRpermHandler::parseSize(vf->vfile_getSize());
		name =  vf->vfile_getName();
		bool isDir = vf->vfile_isDir();
		/*KRListItem::cmpColor color = KRListItem::none;
		if( otherPanel->type == "list" && compareMode ){
		  vfile* ovf = otherPanel->files->vfs_search(vf->vfile_getName());
		  if (ovf == 0 ) color = KRListItem::exclusive;  // this file doesn't exist on the other panel
		  else{ // if we found such a file
        QString date1 = KRpermHandler::date2qstring(vf->vfile_getDateTime());
        QString date2 = KRpermHandler::date2qstring(ovf->vfile_getDateTime());
        if (date1 > date2) color = KRListItem::newer; // this file is newer than the other
        else
        if (date1 < date2) color = KRListItem::older; // this file is older than the other
        else
        if (date1 == date2) color = KRListItem::identical; // the files are the same
		  }
		}*/
		
		if(!isDir || (isDir && (_filter & ApplyToDirs))){
			switch(_filter){
    		case KrView::All    : break;
				case KrView::Custom : if (!QDir::match(_filterMask,name)) continue;
				                      break;
				/*case KrView::EXEC:  if (!vf->vfile_isExecutable()) continue;
											        break; */
			}
		  /*if ( compareMode && !(color & colorMask) ) continue;*/
		}

		item = new KrViewItem(this, item, vf);

		// if the item should be current - make it so
		if ((dynamic_cast<KrViewItem*>(item))->name() == nameToMakeCurrent())
      currentItem = item;
	}
	
  _listview->setCurrentItem(currentItem);
	ensureItemVisible(currentItem);
}
