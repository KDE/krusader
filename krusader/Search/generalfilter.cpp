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

static const char* const image1_data[] = {
"16 16 52 1",
". c None",
"# c #000000",
"Q c #101010",
"X c #181c18",
"K c #202020",
"w c #313031",
"W c #313431",
"V c #414041",
"U c #414441",
"a c #4a3018",
"E c #4a484a",
"T c #4a4c4a",
"l c #524c4a",
"h c #52504a",
"j c #525052",
"i c #5a5952",
"J c #6a6962",
"P c #6a6d6a",
"S c #8b8d8b",
"D c #949594",
"R c #9c999c",
"O c #9c9d9c",
"C c #a4a19c",
"M c #a4a1a4",
"N c #a4a5a4",
"I c #acaaac",
"B c #acaeac",
"F c #b47d41",
"v c #b4b2b4",
"u c #b4b6b4",
"A c #bdbabd",
"t c #bdbebd",
"k c #c5854a",
"H c #c5c2c5",
"s c #c5c6c5",
"r c #cdcacd",
"L c #cdcecd",
"z c #d5d2d5",
"q c #d5d6d5",
"x c #dedade",
"p c #dedede",
"b c #e6a562",
"G c #e6e2e6",
"o c #e6e6e6",
"y c #eeeae6",
"n c #eeeeee",
"m c #f6f6f6",
"f c #ffae62",
"e c #ffc283",
"d c #ffc683",
"g c #ffca8b",
"c c #ffd29c",
"......####......",
".....##.####.#..",
"....#.....####..",
"...........###..",
"..........####..",
".aaaa...........",
"abcdbaaaaaaa....",
"acdeffffffff#...",
"ageahhhihhjjh###",
"aeklmmnopqrstuvw",
"aeaxpyopzsABCDE.",
"aFhxxGpqrHuICJK.",
"aaLrqqzLHABMDE..",
"ajLtssHtuBNOPQ..",
"atNIBBINMRDSE...",
"KEETTTEEUVVWX..."};

static const char * const image2_data[] = {
"16 16 17 1",
" 	c None",
".	c #704A13",
"+	c #8A6E3C",
"@	c #997E52",
"#	c #2C1D06",
"$	c #EAD9B9",
"%	c #E0CBA7",
"&	c #C8A15E",
"*	c #DAB579",
"=	c #F8F5F0",
"-	c #BF9346",
";	c #AB6503",
">	c #C1892B",
",	c #B6710C",
"'	c #965702",
")	c #5A3202",
"!	c #0D0702",
"                ",
"     .+@@@+#    ",
"    +$$$%$$&#   ",
"   .$%**%**$&#  ",
"  .$*&*$=***%&# ",
" .%*&**==**&&%-#",
" .-&&&*$=%&&&-+#",
" #;>$$$==$$$*,.#",
" #;>========$,.#",
" #;,-&*$=*&-,,.#",
" #.;,,-%=&>,,')#",
" !)';,,%=>,,')# ",
"  !).;,&%>;;)#  ",
"   !).;;;;;)#   ",
"    !#).)))#    ",
"     ######     "};

 
GeneralFilter::GeneralFilter( QWidget *parent, const char *name ) : QWidget( parent, name )
{
  QGridLayout *filterLayout = new QGridLayout( this );
  filterLayout->setSpacing( 6 );
  filterLayout->setMargin( 11 );

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
  searchInBtnAdd->setPixmap( QPixmap( ( const char** ) image2_data ) );
  searchInLayout->addWidget( searchInBtnAdd, 0, 1 );
    
  searchInBtn = new QToolButton( searchInGroup, "searchInBtn" );
  searchInBtn->setText( "" );
  searchInBtn->setPixmap( QPixmap( ( const char** ) image1_data ) );
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
  dontSearchInBtnAdd->setPixmap( QPixmap( ( const char** ) image2_data ) );
  dontSearchInLayout->addWidget( dontSearchInBtnAdd, 0, 1 );
  
  dontSearchInBtn = new QToolButton( dontSearchInGroup, "dontSearchInBtn" );
  dontSearchInBtn->setText( "" );
  dontSearchInBtn->setPixmap( QPixmap( ( const char** ) image1_data ) );
  dontSearchInLayout->addWidget( dontSearchInBtn, 0, 2 );

  dontSearchIn = new QListBox( dontSearchInGroup, "dontSearchIn" );
  dontSearchIn->setSelectionMode( QListBox::Extended );
  dontSearchInLayout->addMultiCellWidget( dontSearchIn, 1, 1, 0, 2 );
      
  filterLayout->addWidget( dontSearchInGroup, 1, 1 );
  
  // add shell completion
  
  completion.setMode( KURLCompletion::FileCompletion );
  searchInEdit->setCompletionObject( &completion );
  dontSearchInEdit->setCompletionObject( &completion );
  
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
  
  filterLayout->addMultiCellWidget( containsGroup, 2, 2, 0, 1 );  
  
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
  
  // Connection table
    
  connect( searchInBtn, SIGNAL( clicked() ), this, SLOT( addToSearchIn() ) );
  connect( searchInBtnAdd, SIGNAL( clicked() ), this, SLOT( addToSearchInManually() ) );
  connect( searchInEdit, SIGNAL( returnPressed(const QString&) ), this, SLOT( addToSearchInManually() ) );
  connect( dontSearchInBtn, SIGNAL( clicked() ), this, SLOT( addToDontSearchIn() ) );
  connect( dontSearchInBtnAdd, SIGNAL( clicked() ), this, SLOT( addToDontSearchInManually() ) );  
  connect( dontSearchInEdit, SIGNAL( returnPressed(const QString&) ), this, SLOT( addToDontSearchInManually() ) );
  connect( searchInArchives, SIGNAL(toggled(bool)), containsText, SLOT(setDisabled(bool)));
  connect( searchInArchives, SIGNAL(toggled(bool)), containsTextCase, SLOT(setDisabled(bool)));
  connect( searchInArchives, SIGNAL(toggled(bool)), containsWholeWord, SLOT(setDisabled(bool)));
  connect( searchFor, SIGNAL(activated(const QString&)), searchFor, SLOT(addToHistory(const QString&)));
  connect( containsText, SIGNAL(activated(const QString&)), containsText, SLOT(addToHistory(const QString&)));
    
  // tab order
    
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
  query->inArchive = searchInArchives->isChecked();
  query->recurse = searchInDirs->isChecked();
  query->followLinks = followLinks->isChecked();
  if (ofType->currentText()!=i18n("All Files"))
    query->type = ofType->currentText();
  else query->type = QString::null;

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
  if( e->key() == Key_Delete )
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

#include "generalfilter.moc"
