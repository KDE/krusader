/***************************************************************************
                         locate.cpp  -  description
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

#include "locate.h"
#include "../krusader.h"
#include "../krusaderview.h"
#include "../Panel/listpanel.h"
#include "../Panel/panelfunc.h"
#include "../defaults.h"
#include "../krservices.h"
#include "../VFS/vfs.h"
#include "../VFS/virt_vfs.h"
#include "../KViewer/krviewer.h"
#include "../panelmanager.h"
#include "../kicons.h"
#include <klocale.h>
#include <q3hbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qfontmetrics.h>
//Added by qt3to4:
#include <QKeyEvent>
#include <Q3GridLayout>
#include <QFrame>
#include <kmessagebox.h>
#include <kmenu.h>
#include <qcursor.h>
#include <qeventloop.h>
#include <kfinddialog.h>
#include <kfind.h>
#include <kinputdialog.h>
#include <qregexp.h>
#include <qdir.h>
#include <qclipboard.h>
#include <k3urldrag.h>
#include <qfont.h>

// these are the values that will exist in the menu
#define VIEW_ID                     90
#define EDIT_ID                     91
#define FIND_ID                     92
#define FIND_NEXT_ID                93
#define FIND_PREV_ID                94
#define COPY_SELECTED_TO_CLIPBOARD  95
//////////////////////////////////////////////////////////

class LocateListView : public K3ListView
{
public:
  LocateListView( QWidget * parent ) : K3ListView( parent )
  {
  }

  void startDrag()
  {
    KUrl::List urls;

    Q3ListViewItem * item = firstChild();
    while( item )
    {
      if( item->isSelected() )
         urls.push_back( KUrl( item->text( 0 ) ) );

      item = item->nextSibling();
    }

    if( urls.count() == 0 )
      return;

    K3URLDrag *d = new K3URLDrag(urls, this);
    d->setPixmap( FL_LOADICON( "file" ), QPoint( -7, 0 ) );
    d->dragCopy();
  }
};

K3Process *  LocateDlg::updateProcess = 0;
LocateDlg * LocateDlg::LocateDialog = 0;

LocateDlg::LocateDlg() : KDialog( 0 ), isFeedToListBox( false )
{
  setButtons( KDialog::User1 | KDialog::User2 | KDialog::User3 | KDialog::Close );
  setDefaultButton( KDialog::User3 );
  setCaption( i18n("Locate") );
  setWindowModality( Qt::NonModal );
  setButtonGuiItem( KDialog::User1, KGuiItem( i18n("Stop") ) );
  setButtonGuiItem( KDialog::User2, KGuiItem( i18n("Update DB") ) );
  setButtonGuiItem( KDialog::User3, KGuiItem( i18n("Locate") ) );

  QWidget *widget=new QWidget(this, "locateMainWidget");
  Q3GridLayout *grid = new Q3GridLayout( widget );
  grid->setSpacing( 6 );
  grid->setMargin( 11 );

  setPlainCaption( i18n( "Krusader::Locate" ) );
  
  Q3HBox *hbox = new Q3HBox( widget, "locateHBox" );
  QLabel *label = new QLabel( i18n( "Search for:" ), hbox, "locateLabel" );
  locateSearchFor = new KHistoryComboBox( false, hbox );
  label->setBuddy( locateSearchFor );
  KConfigGroup group( krConfig, "Locate");
  QStringList list = group.readEntry("Search For", QStringList());
  locateSearchFor->setMaxCount(25);  // remember 25 items
  locateSearchFor->setHistoryItems(list);
  locateSearchFor->setEditable( true );
  locateSearchFor->setDuplicatesEnabled( false );
  locateSearchFor->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
  locateSearchFor->lineEdit()->setFocus();

  grid->addWidget( hbox, 0, 0 );

  Q3HBox *hbox2 = new Q3HBox( widget, "locateHBox" );
  QSpacerItem* spacer = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  hbox2->layout()->addItem( spacer );
  dontSearchInPath = new QCheckBox( i18n( "Don't search in path" ), hbox2, "dontSearchInPath" );
  dontSearchInPath->setChecked( group.readEntry("Dont Search In Path", false) );
  existingFiles = new QCheckBox( i18n( "Show only the existing files" ), hbox2, "existingFiles" );
  existingFiles->setChecked( group.readEntry("Existing Files", false) );
  caseSensitive = new QCheckBox( i18n( "Case Sensitive" ), hbox2, "caseSensitive" );
  caseSensitive->setChecked( group.readEntry("Case Sensitive", false) );
  grid->addWidget( hbox2, 1, 0 );

  QFrame *line1 = new QFrame( widget );
  line1->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  grid->addWidget( line1, 2, 0 );

  resultList=new LocateListView( widget );  // create the main container

  KConfigGroup gl( krConfig, "Look&Feel");
  resultList->setFont(gl.readEntry("Filelist Font",*_FilelistFont));

  resultList->setAllColumnsShowFocus(true);
  resultList->setVScrollBarMode(Q3ScrollView::Auto);
  resultList->setHScrollBarMode(Q3ScrollView::Auto);
  resultList->setShowSortIndicator(false);
  resultList->setSorting(-1);
  resultList->setSelectionMode( Q3ListView::Extended );

  resultList->addColumn( i18n("Results"), QFontMetrics(resultList->font()).width("W") * 60 );
  resultList->setColumnWidthMode(0,Q3ListView::Maximum);

  connect( resultList,SIGNAL(rightButtonPressed(Q3ListViewItem *, const QPoint &, int)),
           this, SLOT(slotRightClick(Q3ListViewItem *)));
  connect( resultList,SIGNAL(doubleClicked(Q3ListViewItem *)),
           this, SLOT(slotDoubleClick(Q3ListViewItem *)));
  connect( resultList,SIGNAL(returnPressed(Q3ListViewItem *)),
           this, SLOT(slotDoubleClick(Q3ListViewItem *)));
  connect( this, SIGNAL( user1Clicked() ), this, SLOT( slotUser1() ) );
  connect( this, SIGNAL( user2Clicked() ), this, SLOT( slotUser2() ) );
  connect( this, SIGNAL( user3Clicked() ), this, SLOT( slotUser3() ) );
           
  grid->addWidget( resultList, 3, 0 );

  QFrame *line2 = new QFrame( widget );
  line2->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  grid->addWidget( line2, 4, 0 );

  enableButton( KDialog::User1, false );  /* disable the stop button */

  if( updateProcess )
  {
    if( updateProcess->isRunning() )
    {
      connect( updateProcess, SIGNAL(processExited(K3Process *)), this, SLOT(updateFinished()));
      enableButton( KDialog::User2, false );
    }
    else
      updateFinished();
  }

  setMainWidget(widget);
  show();

  LocateDialog = this;
}

