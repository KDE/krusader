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
#include "../KViewer/krviewer.h"
#include <klocale.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qfontmetrics.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <qcursor.h>
#include <qeventloop.h>
#include <kfinddialog.h>
#include <qregexp.h>
#include <qdir.h>

// these are the values that will exist in the menu
#define VIEW_ID             90
#define EDIT_ID             91
#define FIND_ID             92
#define FIND_NEXT_ID        93
#define FIND_PREV_ID        94
//////////////////////////////////////////////////////////

KProcess *  LocateDlg::updateProcess = 0;

LocateDlg::LocateDlg() : KDialogBase(0,0,false,"Locate", KDialogBase::User1 | KDialogBase::User2 | KDialogBase::User3 | KDialogBase::Close,
      KDialogBase::User3, false, i18n("Stop"), i18n("Update DB"), i18n("Locate") )
{
  QWidget *widget=new QWidget(this, "locateMainWidget");
  QGridLayout *grid = new QGridLayout( widget );
  grid->setSpacing( 6 );
  grid->setMargin( 11 );

  setPlainCaption( i18n( "Krusader::Locate" ) );
  
  QHBox *hbox = new QHBox( widget, "locateHBox" );
  new QLabel( i18n( "Search for:" ), hbox, "locateLabel" );
  locateSearchFor = new KHistoryCombo( false, hbox, "locateSearchFor" );
  krConfig->setGroup("Locate");
  QStringList list = krConfig->readListEntry("Search For");
  locateSearchFor->setMaxCount(25);  // remember 25 items
  locateSearchFor->setHistoryItems(list);
  locateSearchFor->setEditable( true );
  locateSearchFor->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
  locateSearchFor->lineEdit()->setFocus();

  grid->addWidget( hbox, 0, 0 );

  QHBox *hbox2 = new QHBox( widget, "locateHBox" );
  QSpacerItem* spacer = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  hbox2->layout()->addItem( spacer );
  dontSearchInPath = new QCheckBox( i18n( "Don't search in path" ), hbox2, "dontSearchInPath" );
  dontSearchInPath->setChecked( krConfig->readBoolEntry("Dont Search In Path") );
  existingFiles = new QCheckBox( i18n( "Show only the existing files" ), hbox2, "existingFiles" );
  existingFiles->setChecked( krConfig->readBoolEntry("Existing Files") );
  caseSensitive = new QCheckBox( i18n( "Case Sensitive" ), hbox2, "caseSensitive" );
  caseSensitive->setChecked( krConfig->readBoolEntry("Case Sensitive") );
  grid->addWidget( hbox2, 1, 0 );

  QFrame *line1 = new QFrame( widget, "locateLine1" );
  line1->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  grid->addWidget( line1, 2, 0 );

  resultList=new KListView( widget );  // create the main container

  krConfig->setGroup("Look&Feel");
  resultList->setFont(krConfig->readFontEntry("Filelist Font",_FilelistFont));

  resultList->setAllColumnsShowFocus(true);
  resultList->setVScrollBarMode(QScrollView::Auto);
  resultList->setHScrollBarMode(QScrollView::Auto);
  resultList->setShowSortIndicator(false);
  resultList->setSorting(-1);

  resultList->addColumn( i18n("Results"), QFontMetrics(resultList->font()).width("W") * 60 );
  resultList->setColumnWidthMode(0,QListView::Maximum);

  connect( resultList,SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
           this, SLOT(slotRightClick(QListViewItem *)));
  connect( resultList,SIGNAL(doubleClicked(QListViewItem *)),
           this, SLOT(slotDoubleClick(QListViewItem *)));
  connect( resultList,SIGNAL(returnPressed(QListViewItem *)),
           this, SLOT(slotDoubleClick(QListViewItem *)));
           
  grid->addWidget( resultList, 3, 0 );

  QFrame *line2 = new QFrame( widget, "locateLine2" );
  line2->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  grid->addWidget( line2, 4, 0 );

  enableButton( KDialogBase::User1, false );  /* disable the stop button */

  if( updateProcess )
  {
    if( updateProcess->isRunning() )
    {
      connect( updateProcess, SIGNAL(processExited(KProcess *)), this, SLOT(updateFinished()));
      enableButton( KDialogBase::User2, false );
    }
    else
      updateFinished();
  }

  setMainWidget(widget);
  show();

  while( isShown() )
  {
    qApp->processEvents();
    qApp->eventLoop()->processEvents( QEventLoop::AllEvents|QEventLoop::WaitForMore);
  }
}

