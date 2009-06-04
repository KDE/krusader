/***************************************************************************
                         KgProtocols.cpp  -  description
                             -------------------
    copyright            : (C) 2004 by Csaba Karai
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

#include "kgprotocols.h"
#include "../krusader.h"
#include "../krservices.h"
#include <klocale.h>
#include <kprotocolinfo.h>
#include <kmimetype.h>
#include <qheaderview.h>
#include <QGridLayout>
#include <QVBoxLayout>
#include <kiconloader.h>

QString KgProtocols::defaultProtocols  = "krarc,iso,tar";
QString KgProtocols::defaultIsoMimes   = "application/x-iso,application/x-cd-image,"
                                         "application/x-dvd-image";
QString KgProtocols::defaultKrarcMimes = "application/x-7z,application/x-7z-compressed,"
                                         "application/x-ace,application/x-ace-compressed,"
                                         "application/x-arj,application/x-arj-compressed,"
                                         "application/x-bzip2,"
                                         "application/x-cpio,application/x-deb,"
                                         "application/x-debian-package,"
                                         "application/x-gzip,application/x-jar,"
                                         "application/x-lha,application/x-lha-compressed,"
                                         "application/x-rar,application/x-rar-compressed,"
                                         "application/x-rpm,application/zip,"
                                         "application/x-zip,application/x-zip-compressed";
QString KgProtocols::defaultTarMimes   = "application/x-tar,application/x-tarz,"
                                         "application/x-bzip-compressed-tar,"
                                         "application/x-compressed-tar,"
                                         "application/x-tbz,application/x-tgz";

KgProtocols::KgProtocols( bool first, QWidget* parent ) :
      KonfiguratorPage( first, parent )
{
  QGridLayout *KgProtocolsLayout = new QGridLayout( this );
  KgProtocolsLayout->setSpacing( 6 );

  //  -------------------------- LINK VIEW ----------------------------------
  
  QGroupBox *linkGrp = createFrame( i18n( "Links" ), this );    
  QGridLayout *linkGrid = createGridLayout( linkGrp );
  
  QStringList labels;
  labels << i18n( "Defined Links" );

  linkList = new KrTreeWidget( linkGrp );
  linkList->setHeaderLabels( labels );
  linkList->setRootIsDecorated( true );
  
  linkGrid->addWidget( linkList, 0, 0 );
  KgProtocolsLayout->addWidget( linkGrp, 0, 0, 2, 1 );

  //  -------------------------- BUTTONS ----------------------------------

  QWidget *vbox1Widget = new QWidget( this );
  QVBoxLayout *vbox1 = new QVBoxLayout( vbox1Widget );
  
  addSpacer( vbox1 );
  btnAddProtocol = new QPushButton( vbox1Widget );
  btnAddProtocol->setIcon( krLoader->loadIcon( "arrow-left", KIconLoader::Small ) );
  btnAddProtocol->setWhatsThis( i18n( "Add protocol to the link list." ) );
  vbox1->addWidget( btnAddProtocol );

  btnRemoveProtocol = new QPushButton( vbox1Widget );
  btnRemoveProtocol->setIcon( krLoader->loadIcon( "arrow-right", KIconLoader::Small ) );
  btnRemoveProtocol->setWhatsThis( i18n( "Remove protocol from the link list." ) );
  vbox1->addWidget( btnRemoveProtocol );
  addSpacer( vbox1 );
  
  KgProtocolsLayout->addWidget( vbox1Widget, 0 ,1 );

  QWidget *vbox2Widget = new QWidget( this );
  QVBoxLayout *vbox2 = new QVBoxLayout( vbox2Widget );
  
  addSpacer( vbox2 );
  btnAddMime = new QPushButton( vbox2Widget );
  btnAddMime->setIcon( krLoader->loadIcon( "arrow-left", KIconLoader::Small ) );
  btnAddMime->setWhatsThis( i18n( "Add mime to the selected protocol on the link list." ) );
  vbox2->addWidget( btnAddMime );

  btnRemoveMime = new QPushButton( vbox2Widget );
  btnRemoveMime->setIcon( krLoader->loadIcon( "arrow-right", KIconLoader::Small ) );
  btnRemoveMime->setWhatsThis( i18n( "Remove mime from the link list." ) );
  vbox2->addWidget( btnRemoveMime );
  addSpacer( vbox2 );
  
  KgProtocolsLayout->addWidget( vbox2Widget, 1 ,1 );
  
  //  -------------------------- PROTOCOLS LISTBOX ----------------------------------

  QGroupBox *protocolGrp = createFrame( i18n( "Protocols" ), this );    
  QGridLayout *protocolGrid = createGridLayout( protocolGrp );
  
  protocolList = new KrListWidget( protocolGrp );
  loadListCapableProtocols();
  protocolGrid->addWidget( protocolList, 0, 0 );

  KgProtocolsLayout->addWidget( protocolGrp, 0 ,2 );

  //  -------------------------- MIMES LISTBOX ----------------------------------

  QGroupBox *mimeGrp = createFrame( i18n( "Mimes" ), this );    
  QGridLayout *mimeGrid = createGridLayout( mimeGrp );
  
  mimeList = new KrListWidget( mimeGrp );
  loadMimes();
  mimeGrid->addWidget( mimeList, 0, 0 );

  KgProtocolsLayout->addWidget( mimeGrp, 1 ,2 );
  
  //  -------------------------- CONNECT TABLE ----------------------------------  
  
  connect( protocolList,      SIGNAL( itemSelectionChanged() ), this, SLOT( slotDisableButtons() ) );
  connect( linkList,          SIGNAL( itemSelectionChanged() ), this, SLOT( slotDisableButtons() ) );
  connect( mimeList,          SIGNAL( itemSelectionChanged() ), this, SLOT( slotDisableButtons() ) );
  connect( linkList,          SIGNAL( currentItemChanged ( QTreeWidgetItem *, QTreeWidgetItem * ) ), this, SLOT( slotDisableButtons() ) );
  connect( btnAddProtocol,    SIGNAL( clicked() )         , this, SLOT( slotAddProtocol() ) );
  connect( btnRemoveProtocol, SIGNAL( clicked() )         , this, SLOT( slotRemoveProtocol() ) );
  connect( btnAddMime,        SIGNAL( clicked() )         , this, SLOT( slotAddMime() ) );
  connect( btnRemoveMime,     SIGNAL( clicked() )         , this, SLOT( slotRemoveMime() ) );
  
  loadInitialValues();
  slotDisableButtons();
}
  
void KgProtocols::addSpacer( QBoxLayout *layout )
{
  layout->addItem( new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding ) );
}

void KgProtocols::loadListCapableProtocols()
{
  QStringList protocols = KProtocolInfo::protocols();
  protocols.sort();
  
  for ( QStringList::Iterator it = protocols.begin(); it != protocols.end();) 
  {
//    if( !KProtocolInfo::supportsListing( *it ) )    // TODO TODO
//    {
//      it = protocols.remove( it );
//      continue;
//    }
    ++it;
  }
  protocolList->addItems( protocols );
}

void KgProtocols::loadMimes()
{
  KMimeType::List mimes = KMimeType::allMimeTypes();
  
  for( KMimeType::List::const_iterator it = mimes.constBegin(); it != mimes.constEnd(); it++ )
    mimeList->addItem( (*it)->name() );
    
  mimeList->sortItems();
}

void KgProtocols::slotDisableButtons()
{
  btnAddProtocol->setEnabled( protocolList->selectedItems().count() != 0 );
  QTreeWidgetItem *listViewItem = linkList->currentItem();
  bool isProtocolSelected = ( listViewItem == 0 ? false : listViewItem->parent() == 0 );
  btnRemoveProtocol->setEnabled( isProtocolSelected );
  btnAddMime->setEnabled( listViewItem != 0 && mimeList->selectedItems().count() != 0 );
  btnRemoveMime->setEnabled( listViewItem == 0 ? false : listViewItem->parent() != 0 );
  
  if( linkList->currentItem() == 0 && linkList->topLevelItemCount() != 0 )
    linkList->setCurrentItem( linkList->topLevelItem( 0 ) );

  QList<QTreeWidgetItem *> list = linkList->selectedItems();
  if( list.count() == 0 && linkList->currentItem() != 0 )
    linkList->currentItem()->setSelected( true );
}

void KgProtocols::slotAddProtocol()
{
  QList<QListWidgetItem *> list = protocolList->selectedItems ();

  if( list.count() > 0 )
  {
    addProtocol( list[ 0 ]->text(), true );
    slotDisableButtons();
    emit sigChanged();
  }
}

void KgProtocols::addProtocol( QString name, bool changeCurrent )
{
  QList<QListWidgetItem *> list = protocolList->findItems( name, Qt::MatchExactly );
  if( list.count() > 0 )
  {
    delete list[ 0 ];
    QTreeWidgetItem *listViewItem = new QTreeWidgetItem( linkList );
    listViewItem->setText( 0, name );
    listViewItem->setIcon( 0, krLoader->loadIcon( "exec", KIconLoader::Small ) );
    
    if( changeCurrent )
      linkList->setCurrentItem( listViewItem );
  }
}

void KgProtocols::slotRemoveProtocol()
{
  QTreeWidgetItem *item = linkList->currentItem();
  if( item )
  {
    removeProtocol( item->text( 0 ) );
    slotDisableButtons();
    emit sigChanged();
  }
}

void KgProtocols::removeProtocol( QString name )
{
  QList<QTreeWidgetItem *> itemList = linkList->findItems ( name, Qt::MatchExactly, 0 );

  if( itemList.count() )
  {
    QTreeWidgetItem *item = itemList[ 0 ];

    while( item->childCount() != 0 )
      removeMime( item->child( 0 )->text( 0 ) );
     
    linkList->takeTopLevelItem( linkList->indexOfTopLevelItem( item ) );
    protocolList->addItem( name );
    protocolList->sortItems();
  }
}

void KgProtocols::slotAddMime()
{
  QList<QListWidgetItem *> list = mimeList->selectedItems();
  if( list.count() > 0 && linkList->currentItem() != 0 )
  {
    QTreeWidgetItem *itemToAdd = linkList->currentItem();
    if( itemToAdd->parent() )
      itemToAdd = itemToAdd->parent();
      
    addMime( list[ 0 ]->text(), itemToAdd->text( 0 ) );
    slotDisableButtons();
    emit sigChanged();
  }
}

void KgProtocols::addMime( QString name, QString protocol )
{
  QList<QListWidgetItem *> list = mimeList->findItems( name, Qt::MatchExactly );

  QList<QTreeWidgetItem *> itemList = linkList->findItems ( protocol, Qt::MatchExactly | Qt::MatchRecursive, 0 );

  QTreeWidgetItem *currentListItem = 0;
  if( itemList.count() != 0 )
     currentListItem = itemList[ 0 ];
  
  if( list.count() > 0 && currentListItem && currentListItem->parent() == 0 )
  {
    delete list[ 0 ];
    QTreeWidgetItem *listViewItem = new QTreeWidgetItem( currentListItem );
    listViewItem->setText( 0, name );
    listViewItem->setIcon( 0, krLoader->loadIcon( "mime", KIconLoader::Small ) );
    // FIXME The following causes crash due to bug in QT 4.3.4 - 4.4.
    // reenable in the future, when the problem will be fixed.
    // linkList->expandItem( currentListItem );
  }
}

void KgProtocols::slotRemoveMime()
{
  QTreeWidgetItem *item = linkList->currentItem();
  if( item )
  {
    removeMime( item->text( 0 ) );
    slotDisableButtons();
    emit sigChanged();
  }
}

void KgProtocols::removeMime( QString name )
{
  QList<QTreeWidgetItem *> itemList = linkList->findItems ( name, Qt::MatchExactly | Qt::MatchRecursive, 0 );

  QTreeWidgetItem *currentMimeItem = 0;
  if( itemList.count() != 0 )
     currentMimeItem = itemList[ 0 ];
  
  if( currentMimeItem && currentMimeItem->parent() != 0 )
  {
    mimeList->addItem( currentMimeItem->text( 0 ) );
    mimeList->sortItems();
    currentMimeItem->parent()->removeChild( currentMimeItem );
  }
}

void KgProtocols::loadInitialValues()
{
  if(linkList->model()->rowCount() > 0)
    while( linkList->topLevelItemCount() != 0 )
      removeProtocol( linkList->topLevelItem( 0 )->text( 0 ) );
  
  KConfigGroup group( krConfig, "Protocols" );
  QStringList protList = group.readEntry( "Handled Protocols", QStringList() );
    
  for( QStringList::Iterator it = protList.begin(); it != protList.end(); it++ ) 
  {
    addProtocol( *it );
    
    QStringList mimes = group.readEntry( QString( "Mimes For %1" ).arg( *it ), QStringList() );
    
    for( QStringList::Iterator it2 = mimes.begin(); it2 != mimes.end(); it2++ )
      addMime( *it2, *it );
  }
  
  if( linkList->topLevelItemCount() != 0 )
    linkList->setCurrentItem( linkList->topLevelItem( 0 ) );
  slotDisableButtons();
  linkList->expandAll();
}

void KgProtocols::setDefaults()
{
  while( linkList->topLevelItemCount() != 0 )
    removeProtocol( linkList->topLevelItem( 0 )->text( 0 ) );
  
  addProtocol( "iso" );
  QStringList isoMimes = defaultIsoMimes.split( ',' );
  for( QStringList::Iterator it = isoMimes.begin(); it != isoMimes.end(); it++ )
    addMime( *it, "iso" );
  
  addProtocol( "krarc" );
  QStringList krarcMimes = defaultKrarcMimes.split( ',' );
  for( QStringList::Iterator it = krarcMimes.begin(); it != krarcMimes.end(); it++ )
    addMime( *it, "krarc" );
  
  addProtocol( "tar" );
  QStringList tarMimes = defaultTarMimes.split( ',' );
  for( QStringList::Iterator it = tarMimes.begin(); it != tarMimes.end(); it++ )
    addMime( *it, "tar" );
    
  slotDisableButtons();
    
  if( isChanged() )
    emit sigChanged();
}

bool KgProtocols::isChanged()
{
  KConfigGroup group( krConfig, "Protocols" );
  QStringList protList = group.readEntry( "Handled Protocols", QStringList() );
  
  if( (int)protList.count() != linkList->topLevelItemCount() )
    return true;
  
  for( int i = 0; i != linkList->topLevelItemCount(); i++ )
  {
    QTreeWidgetItem *item = linkList->topLevelItem( i );

    if( !protList.contains( item->text( 0 ) ) )
      return true;
      
    QStringList mimes = group.readEntry( QString( "Mimes For %1" ).arg( item->text( 0 ) ), QStringList() );
    
    if( (int)mimes.count() != item->childCount() )
      return true;

    for( int j=0; j != item->childCount(); j++ )
    {
      QTreeWidgetItem *children = item->child( j );

      if( !mimes.contains( children->text( 0 ) ) )
        return true;
    }
  }
    
  return false;
}

bool KgProtocols::apply()
{
  KConfigGroup group( krConfig, "Protocols" );
  
  QStringList protocolList;
  
  for( int i = 0; i != linkList->topLevelItemCount(); i++ )
  {
    QTreeWidgetItem *item = linkList->topLevelItem( i );

    protocolList.append( item->text( 0 ) );
    
    QStringList mimes;

    for( int j=0; j != item->childCount(); j++ )
    {
      QTreeWidgetItem *children = item->child( j );

      mimes.append( children->text( 0 ) );
    }
    group.writeEntry( QString( "Mimes For %1" ).arg( item->text( 0 ) ), mimes );
  }
  group.writeEntry( "Handled Protocols", protocolList );
  krConfig->sync();
  
  KrServices::clearProtocolCache();
  
  emit sigChanged();  
  return false;
}

void KgProtocols::init()
{
  if( !krConfig->groupList().contains( "Protocols" ) )
  {
    KConfigGroup group( krConfig, "Protocols" );
    group.writeEntry( "Handled Protocols", defaultProtocols );
    group.writeEntry( "Mimes For iso",     defaultIsoMimes );
    group.writeEntry( "Mimes For krarc",   defaultKrarcMimes );
    group.writeEntry( "Mimes For tar",     defaultTarMimes );
  }
}

#include "kgprotocols.moc"