void LocateDlg::slotUser1()   /* The stop / feed to listbox button */
{
  if( isFeedToListBox )
    feedToListBox();
  else
    stopping = true;
}

void LocateDlg::slotUser2()   /* The Update DB button */
{
  if( !updateProcess )
  {
    KConfigGroup group( krConfig, "Locate");

    updateProcess = new K3Process();
    *updateProcess << KrServices::fullPathName( "updatedb" );
    *updateProcess << KrServices::separateArgs( group.readEntry( "UpdateDB Arguments" ) );
    
    connect( updateProcess, SIGNAL(processExited(K3Process *)), this, SLOT(updateFinished()));
    updateProcess->start(K3Process::NotifyOnExit);
    enableButton( KDialog::User2, false );
  }
}

void LocateDlg::updateFinished()
{
  delete updateProcess;
  updateProcess = 0;
  enableButton( KDialog::User2, true );
}

void LocateDlg::slotUser3()   /* The locate button */
{
  locateSearchFor->addToHistory(locateSearchFor->currentText());
  QStringList list = locateSearchFor->historyItems();
  KConfigGroup group( krConfig, "Locate");
  group.writeEntry("Search For", list);
  group.writeEntry("Dont Search In Path", dontSearchPath = dontSearchInPath->isChecked() );
  group.writeEntry("Existing Files", onlyExist = existingFiles->isChecked() );
  group.writeEntry("Case Sensitive", isCs = caseSensitive->isChecked() );

  if( !KrServices::cmdExist( "locate" ) )
  {
    KMessageBox::error(0,
      i18n("Can't start 'locate'! Check the 'Dependencies' page in konfigurator."));
    return;
  }
  
  resultList->clear();
  lastItem = 0;
  remaining = "";

  enableButton( KDialog::User3, false );  /* disable the locate button */
  enableButton( KDialog::User1, true );   /* enable the stop button */
  setButtonText( KDialog::User1, i18n( "Stop" ) ); /* the button behaves as stop */
  isFeedToListBox = false;
  resultList->setFocus();

  qApp->processEvents();

  stopping = false;
  
  K3Process locateProc;
  connect( &locateProc, SIGNAL( receivedStdout(K3Process *, char *, int) ),
            this, SLOT( processStdout(K3Process *, char *, int) ) );
  connect( &locateProc, SIGNAL( receivedStderr(K3Process *, char *, int) ),
            this, SLOT( processStderr(K3Process *, char *, int) ) );

  locateProc << KrServices::fullPathName( "locate" );
  if( !isCs )
    locateProc << "-i";
  locateProc << (pattern = locateSearchFor->currentText());
  
  if( !pattern.startsWith( "*" ) )
    pattern = "*" + pattern;
  if( !pattern.endsWith( "*" ) )
    pattern = pattern + "*";
  
  collectedErr = "";
  bool result = !locateProc.start( K3Process::Block, K3Process::AllOutput );
  if( !collectedErr.isEmpty() && ( !locateProc.normalExit() || locateProc.exitStatus() ) )
  {
     KMessageBox::error( krApp, i18n( "Locate produced the following error message:\n\n" ) + collectedErr );
  }else if ( result )
  {
     KMessageBox::error( krApp, i18n( "Error during the start of 'locate' process!" ) );
  }
  enableButton( KDialog::User3, true );  /* enable the locate button */
  
  if( resultList->childCount() == 0 )
  {
    locateSearchFor->setFocus();
    enableButton( KDialog::User1, false ); /* disable the stop button */
    isFeedToListBox = false;
  }else{
    setButtonText( KDialog::User1, i18n("Feed to listbox") ); /* feed to listbox */
    isFeedToListBox = true;
  }
}