void LocateDlg::slotUser1()   /* The stop button */
{
  stopping = true;
}

void LocateDlg::slotUser2()   /* The Update DB button */
{
  if( !updateProcess )
  {
    krConfig->setGroup("Locate");

    updateProcess = new KProcess();
    *updateProcess << KrServices::fullPathName( "updatedb" );
    *updateProcess << KrServices::separateArgs( krConfig->readEntry( "UpdateDB Arguments", "" ) );
    
    connect( updateProcess, SIGNAL(processExited(KProcess *)), this, SLOT(updateFinished()));
    updateProcess->start(KProcess::NotifyOnExit);
    enableButton( KDialogBase::User2, false );
  }
}

void LocateDlg::updateFinished()
{
  delete updateProcess;
  updateProcess = 0;
  enableButton( KDialogBase::User2, true );
}

void LocateDlg::slotUser3()   /* The locate button */
{
  locateSearchFor->addToHistory(locateSearchFor->currentText());
  QStringList list = locateSearchFor->historyItems();
  krConfig->setGroup("Locate");
  krConfig->writeEntry("Search For", list);
  krConfig->writeEntry("Dont Search In Path", dontSearchPath = dontSearchInPath->isChecked() );
  krConfig->writeEntry("Existing Files", onlyExist = existingFiles->isChecked() );
  krConfig->writeEntry("Case Sensitive", isCs = caseSensitive->isChecked() );

  if( !KrServices::cmdExist( "locate" ) )
  {
    KMessageBox::error(0,
      i18n("Can't start 'locate'! Check the 'Dependencies' page in konfigurator."));
    return;
  }
  
  resultList->clear();
  lastItem = 0;
  remaining = "";

  enableButton( KDialogBase::User3, false );  /* disable the locate button */
  enableButton( KDialogBase::User1, true );   /* enable the stop button */

  qApp->processEvents();

  stopping = false;
  
  KProcess locateProc;
  connect( &locateProc, SIGNAL( receivedStdout(KProcess *, char *, int) ),
            this, SLOT( processStdout(KProcess *, char *, int) ) );
  connect( &locateProc, SIGNAL( receivedStderr(KProcess *, char *, int) ),
            this, SLOT( processStderr(KProcess *, char *, int) ) );

  locateProc << KrServices::fullPathName( "locate" );
  if( !isCs )
    locateProc << "-i";
  locateProc << (pattern = locateSearchFor->currentText());
  
  if( !pattern.startsWith( "*" ) )
    pattern = "*" + pattern;
  if( !pattern.endsWith( "*" ) )
    pattern = pattern + "*";
  
  collectedErr = "";
  bool result = !locateProc.start( KProcess::Block, KProcess::AllOutput );
  if( !collectedErr.isEmpty() && ( !locateProc.normalExit() || locateProc.exitStatus() ) )
  {
     KMessageBox::error( krApp, i18n( "Locate produced the following error message:\n\n" ) + collectedErr );
  }else if ( result )
  {
     KMessageBox::error( krApp, i18n( "Error during the start of `locate` process!" ) );
  }
  enableButton( KDialogBase::User3, true );  /* enable the locate button */
  enableButton( KDialogBase::User1, false ); /* disable the stop button */
}

void LocateDlg::processStdout(KProcess *proc, char *buffer, int length)
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
        QRegExp regExp( pattern, isCs, true );
        QString fileName = (*it).stripWhiteSpace();
        if( fileName.endsWith( "/" ) && fileName != "/" )
          fileName.truncate( fileName.length() -1 );
        fileName = fileName.mid( fileName.findRev( '/' ) + 1 );
        
        if( !regExp.exactMatch( fileName ) )
          continue;        
      }
      if( onlyExist )
      {
        KFileItem file(KFileItem::Unknown, KFileItem::Unknown, (*it).stripWhiteSpace() );
        if( !file.isReadable() )
          continue;
      }
      
      if( lastItem )    
        lastItem = new KListViewItem( resultList, lastItem, *it );
      else
        lastItem = new KListViewItem( resultList, *it );
    }
  }

  if( stopping )
    proc->kill( SIGKILL );
  
  qApp->processEvents();
}

void LocateDlg::processStderr(KProcess *, char *buffer, int length)
{
  char *buf = new char[ length+1 ];
  memcpy( buf, buffer, length );
  buf[ length ] = 0;

  collectedErr += QString::fromLocal8Bit( buf );
  delete []buf;  
}

