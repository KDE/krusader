/***************************************************************************
                    kurllistrequester.cpp  -  description
                             -------------------
    copyright            : (C) 2005 by Csaba Karai
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

#include "kurllistrequester.h"
#include "../VFS/vfs.h"
#include <qpixmap.h>
#include <qcursor.h>
#include <qlayout.h>
#include <kfiledialog.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <klocale.h>

#define DELETE_ITEM_ID    100

KURLListRequester::KURLListRequester( QWidget *parent, const char * name ) : QWidget( parent, name )
{
  KIconLoader *iconLoader = new KIconLoader();
  QPixmap imageAdd = iconLoader->loadIcon( "1downarrow", KIcon::Panel, 16 );
  QPixmap imageFolder = iconLoader->loadIcon( "folder", KIcon::Panel, 16 );
    
  // Creating the widget
  
  QGridLayout *urlListRequesterGrid = new QGridLayout( this );
  urlListRequesterGrid->setSpacing( 0 );
  urlListRequesterGrid->setMargin( 0 );
    
  urlLineEdit = new KLineEdit( this, "urlLineEdit" );
  urlListRequesterGrid->addWidget( urlLineEdit, 0, 0 );
      
  urlListBox = new QListBox( this, "urlListBox" );
  urlListBox->setSelectionMode( QListBox::Extended );
  urlListRequesterGrid->addMultiCellWidget( urlListBox, 1, 1, 0, 2 );

  urlAddBtn = new QToolButton( this, "urlAddBtn" );
  urlAddBtn->setText( "" );
  urlAddBtn->setPixmap( imageAdd );
  urlListRequesterGrid->addWidget( urlAddBtn, 0, 1 );
    
  urlBrowseBtn = new QToolButton( this, "urlBrowseBtn" );
  urlBrowseBtn->setText( "" );
  urlBrowseBtn->setPixmap( imageFolder );
  urlListRequesterGrid->addWidget( urlBrowseBtn, 0, 2 );

  // add shell completion
    
  completion.setMode( KURLCompletion::FileCompletion );
  urlLineEdit->setCompletionObject( &completion );
  
  // connection table
  
  connect( urlAddBtn, SIGNAL( clicked() ), this, SLOT( slotAdd() ) );
  connect( urlBrowseBtn, SIGNAL( clicked() ), this, SLOT( slotBrowse() ) );
  connect( urlLineEdit, SIGNAL( returnPressed(const QString&) ), this, SLOT( slotAdd() ) );
  connect( urlListBox, SIGNAL( rightButtonClicked ( QListBoxItem *, const QPoint & ) ), this,
                       SLOT( slotRightClicked( QListBoxItem * ) ) );
}

void KURLListRequester::slotAdd()
{
  QString text = urlLineEdit->text().simplifyWhiteSpace();
  if( text.length() )
  {  
    urlListBox->insertItem( urlLineEdit->text() );
    urlLineEdit->clear();
  }
}

void KURLListRequester::slotBrowse()
{
  KURL url = KFileDialog::getExistingURL( QString::null, this );
  if( !url.isEmpty())
    urlLineEdit->setText( url.prettyURL( 0,KURL::StripFileProtocol ) );
  urlLineEdit->setFocus();
}

void KURLListRequester::keyPressEvent(QKeyEvent *e)
{
  if( e->key() == Key_Delete )
  {
    if( urlListBox->hasFocus() )
    {
      deleteSelectedItems();
      return;
    }
  }

  QWidget::keyPressEvent( e );
}

void KURLListRequester::deleteSelectedItems()
{
  int i=0;
  QListBoxItem *item;

  while( (item = urlListBox->item(i)) )
  {
    if( item->isSelected() )
    {
      urlListBox->removeItem( i );
      continue;
    }
    i++;
  }
}

void KURLListRequester::slotRightClicked( QListBoxItem *item )
{
  if( item == 0 )
    return;
    
  KPopupMenu popupMenu( this );
  popupMenu.insertItem( i18n( "Delete" ), DELETE_ITEM_ID );
  
  switch( popupMenu.exec( QCursor::pos() ) )
  {
  case DELETE_ITEM_ID:
    if( item->isSelected() )
      deleteSelectedItems();
    else
      urlListBox->removeItem( urlListBox->index( item ) );
    break;
  }
}

KURL::List KURLListRequester::urlList()
{
  KURL::List urls;
  
  if (!urlLineEdit->text().simplifyWhiteSpace().isEmpty())
    urls.append( vfs::fromPathOrURL( urlLineEdit->text().simplifyWhiteSpace() ) );
    
  QListBoxItem *item = urlListBox->firstItem();
  while ( item )
  {    
    urls.append( vfs::fromPathOrURL( item->text().simplifyWhiteSpace() ) );
    item = item->next();
  }
    
  return urls;
}

void KURLListRequester::setUrlList( KURL::List urlList )
{
  urlLineEdit->clear();
  urlListBox->clear();

  KURL::List::iterator it;
    
  for ( it = urlList.begin(); it != urlList.end(); ++it )
    urlListBox->insertItem( (*it).prettyURL( 0,KURL::StripFileProtocol ) );
}

#include "kurllistrequester.moc"