void LocateDlg::processStdout(K3Process *proc, char *buffer, int length)
{
  char *buf = new char[ length+1 ];
  memcpy( buf, buffer, length );
  buf[ length ] = 0;

  remaining += QString::fromLocal8Bit( buf );
  delete []buf;

  QStringList list = QStringList::split("\n", remaining );
  int items = list.size();

  for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
  {
    if( --items == 0 && buffer[length-1] != '\n' )
      remaining = *it;
    else
    {
      if( dontSearchPath )
      {
        QRegExp regExp( pattern, isCs, QRegExp::Wildcard );
        QString fileName = (*it).trimmed();
        if( fileName.endsWith( "/" ) && fileName != "/" )
          fileName.truncate( fileName.length() -1 );
        fileName = fileName.mid( fileName.findRev( '/' ) + 1 );
        
        if( !regExp.exactMatch( fileName ) )
          continue;        
      }
      if( onlyExist )
      {
        KFileItem file(KFileItem::Unknown, KFileItem::Unknown, (*it).trimmed() );
        if( !file.isReadable() )
          continue;
      }
      
      if( lastItem )    
        lastItem = new K3ListViewItem( resultList, lastItem, *it );
      else
        lastItem = new K3ListViewItem( resultList, *it );

      lastItem->setDragEnabled( true );
    }
  }

  if( stopping )
    proc->kill( SIGKILL );
  
  qApp->processEvents();
}

void LocateDlg::processStderr(K3Process *, char *buffer, int length)
{
  char *buf = new char[ length+1 ];
  memcpy( buf, buffer, length );
  buf[ length ] = 0;

  collectedErr += QString::fromLocal8Bit( buf );
  delete []buf;  
}