void LocateDlg::slotRightClick(QListViewItem *item)
{
  if ( !item )
    return;

  // create the menu
  KPopupMenu popup;
  popup.insertTitle(i18n("Locate"));

  popup.insertItem(i18n("View (F3)"), VIEW_ID);
  popup.insertItem(i18n("Edit (F4)"), EDIT_ID);

  popup.insertSeparator();

  popup.insertItem(i18n("Find (Ctrl+F)"), FIND_ID);
  popup.insertItem(i18n("Find next (Ctrl+N)"), FIND_NEXT_ID);
  popup.insertItem(i18n("Find previous (Ctrl+P)"), FIND_PREV_ID);

  int result=popup.exec(QCursor::pos());

  // check out the user's option
  switch (result)
  {
  case VIEW_ID:
  case EDIT_ID:
  case FIND_ID:
  case FIND_NEXT_ID:
  case FIND_PREV_ID:
    operate( item, result );
    break;
  }
}

void LocateDlg::slotDoubleClick(QListViewItem *item)
{
  if ( !item )
    return;

  QString dirName = item->text(0);
  if( !QDir( dirName ).exists() )
    dirName.truncate( dirName.findRev( '/' ) );
    
  ACTIVE_FUNC->delayedOpenUrl(vfs::fromPathOrURL( dirName ) );
  KDialogBase::accept();
}

void LocateDlg::keyPressEvent( QKeyEvent *e )
{
  switch ( e->key() )
  {
  case Key_F3 :
    if( resultList->currentItem() )
      operate( resultList->currentItem(), VIEW_ID );
    break;
  case Key_F4 :
    if( resultList->currentItem() )
      operate( resultList->currentItem(), EDIT_ID );
    break;
  case Key_N :
    if ( e->state() == ControlButton )
      operate( resultList->currentItem(), FIND_NEXT_ID );
    break;
  case Key_P :
    if ( e->state() == ControlButton )
      operate( resultList->currentItem(), FIND_PREV_ID );
    break;
  case Key_F :
    if ( e->state() == ControlButton )
      operate( resultList->currentItem(), FIND_ID );
    break;
  }

  QDialog::keyPressEvent( e );
}

void LocateDlg::operate( QListViewItem *item, int task )
{
  KURL name = vfs::fromPathOrURL( item->text( 0 ) );
  
  switch ( task )
  {
  case VIEW_ID:
    KrViewer::view( name ); // view the file
    break;
  case EDIT_ID:
    KrViewer::edit( name ); // view the file
    break;
  case FIND_ID:
    {
      krConfig->setGroup("Locate");
      long options = krConfig->readNumEntry("Find Options", 0);
      QStringList list = krConfig->readListEntry("Find Patterns");
      
      KFindDialog dlg( this, "locateFindDialog", options, list );
      if ( dlg.exec() != QDialog::Accepted )
        return;

      if( list.first() != ( findPattern = dlg.pattern() ) )
        list.push_front( dlg.pattern() );
        
      krConfig->writeEntry( "Find Options", findOptions = dlg.options() );
      krConfig->writeEntry( "Find Patterns", list );

      if( !( findOptions & KFindDialog::FromCursor ) )
        resultList->setCurrentItem( ( findOptions & KFindDialog::FindBackwards ) ?
                                    resultList->lastItem() : resultList->firstChild() );

      findCurrentItem = (KListViewItem *)resultList->currentItem();
      
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
        findOptions ^= KFindDialog::FindBackwards;
      
      findCurrentItem = (KListViewItem *)resultList->currentItem();
      nextLine();

      if( find() && findCurrentItem )
        resultList->setCurrentItem( findCurrentItem );
      else
        KMessageBox::information( 0, i18n( "Search string not found!" ) );

      resultList->ensureItemVisible( resultList->currentItem() );

      if( task == FIND_PREV_ID )
        findOptions ^= KFindDialog::FindBackwards;
    }
    break;
  }
}

void LocateDlg::nextLine()
{
  if( findOptions & KFindDialog::FindBackwards )
    findCurrentItem = (KListViewItem *)findCurrentItem->itemAbove();
  else
    findCurrentItem = (KListViewItem *)findCurrentItem->itemBelow();
}

bool LocateDlg::find()
{
  while( findCurrentItem )
  {
    QString item = findCurrentItem->text( 0 );

    if( findOptions & KFindDialog::RegularExpression )
    {
      if( item.contains( QRegExp( findPattern, findOptions & KFindDialog::CaseSensitive ) ) )
        return true;
    }
    else
    {
      if( item.contains( findPattern, findOptions & KFindDialog::CaseSensitive ) )
        return true;
    }
    
    nextLine();
  }
  
  return false;
}

#include "locate.moc"
