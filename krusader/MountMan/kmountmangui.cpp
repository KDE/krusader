/***************************************************************************
                                kmountmangui.cpp
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



#include "kmountmangui.h"
#include "kmountman.h"
#include "../krusader.h"
#include "../Dialogs/krspecialwidgets.h"
#include "../kicons.h"
#include "../defaults.h"
#include "../VFS/vfs.h"
#include <klocale.h>
#include <qpixmap.h>
#include <kpopupmenu.h>
#include <qbitmap.h>
#include <kmessagebox.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <kprocess.h>
#include <kdebug.h>
#include <qcursor.h>

using namespace MountMan;

KMountManGUI::KMountManGUI() : KDialogBase(krApp, 0,true,"Mount.Man"),
      info(0), mountList(0) {
  watcher = new QTimer(this);
  connect(watcher,SIGNAL(timeout()), this, SLOT(checkMountChange())); //<>

  connect((QObject*)&krMtMan,SIGNAL(updated()),this,SLOT(updateList())); // <>
  setButtonOKText(i18n("&Close"),i18n("Close the Mount.Man window."),0);
  showButtonApply(false); showButtonCancel(false);
  setPlainCaption(i18n("MountMan - Your Mount-Manager"));
  widget=new KJanusWidget(this,0,KJanusWidget::Tabbed);
  createLayout();
  setMainWidget(widget);
  widget->setMinimumSize(widget->sizeHint().width()+mountList->columnWidth(5),
                         widget->sizeHint().height());
  setMinimumSize(widget->minimumSize().width(), widget->minimumSize().height());
  resize(minimumSize());

  // connections
  connect(mountList,SIGNAL(doubleClicked(QListViewItem *)),this,
          SLOT(doubleClicked(QListViewItem*)));
  connect(mountList,SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
          this, SLOT(clicked(QListViewItem*)));
  connect(mountList,SIGNAL(clicked(QListViewItem *)), this,
          SLOT(changeActive(QListViewItem *)));
  connect(mountList,SIGNAL(selectionChanged(QListViewItem *)), this,
        SLOT(changeActive(QListViewItem *)));

  exec();
}

KMountManGUI::~KMountManGUI()
{
  watcher->stop();
  delete watcher;
}

void KMountManGUI::createLayout() {
  mainPage=widget->addPage(i18n("Filesystems"),0);
  createMainPage();
  widget->showPage(Filesystems);
}

void KMountManGUI::createMainPage() {
  // check if we need to clean up first!
  if (mountList!=0) {
    mountList->hide();
    delete mountList;
    mountList=0;
  }
  // clean up is finished...
  QGridLayout *layout=new QGridLayout(mainPage,1,1);
  mountList=new QListView(mainPage);  // create the main container
  krConfig->setGroup("Look&Feel");
  mountList->setFont(krConfig->readFontEntry("Filelist Font",_FilelistFont));
	mountList->setAllColumnsShowFocus(true);
	mountList->setMultiSelection(false);
	mountList->setSelectionMode(QListView::Single);
	mountList->setVScrollBarMode(QScrollView::AlwaysOn);
	mountList->setHScrollBarMode(QScrollView::Auto);
	mountList->setShowSortIndicator(true);
	int i=QFontMetrics(mountList->font()).width("W");
	int j=QFontMetrics(mountList->font()).width("0");
	j=(i>j ? i : j);
	mountList->addColumn(i18n("Name"),j*8);
	mountList->addColumn(i18n("Type"),j*4);
	mountList->addColumn(i18n("Mnt.Point"),j*6);
	mountList->addColumn(i18n("Total Size"),j*6);
	mountList->addColumn(i18n("Free Size"),j*6);
	mountList->addColumn(i18n("Free %"),j*5);
	mountList->setColumnWidthMode(0,QListView::Maximum);
	mountList->setColumnWidthMode(1,QListView::Maximum);
	mountList->setColumnWidthMode(2,QListView::Maximum);
	mountList->setColumnWidthMode(3,QListView::Maximum);
	mountList->setColumnWidthMode(4,QListView::Maximum);
	mountList->setColumnWidthMode(5,QListView::Maximum);
  // now the list is created, time to fill it with data.
  krMtMan.forceUpdate();
  QGroupBox *box=new QGroupBox("MountMan.Info",mainPage);
  box->setAlignment(Qt::AlignHCenter);
  info=new KRFSDisplay(box);
  info->resize(info->width(),height());
  layout->addWidget(box,0,0);  
  layout->addWidget(mountList,0,1);
}

void KMountManGUI::updateList() {
  fsData *it;
  mountList->clear();
  for (it=krMtMan.filesystems.first(); it!=0 ; it=krMtMan.filesystems.next()) {
    bool mtd=it->mounted();
    QString tSize=QString("%1").arg(KIO::convertSizeFromKB(it->totalBlks()));
    QString fSize=QString("%1").arg(KIO::convertSizeFromKB(it->freeBlks()));
    QString sPrct=QString("%1%").arg(100-(it->usedPerct()));
    QListViewItem *item=new QListViewItem(mountList,it->name(),
       it->type(),it->mntPoint(),
       (mtd ? tSize : QString("N/A")),(mtd ? fSize : QString("N/A")),
       (mtd ? sPrct : QString("N/A")));
 		QString id=krMtMan.followLink(it->name()).left(2);
 		QPixmap *icon=0;
 		if (id=="fd" || krMtMan.followLink(it->name())=="floppy")
 		  icon=new QPixmap(LOADICON(mtd ? "3floppy_mount" : "3floppy_unmount"));
 		else
 		if (id=="cd" || it->type()=="iso9660") icon=new QPixmap(LOADICON(mtd ? "cdrom_mount" : "cdrom_unmount"));
 		else
 		if (it->type()=="nfs" || it->type()=="smbfs")
 		  icon=new QPixmap(LOADICON(mtd ? "nfs_mount" : "nfs_unmount"));
    else 		
 		if (id=="hd") icon=new QPixmap(LOADICON(mtd ? "hdd_mount" : "hdd_unmount"));
 		else
		icon=new QPixmap(LOADICON(mtd ? "hdd_mount" : "hdd_unmount"));    // our default
 		
    item->setPixmap(0,*icon);
    delete icon;
  }
  mountList->clearSelection();
  if (info) {
    info->setEmpty(true);
    info->repaint();
  }
  watcher->start( WATCHER_DELAY, true);   // starting the watch timer ( single shot )
}

void KMountManGUI::checkMountChange()
{
  if( krMtMan.checkMtabChanged() )
    krMtMan.forceUpdate();
  watcher->start( WATCHER_DELAY, true);   // starting the watch timer ( single shot )
}

void KMountManGUI::doubleClicked(QListViewItem *i) {
  if (!i) return;
  // change the active panel to this mountpoint
  fsData *system=krMtMan.location(i->text(0));
  connect((QObject*)this,SIGNAL(refreshPanel(const KURL &)), (QObject*)SLOTS,
          SLOT(refresh(const KURL &)));
  emit refreshPanel( vfs::fromPathOrURL( system->mntPoint() ) );
  disconnect(this,SIGNAL(refreshPanel(const KURL &)),0,0);
  slotClose();
}

// when user clicks on a filesystem, change information
void KMountManGUI::changeActive(QListViewItem *i) {
  if (!i) return;
  fsData *system=krMtMan.location(i->text(0));
  info->setAlias(system->mntPoint());
  info->setRealName(system->name());
  info->setMounted(system->mounted());
  info->setEmpty(false);
  info->setTotalSpace(system->totalBlks());
  info->setFreeSpace(system->freeBlks());
  info->setSupermount(system->supermount);
  info->repaint();
}

// called when right-clicked on a filesystem
void KMountManGUI::clicked(QListViewItem *item) {
  // these are the values that will exist in the menu
  #define MOUNT_ID       90
  #define UNMOUNT_ID     91
  #define FORMAT_ID      93
  #define EJECT_ID       94
  //////////////////////////////////////////////////////////
  if (!item) return;
  fsData *system=krMtMan.location(item->text(0));
  if (!system) {
    KMessageBox::error(0, i18n("MountMan has an internal error. Please notify the developers. Thank you."));
    exit(0);
  }
  // create the menu
  KPopupMenu popup;
  popup.insertTitle(i18n("MountMan"));
  if (!system->mounted())
    popup.insertItem(i18n("Mount"),MOUNT_ID);
  else popup.insertItem(i18n("Unmount"),UNMOUNT_ID);
  if (krMtMan.ejectable(system->mntPoint()))
//  if (system->type()=="iso9660" || krMtMan.followLink(system->name()).left(2)=="cd")
    popup.insertItem(i18n("Eject"),EJECT_ID);
  else {
    popup.insertItem(i18n("Format"),FORMAT_ID);
    popup.setItemEnabled(FORMAT_ID,false);
  }

  int result=popup.exec(QCursor::pos());
  // check out the user's option
  switch (result) {
    case -1 : return;     // the user clicked outside of the menu
    case MOUNT_ID   :
    case UNMOUNT_ID :
      krMtMan.toggleMount(system->name());
      break;
    case FORMAT_ID :
      break;
    case EJECT_ID :
      KMountMan::eject(system->mntPoint());
      break;
  }
}

#include "kmountmangui.moc"