void LocateDlg::slotRightClick(Q3ListViewItem *item)
{
  if ( !item )
    return;

  // create the menu
  KMenu popup;
  popup.setTitle(i18n("Locate"));

  QAction * actView = popup.addAction(i18n("View (F3)") );
  QAction * actEdit = popup.addAction(i18n("Edit (F4)") );
  popup.addSeparator();

  QAction * actFind = popup.addAction(i18n("Find (Ctrl+F)") );
  QAction * actNext = popup.addAction(i18n("Find next (Ctrl+N)") );
  QAction * actPrev = popup.addAction(i18n("Find previous (Ctrl+P)") );
  popup.addSeparator();

  QAction * actClip = popup.addAction(i18n("Copy selected to clipboard") );


  QAction * result = popup.exec(QCursor::pos());

  int ret = -1;

  if( result == actView )
    ret = VIEW_ID;
  else if( result == actEdit )
    ret = EDIT_ID;
  else if( result == actFind )
    ret = FIND_ID;
  else if( result == actNext )
    ret = FIND_NEXT_ID;
  else if( result == actPrev )
    ret = FIND_PREV_ID;
  else if( result == actClip )
    ret = COPY_SELECTED_TO_CLIPBOARD;

  if( ret != - 1 )
    operate( item, ret );
}

void LocateDlg::slotDoubleClick(Q3ListViewItem *item)
{
  if ( !item )
    return;

  QString dirName = item->text(0);
  QString fileName;

  if( !QDir( dirName ).exists() )
  {
    fileName = dirName.mid( dirName.findRev( '/' ) + 1 );
    dirName.truncate( dirName.findRev( '/' ) );
  }
    
  ACTIVE_FUNC->openUrl(KUrl( dirName ), fileName );
  KDialog::accept();
}

void LocateDlg::keyPressEvent( QKeyEvent *e )
{
  if( Krusader::actCopy->shortcut().contains( QKeySequence( e->key() ) ) )
  {
    operate( 0, COPY_SELECTED_TO_CLIPBOARD );
    e->accept();
    return;
  }

  switch ( e->key() )
  {
  case Qt::Key_M :
    if( e->modifiers() == Qt::ControlModifier )
    {
      resultList->setFocus();
      e->accept();
    }
    break;
  case Qt::Key_F3 :
    if( resultList->currentItem() )
      operate( resultList->currentItem(), VIEW_ID );
    break;
  case Qt::Key_F4 :
    if( resultList->currentItem() )
      operate( resultList->currentItem(), EDIT_ID );
    break;
  case Qt::Key_N :
    if ( e->modifiers() == Qt::ControlModifier )
      operate( resultList->currentItem(), FIND_NEXT_ID );
    break;
  case Qt::Key_P :
    if ( e->modifiers() == Qt::ControlModifier )
      operate( resultList->currentItem(), FIND_PREV_ID );
    break;
  case Qt::Key_F :
    if ( e->modifiers() == Qt::ControlModifier )
      operate( resultList->currentItem(), FIND_ID );
    break;
  }

  QDialog::keyPressEvent( e );
}

