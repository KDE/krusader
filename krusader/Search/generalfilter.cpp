/***************************************************************************
                      generalfilter.cpp  -  description
                             -------------------
    copyright            : (C) 2003 by Shie Erlich & Rafi Yanai & Csaba Karai
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

#include "generalfilter.h"
#include "../krusader.h"
#include "../VFS/vfs.h"

#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kiconloader.h>
 
GeneralFilter::GeneralFilter( bool hasDirOptions, QWidget *parent, const char *name ) : QWidget( parent, name )
{
  QGridLayout *filterLayout = new QGridLayout( this );
  filterLayout->setSpacing( 6 );
  filterLayout->setMargin( 11 );

  this->hasDirOptions = hasDirOptions;
  
  // Options for name filtering
  
  QGroupBox *nameGroup = new QGroupBox( this, "nameGroup" );
  nameGroup->setTitle( i18n( "File name:" ) );
  nameGroup->setColumnLayout(0, Qt::Vertical );
  nameGroup->layout()->setSpacing( 0 );
  nameGroup->layout()->setMargin( 0 );
  QGridLayout *nameGroupLayout = new QGridLayout( nameGroup->layout() );
  nameGroupLayout->setAlignment( Qt::AlignTop );
  nameGroupLayout->setSpacing( 6 );
  nameGroupLayout->setMargin( 11 );
  
  searchForCase = new QCheckBox( nameGroup, "searchForCase" );
  searchForCase->setText( i18n( "&Case sensitive" ) );
  searchForCase->setChecked( false );  
  nameGroupLayout->addWidget( searchForCase, 1, 2 );
    
  QLabel *searchForLabel = new QLabel( nameGroup, "searchForLabel" );
  searchForLabel->setText( i18n( "Search &for:" ) );
  nameGroupLayout->addWidget( searchForLabel, 0, 0 );
    
  searchFor = new KHistoryCombo( false, nameGroup, "searchFor" );
  searchFor->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, searchFor->sizePolicy().hasHeightForWidth() ) );
  searchFor->setEditable( true );
  searchForLabel->setBuddy( searchFor );    
  nameGroupLayout->addMultiCellWidget( searchFor, 0, 0, 1, 2 );
    
  QLabel *searchType = new QLabel( nameGroup, "searchType" );
  searchType->setText( i18n( "&Of type:" ) );
  nameGroupLayout->addWidget( searchType, 1, 0 );
    
  ofType = new KComboBox( false, nameGroup, "ofType" );
  ofType->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, ofType->sizePolicy().hasHeightForWidth() ) );
  ofType->setEditable( false );
  searchType->setBuddy(ofType);
  ofType->insertItem(i18n("All Files"));
  ofType->insertItem(i18n("Archives"));
  ofType->insertItem(i18n("Directories"));
  ofType->insertItem(i18n("Image Files"));
  ofType->insertItem(i18n("Text Files"));
  ofType->insertItem(i18n("Video Files"));
  ofType->insertItem(i18n("Audio Files"));
    
  nameGroupLayout->addWidget( ofType, 1, 1 );
  filterLayout->addMultiCellWidget( nameGroup, 0, 0, 0, 1 );

  if( hasDirOptions )
  {
    KIconLoader *iconLoader = new KIconLoader();
    QPixmap imageAdd = iconLoader->loadIcon( "1downarrow", KIcon::Panel, 16 );
    QPixmap imageFolder = iconLoader->loadIcon( "folder", KIcon::Panel, 16 );
    
    // Options for search in

    QGroupBox *searchInGroup = new QGroupBox( this, "searchInGroup" );
    searchInGroup->setTitle( i18n( "&Search in" ) );
    searchInGroup->setColumnLayout(0, Qt::Vertical );
    searchInGroup->layout()->setSpacing( 0 );
    searchInGroup->layout()->setMargin( 0 );
    QGridLayout *searchInLayout = new QGridLayout( searchInGroup->layout() );
    searchInLayout->setAlignment( Qt::AlignTop );
    searchInLayout->setSpacing( 6 );
    searchInLayout->setMargin( 11 );
    
    searchInEdit = new KLineEdit( searchInGroup, "searchInEdit" );
    searchInLayout->addWidget( searchInEdit, 0, 0 );
      
    searchIn = new QListBox( searchInGroup, "searchIn" );
    searchIn->setSelectionMode( QListBox::Extended );
    searchInLayout->addMultiCellWidget( searchIn, 1, 1, 0, 2 );

    searchInBtnAdd = new QToolButton( searchInGroup, "searchInBtnAdd" );
    searchInBtnAdd->setText( "" );
    searchInBtnAdd->setPixmap( imageAdd );
    searchInLayout->addWidget( searchInBtnAdd, 0, 1 );
    
    searchInBtn = new QToolButton( searchInGroup, "searchInBtn" );
    searchInBtn->setText( "" );
    searchInBtn->setPixmap( imageFolder );
    searchInLayout->addWidget( searchInBtn, 0, 2 );

    filterLayout->addWidget( searchInGroup, 1, 0 );
  
    // Options for don't search in

    QGroupBox *dontSearchInGroup = new QGroupBox( this, "dontSearchInGroup" );
    dontSearchInGroup->setTitle( i18n( "&Don't search in" ) );
    dontSearchInGroup->setColumnLayout(0, Qt::Vertical );
    dontSearchInGroup->layout()->setSpacing( 0 );
    dontSearchInGroup->layout()->setMargin( 0 );
    QGridLayout *dontSearchInLayout = new QGridLayout( dontSearchInGroup->layout() );
    dontSearchInLayout->setAlignment( Qt::AlignTop );
    dontSearchInLayout->setSpacing( 6 );
    dontSearchInLayout->setMargin( 11 );

    dontSearchInEdit = new KLineEdit( dontSearchInGroup, "dontSearchInEdit" );
    dontSearchInLayout->addWidget( dontSearchInEdit, 0, 0 );

    dontSearchInBtnAdd = new QToolButton( dontSearchInGroup, "dontSearchInBtnAdd" );
    dontSearchInBtnAdd->setText( "" );
    dontSearchInBtnAdd->setPixmap( imageAdd );
    dontSearchInLayout->addWidget( dontSearchInBtnAdd, 0, 1 );
  
    dontSearchInBtn = new QToolButton( dontSearchInGroup, "dontSearchInBtn" );
    dontSearchInBtn->setText( "" );
    dontSearchInBtn->setPixmap( imageFolder );
    dontSearchInLayout->addWidget( dontSearchInBtn, 0, 2 );

    dontSearchIn = new QListBox( dontSearchInGroup, "dontSearchIn" );
    dontSearchIn->setSelectionMode( QListBox::Extended );
    dontSearchInLayout->addMultiCellWidget( dontSearchIn, 1, 1, 0, 2 );
      
    filterLayout->addWidget( dontSearchInGroup, 1, 1 );
  
    // add shell completion
  
    completion.setMode( KURLCompletion::FileCompletion );
    searchInEdit->setCompletionObject( &completion );
    dontSearchInEdit->setCompletionObject( &completion );
  }
  
  // Options for containing text
 
  QGroupBox *containsGroup = new QGroupBox( this, "containsGroup" );
  containsGroup->setTitle( i18n( "Containing text" ) );
  containsGroup->setColumnLayout(0, Qt::Vertical );
  containsGroup->layout()->setSpacing( 0 );
  containsGroup->layout()->setMargin( 0 );
  QGridLayout *containsLayout = new QGridLayout( containsGroup->layout() );
  containsLayout->setAlignment( Qt::AlignTop );
  containsLayout->setSpacing( 6 );
  containsLayout->setMargin( 11 );

  QHBoxLayout *containsTextLayout = new QHBoxLayout();
  containsTextLayout->setSpacing( 6 );
  containsTextLayout->setMargin( 0 );

  QLabel *containsLabel = new QLabel( containsGroup, "containsLabel" );
  containsLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)1, containsLabel->sizePolicy().hasHeightForWidth() ) );
  containsLabel->setText( i18n( "&Text:" ) );
  containsTextLayout->addWidget( containsLabel );

  containsText = new KHistoryCombo( false, containsGroup, "containsText" );
  containsText->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, containsText->sizePolicy().hasHeightForWidth() ) );
  containsTextLayout->addWidget( containsText );
  containsLabel->setBuddy(containsText);

  containsLayout->addLayout( containsTextLayout, 0, 0 );

  QHBoxLayout *containsCbsLayout = new QHBoxLayout();
  containsCbsLayout->setSpacing( 6 );
  containsCbsLayout->setMargin( 0 );
  QSpacerItem* cbSpacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  containsCbsLayout->addItem( cbSpacer );

  containsWholeWord = new QCheckBox( containsGroup, "containsWholeWord" );
  containsWholeWord->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, containsWholeWord->sizePolicy().hasHeightForWidth() ) );
  containsWholeWord->setText( i18n( "&Match whole word only" ) );
  containsWholeWord->setChecked( false );
  containsCbsLayout->addWidget( containsWholeWord );

  containsTextCase = new QCheckBox( containsGroup, "containsTextCase" );
  containsTextCase->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, containsTextCase->sizePolicy().hasHeightForWidth() ) );
  containsTextCase->setText( i18n( "Cas&e sensitive" ) );
  containsTextCase->setChecked( true );
  containsCbsLayout->addWidget( containsTextCase );

  containsLayout->addLayout( containsCbsLayout, 1, 0 );
  
  int position = hasDirOptions ?  2 : 1;
  filterLayout->addMultiCellWidget( containsGroup, position, position, 0, 1 );  
  
  if( hasDirOptions )
  {
    // Options for recursive searching
    
    QHBoxLayout *recurseLayout = new QHBoxLayout();
    recurseLayout->setSpacing( 6 );
    recurseLayout->setMargin( 0 );
    QSpacerItem* recurseSpacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    recurseLayout->addItem( recurseSpacer );

    searchInDirs = new QCheckBox( this, "searchInDirs" );
    searchInDirs->setText( i18n( "Search in s&ubdirectories" ) );
    searchInDirs->setChecked( true );
    recurseLayout->addWidget( searchInDirs );

    searchInArchives = new QCheckBox( this, "searchInArchives" );
    searchInArchives->setText( i18n( "Search in arch&ives" ) );
    recurseLayout->addWidget( searchInArchives );

    followLinks = new QCheckBox( this, "followLinks" );
    followLinks->setText( i18n( "Follow &links" ) );
    recurseLayout->addWidget( followLinks );

    filterLayout->addMultiCellLayout( recurseLayout, 3, 3, 0, 1 ); 
  }
  
  // Connection table
  
  if( hasDirOptions )
  {  
    connect( searchInBtn, SIGNAL( clicked() ), this, SLOT( addToSearchIn() ) );
    connect( searchInBtnAdd, SIGNAL( clicked() ), this, SLOT( addToSearchInManually() ) );
    connect( searchInEdit, SIGNAL( returnPressed(const QString&) ), this, SLOT( addToSearchInManually() ) );
    connect( dontSearchInBtn, SIGNAL( clicked() ), this, SLOT( addToDontSearchIn() ) );
    connect( dontSearchInBtnAdd, SIGNAL( clicked() ), this, SLOT( addToDontSearchInManually() ) );  
    connect( dontSearchInEdit, SIGNAL( returnPressed(const QString&) ), this, SLOT( addToDontSearchInManually() ) );
    connect( searchInArchives, SIGNAL(toggled(bool)), containsText, SLOT(setDisabled(bool)));
    connect( searchInArchives, SIGNAL(toggled(bool)), containsTextCase, SLOT(setDisabled(bool)));
    connect( searchInArchives, SIGNAL(toggled(bool)), containsWholeWord, SLOT(setDisabled(bool)));
  }
  connect( searchFor, SIGNAL(activated(const QString&)), searchFor, SLOT(addToHistory(const QString&)));
  connect( containsText, SIGNAL(activated(const QString&)), containsText, SLOT(addToHistory(const QString&)));
    
  // tab order
    
  if( hasDirOptions )
  {
    setTabOrder( searchFor, ofType );
    setTabOrder( ofType, searchInEdit );
    setTabOrder( searchInEdit, dontSearchInEdit );
    setTabOrder( dontSearchInEdit, containsText );
    setTabOrder( containsText, dontSearchIn );
    setTabOrder( dontSearchIn, searchIn );
    setTabOrder( searchIn, containsTextCase );
    setTabOrder( containsTextCase, searchForCase );
    setTabOrder( searchForCase, searchInDirs );
    setTabOrder( searchInDirs, searchInArchives );
    setTabOrder( searchInArchives, followLinks );
  }
  
  // load the completion and history lists
  // ==> search for
  krConfig->setGroup("Search");
  QStringList list = krConfig->readListEntry("SearchFor Completion");
  searchFor->completionObject()->setItems(list);
  list = krConfig->readListEntry("SearchFor History");
  searchFor->setHistoryItems(list);
  // ==> grep
  krConfig->setGroup("Search");
  list = krConfig->readListEntry("ContainsText Completion");
  containsText->completionObject()->setItems(list);
  list = krConfig->readListEntry("ContainsText History");
  containsText->setHistoryItems(list);
}

GeneralFilter::~GeneralFilter()
{
  // save the history combos
  // ==> search for
  QStringList list = searchFor->completionObject()->items();
  krConfig->setGroup("Search");
  krConfig->writeEntry("SearchFor Completion", list);
  list = searchFor->historyItems();
  krConfig->writeEntry("SearchFor History", list);
  // ==> grep text
  list = containsText->completionObject()->items();
  krConfig->setGroup("Search");
  krConfig->writeEntry("ContainsText Completion", list);
  list = containsText->historyItems();
  krConfig->writeEntry("ContainsText History", list);

  krConfig->sync();
}

void GeneralFilter::addToSearchIn() 
{
  KURL url = KFileDialog::getExistingURL();
  if ( url.isEmpty()) return;
  searchInEdit->setText( url.prettyURL( 0,KURL::StripFileProtocol ) );
  searchInEdit->setFocus();
}

void GeneralFilter::addToSearchInManually() 
{
  if( searchInEdit->text().simplifyWhiteSpace().length() )
  {  
    searchIn->insertItem(searchInEdit->text());
    searchInEdit->clear();
  }
}

void GeneralFilter::addToDontSearchIn() 
{
  KURL url = KFileDialog::getExistingURL();
  if ( url.isEmpty()) return;
  dontSearchInEdit->setText( url.prettyURL( 0,KURL::StripFileProtocol ) );
  dontSearchInEdit->setFocus();
}

void GeneralFilter::addToDontSearchInManually() 
{
  if( dontSearchInEdit->text().simplifyWhiteSpace().length() )
  {
    dontSearchIn->insertItem(dontSearchInEdit->text());
    dontSearchInEdit->clear();
  }
}
  
bool GeneralFilter::fillQuery( KRQuery *query )
{
  // check that we have (at least) what to search, and where to search in
  if (searchFor->currentText().simplifyWhiteSpace().isEmpty()) {
    KMessageBox::error(0,i18n("No search criteria entered !"));
    searchFor->setFocus();
    return false;
  }
  
  // now fill the query
    
  query->setFilter( searchFor->currentText().stripWhiteSpace() );    
  query->matchesCaseSensitive = searchForCase->isChecked();
  if (containsText->isEnabled())
    query->contain = containsText->currentText();
  query->containCaseSensetive = containsTextCase->isChecked();
  query->containWholeWord     = containsWholeWord->isChecked();
  if (ofType->currentText()!=i18n("All Files"))
    query->type = ofType->currentText();
  else query->type = QString::null;

  if ( hasDirOptions )
  {
    query->inArchive = searchInArchives->isChecked();
    query->recurse = searchInDirs->isChecked();
    query->followLinks = followLinks->isChecked();
    
    // create the lists
  
    query->whereToSearch.clear();
    QListBoxItem *item = searchIn->firstItem();
    while ( item )
    {    
      query->whereToSearch.append( vfs::fromPathOrURL( item->text().simplifyWhiteSpace() ) );
      item = item->next();
    }

    if (!searchInEdit->text().simplifyWhiteSpace().isEmpty())
      query->whereToSearch.append( vfs::fromPathOrURL( searchInEdit->text().simplifyWhiteSpace() ) );

    query->whereNotToSearch.clear();
    item = dontSearchIn->firstItem();
    while ( item )
    {
      query->whereNotToSearch.append( vfs::fromPathOrURL( item->text().simplifyWhiteSpace() ) );
      item = item->next();
    }
    if (!dontSearchInEdit->text().simplifyWhiteSpace().isEmpty())
      query->whereNotToSearch.append( vfs::fromPathOrURL( dontSearchInEdit->text().simplifyWhiteSpace() ) );

    // checking the lists
      
    if (query->whereToSearch.isEmpty() ) { // we need a place to search in
      KMessageBox::error(0,i18n("Please specify a location to search in."));
      searchInEdit->setFocus();
      return false;
    }
  }

  return true;
}

void GeneralFilter::queryAccepted()
{
  searchFor->addToHistory(searchFor->currentText());
  containsText->addToHistory(containsText->currentText());
}

void GeneralFilter::deleteSelectedItems( QListBox *list_box )
{
  int i=0;
  QListBoxItem *item;

  while( (item = list_box->item(i)) )
  {
    if( item->isSelected() )
    {
      list_box->removeItem( i );
      continue;
    }
    i++;
  }
}

void GeneralFilter::keyPressEvent(QKeyEvent *e)
{
  if( hasDirOptions && e->key() == Key_Delete )
  {
    if( searchIn->hasFocus() )
    {
      deleteSelectedItems( searchIn );
      return;
    }
    if( dontSearchIn->hasFocus() )
    {
      deleteSelectedItems( dontSearchIn );
      return;
    }
  }

  QWidget::keyPressEvent( e );
}

void GeneralFilter::loadFromProfile( QString name )
{
  krConfig->setGroup( name );

  searchForCase->setChecked( krConfig->readBoolEntry( "Case Sensitive Search", false ) );
  containsTextCase->setChecked( krConfig->readBoolEntry( "Case Sensitive Content", false ) );
  containsWholeWord->setChecked( krConfig->readBoolEntry( "Match Whole Word Only", false ) );
  searchFor->setEditText( krConfig->readEntry( "Search For", "" ) );
  containsText->setEditText( krConfig->readEntry( "Contains Text", "" ) );
  
  QString mime = krConfig->readEntry( "Mime Type", "" );
  for( int i = ofType->count(); i >= 0; i-- )
  {
    ofType->setCurrentItem( i );
    if( ofType->currentText() == mime )
      break;
  }

  if( hasDirOptions )
  {
    searchInDirs->setChecked( krConfig->readBoolEntry( "Search In Subdirectories", true ) );
    searchInArchives->setChecked( krConfig->readBoolEntry( "Search In Archives", false ) );
    followLinks->setChecked( krConfig->readBoolEntry( "Follow Symlinks", false ) );

    searchInEdit->setText( krConfig->readEntry( "Search In Edit", "" ) );
    dontSearchInEdit->setText( krConfig->readEntry( "Dont Search In Edit", "" ) );

    searchIn->clear();
    QStringList searchInList = krConfig->readListEntry( "Search In List" );
    if( !searchInList.isEmpty() )
      searchIn->insertStringList( searchInList );

    dontSearchIn->clear();
    QStringList dontSearchInList = krConfig->readListEntry( "Dont Search In List" );
    if( !dontSearchInList.isEmpty() )
      dontSearchIn->insertStringList( dontSearchInList );
  }
}

void GeneralFilter::saveToProfile( QString name )
{
  krConfig->setGroup( name );
  
  krConfig->writeEntry( "Case Sensitive Search", searchForCase->isChecked() );
  krConfig->writeEntry( "Case Sensitive Content", containsTextCase->isChecked() );
  krConfig->writeEntry( "Match Whole Word Only", containsWholeWord->isChecked() );
  krConfig->writeEntry( "Search For", searchFor->currentText() );  
  krConfig->writeEntry( "Contains Text", containsText->currentText() );  
  
  krConfig->writeEntry( "Mime Type", ofType->currentText() );

  if( hasDirOptions )
  {  
    krConfig->writeEntry( "Search In Subdirectories", searchInDirs->isChecked() );
    krConfig->writeEntry( "Search In Archives", searchInArchives->isChecked() );  
    krConfig->writeEntry( "Follow Symlinks", followLinks->isChecked() );

    krConfig->writeEntry( "Search In Edit", searchInEdit->text() );
    krConfig->writeEntry( "Dont Search In Edit", dontSearchInEdit->text() );
  
    QStringList searchInList;
    QListBoxItem *item;
    for ( item = searchIn->firstItem(); item != 0; item = item->next() )
      searchInList.append( item->text().simplifyWhiteSpace() );
    krConfig->writeEntry( "Search In List", searchInList );
  
    QStringList dontSearchInList;
    for ( item = dontSearchIn->firstItem(); item != 0; item = item->next() )
      dontSearchInList.append( item->text().simplifyWhiteSpace() );
    krConfig->writeEntry( "Dont Search In List", dontSearchInList );
  }
}
  
#include "generalfilter.moc"