void LocateDlg::operate( Q3ListViewItem *item, int task )
{
  KUrl name;
  if( item != 0 )
    name = KUrl( item->text( 0 ) );
  
  switch ( task )
  {
  case VIEW_ID:
    KrViewer::view( name, this ); // view the file
    break;
  case EDIT_ID:
    KrViewer::edit( name, this ); // view the file
    break;
  case FIND_ID:
    {
      KConfigGroup group( krConfig, "Locate");
      long options = group.readEntry("Find Options", (long long)0);
      QStringList list = group.readEntry("Find Patterns", QStringList());
      
      KFindDialog dlg( this, options, list );
      if ( dlg.exec() != QDialog::Accepted )
        return;

      if( list.first() != ( findPattern = dlg.pattern() ) )
        list.push_front( dlg.pattern() );
        
      group.writeEntry( "Find Options", (long long)(findOptions = dlg.options() ) );
      group.writeEntry( "Find Patterns", list );

      if( !( findOptions & KFind::FromCursor ) )
        resultList->setCurrentItem( ( findOptions & KFind::FindBackwards ) ?
                                    resultList->lastItem() : resultList->firstChild() );

      findCurrentItem = (K3ListViewItem *)resultList->currentItem();
      
      if( find() && findCurrentItem )
        resultList->setCurrentItem( findCurrentItem );
      else
        KMessageBox::information( 0, i18n( "Search string not found!" ) );
        
      resultList->ensureItemVisible( resultList->currentItem() );
    }
    break;      
  case FIND_NEXT_ID:
  case FIND_PREV_ID:
    {
      if( task == FIND_PREV_ID )
        findOptions ^= KFind::FindBackwards;
      
      findCurrentItem = (K3ListViewItem *)resultList->currentItem();
      nextLine();

      if( find() && findCurrentItem )
        resultList->setCurrentItem( findCurrentItem );
      else
        KMessageBox::information( 0, i18n( "Search string not found!" ) );

      resultList->ensureItemVisible( resultList->currentItem() );

      if( task == FIND_PREV_ID )
        findOptions ^= KFind::FindBackwards;
    }
    break;
  case COPY_SELECTED_TO_CLIPBOARD:
    {
      KUrl::List urls;

      Q3ListViewItem * item = resultList->firstChild();
      while( item )
      {
        if( item->isSelected() )
           urls.push_back( KUrl( item->text( 0 ) ) );

        item = item->nextSibling();
      }

      if( urls.count() == 0 )
        return;

      K3URLDrag *d = new K3URLDrag(urls, this);
      d->setPixmap( FL_LOADICON( "file" ), QPoint( -7, 0 ) );
      QApplication::clipboard()->setData( d );
    }
    break;
  }
}

void LocateDlg::nextLine()
{
  if( findOptions & KFind::FindBackwards )
    findCurrentItem = (K3ListViewItem *)findCurrentItem->itemAbove();
  else
    findCurrentItem = (K3ListViewItem *)findCurrentItem->itemBelow();
}

bool LocateDlg::find()
{
  while( findCurrentItem )
  {
    QString item = findCurrentItem->text( 0 );

    if( findOptions & KFind::RegularExpression )
    {
      if( item.contains( QRegExp( findPattern, findOptions & KFind::CaseSensitive ) ) )
        return true;
    }
    else
    {
      if( item.contains( findPattern, findOptions & KFind::CaseSensitive ) )
        return true;
    }
    
    nextLine();
  }
  
  return false;
}

void LocateDlg::feedToListBox()
{
  virt_vfs v(0,true);
  v.vfs_refresh( KUrl( "/" ) );
  
  KConfigGroup group( krConfig, "Locate" );  
  int listBoxNum = group.readEntry( "Feed To Listbox Counter", 1 );  
  QString queryName;
  do {
    queryName = i18n("Locate results")+QString( " %1" ).arg( listBoxNum++ );
  }while( v.vfs_search( queryName ) != 0 );
  group.writeEntry( "Feed To Listbox Counter", listBoxNum );  
  
  KConfigGroup ga( krConfig, "Advanced" );
  if ( ga.readEntry( "Confirm Feed to Listbox",  _ConfirmFeedToListbox ) ) {
    bool ok;
    queryName = KInputDialog::getText(
                i18n("Query name"),		// Caption
                i18n("Here you can name the file collection"),	// Questiontext
                queryName,	// Default
                &ok );
     if ( ! ok)
       return;
  }
    
  KUrl::List urlList;
  Q3ListViewItem * item = resultList->firstChild();
  while( item )
  {
    urlList.push_back( KUrl( item->text( 0 ) ) );
    item = item->nextSibling();
  }
  KUrl url = KUrl(QString("virt:/")+ queryName);
  v.vfs_refresh( url );
  v.vfs_addFiles( &urlList, KIO::CopyJob::Copy, 0 );
  //ACTIVE_FUNC->openUrl(url);  
  ACTIVE_MNG->slotNewTab(url.prettyUrl());
  accept();
}

void LocateDlg::reset()
{
  locateSearchFor->lineEdit()->setFocus();
  locateSearchFor->lineEdit()->selectAll();
}

#include "locate.moc"
