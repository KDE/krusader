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
#include "../GUI/krtreewidget.h"
#include "../defaults.h"
#include "../krservices.h"
#include "../VFS/vfs.h"
#include "../VFS/virt_vfs.h"
#include "../KViewer/krviewer.h"
#include "../panelmanager.h"
#include "../kicons.h"
#include <klocale.h>
#include <kprocess.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qfontmetrics.h>
#include <qtreewidget.h>
#include <qheaderview.h>
#include <QKeyEvent>
#include <QGridLayout>
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
#include <QDrag>
#include <QMimeData>
#include <qfont.h>

// these are the values that will exist in the menu
#define VIEW_ID                     90
#define EDIT_ID                     91
#define FIND_ID                     92
#define FIND_NEXT_ID                93
#define FIND_PREV_ID                94
#define COPY_SELECTED_TO_CLIPBOARD  95
//////////////////////////////////////////////////////////

class LocateListView : public KrTreeWidget
{
public:
  LocateListView( QWidget * parent ) : KrTreeWidget( parent )
  {
    setAlternatingRowColors( true );
  }

  void startDrag( Qt::DropActions supportedActs )
  {
    KUrl::List urls;
    QList<QTreeWidgetItem *> list = selectedItems () ;

    QListIterator<QTreeWidgetItem *> it( list );
    
    while( it.hasNext() )
    {
      QTreeWidgetItem * item = it.next();

      urls.push_back( KUrl( item->text( 0 ) ) );
    }

    if( urls.count() == 0 )
      return;


    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    mimeData->setImageData( FL_LOADICON( "file" ) );
    urls.populateMimeData(mimeData);
    drag->setMimeData(mimeData);
    drag->start();
  }
};

KProcess *  LocateDlg::updateProcess = 0;
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

  QWidget *widget=new QWidget(this);
  QGridLayout *grid = new QGridLayout( widget );
  grid->setSpacing( 6 );
  grid->setContentsMargins( 11, 11, 11, 11 );

  setPlainCaption( i18n( "Krusader::Locate" ) );
  
  QWidget *hboxWidget = new QWidget( widget );
  QHBoxLayout *hbox = new QHBoxLayout( hboxWidget );
  hbox->setContentsMargins( 0, 0, 0, 0 );

  QLabel *label = new QLabel( i18n( "Search for:" ), hboxWidget );
  hbox->addWidget( label );

  locateSearchFor = new KHistoryComboBox( false, hboxWidget );
  hbox->addWidget( locateSearchFor );

  label->setBuddy( locateSearchFor );
  KConfigGroup group( krConfig, "Locate");
  QStringList list = group.readEntry("Search For", QStringList());
  locateSearchFor->setMaxCount(25);  // remember 25 items
  locateSearchFor->setHistoryItems(list);
  locateSearchFor->setEditable( true );
  locateSearchFor->setDuplicatesEnabled( false );
  locateSearchFor->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
  locateSearchFor->lineEdit()->setFocus();

  grid->addWidget( hboxWidget, 0, 0 );

  QWidget *hboxWidget2 = new QWidget( widget );
  QHBoxLayout * hbox2 = new QHBoxLayout( hboxWidget2 );
  hbox2->setContentsMargins( 0, 0, 0, 0 );

  QSpacerItem* spacer = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  hbox2->addItem( spacer );

  dontSearchInPath = new QCheckBox( i18n( "Don't search in path" ), hboxWidget2 );
  hbox2->addWidget( dontSearchInPath );
  dontSearchInPath->setChecked( group.readEntry("Dont Search In Path", false) );

  existingFiles = new QCheckBox( i18n( "Show only the existing files" ), hboxWidget2 );
  existingFiles->setChecked( group.readEntry("Existing Files", false) );
  hbox2->addWidget( existingFiles );

  caseSensitive = new QCheckBox( i18n( "Case Sensitive" ), hboxWidget2 );
  caseSensitive->setChecked( group.readEntry("Case Sensitive", false) );
  hbox2->addWidget( caseSensitive );

  grid->addWidget( hboxWidget2, 1, 0 );

  QFrame *line1 = new QFrame( widget );
  line1->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  grid->addWidget( line1, 2, 0 );

  resultList=new LocateListView( widget );  // create the main container
  resultList->setColumnCount( 1 );
  resultList->setHeaderLabel( i18n("Results") );

  resultList->setColumnWidth( 0, QFontMetrics(resultList->font()).width("W") * 60 );

  KConfigGroup gl( krConfig, "Look&Feel");
  resultList->setFont(gl.readEntry("Filelist Font",*_FilelistFont));

  resultList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  resultList->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  resultList->header()->setSortIndicatorShown(false);
  resultList->setSortingEnabled( false );
  resultList->setSelectionMode( QAbstractItemView::ExtendedSelection );
  resultList->setDragEnabled( true );

  connect( resultList,SIGNAL(itemRightClicked(QTreeWidgetItem *, const QPoint &, int)),
           this, SLOT(slotRightClick(QTreeWidgetItem *, const QPoint &)));
  connect( resultList,SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
           this, SLOT(slotDoubleClick(QTreeWidgetItem *)));
  connect( resultList,SIGNAL(itemActivated(QTreeWidgetItem *, int)),
           this, SLOT(slotDoubleClick(QTreeWidgetItem *)));
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
    if( updateProcess->state() == QProcess::Running )
    {
      connect( updateProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(updateFinished()));
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
    locateProc->kill();
}

void LocateDlg::slotUser2()   /* The Update DB button */
{
  if( !updateProcess )
  {
    KConfigGroup group( krConfig, "Locate");

    updateProcess = new KProcess(); // don't set the parent to 'this'! That would cause this process to be deleted once the dialog is closed
    *updateProcess << KrServices::fullPathName( "updatedb" );
    *updateProcess << KrServices::separateArgs( group.readEntry( "UpdateDB Arguments" ) );
    
    connect( updateProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(updateFinished()));
    updateProcess->start();
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

  locateProc = new KProcess(this);
  locateProc->setOutputChannelMode(KProcess::SeparateChannels); // default is forwarding to the parent channels
  connect( locateProc, SIGNAL( readyReadStandardOutput() ), SLOT( processStdout() ) );
  connect( locateProc, SIGNAL( readyReadStandardError() ), SLOT( processStderr() ) );
  connect( locateProc, SIGNAL( finished(int, QProcess::ExitStatus) ), SLOT( locateFinished() ) );
  connect( locateProc, SIGNAL( error(QProcess::ProcessError) ), SLOT( locateError() ) );

  *locateProc << KrServices::fullPathName( "locate" );
  if( !isCs )
    *locateProc << "-i";
  *locateProc << (pattern = locateSearchFor->currentText());
  
  if( !pattern.startsWith( "*" ) )
    pattern = "*" + pattern;
  if( !pattern.endsWith( "*" ) )
    pattern = pattern + "*";
  
  collectedErr = "";
  locateProc->start();
}

void LocateDlg::locateError()
{
  if( locateProc->error() == QProcess::FailedToStart )
    KMessageBox::error( krApp, i18n( "Error during the start of 'locate' process!" ) );
}

void LocateDlg::locateFinished()
{
  if( locateProc->exitStatus() != QProcess::NormalExit || locateProc->exitStatus() )
  {
    if( !collectedErr.isEmpty() )
      KMessageBox::error( krApp, i18n( "Locate produced the following error message:\n\n" ) + collectedErr );
  }
  enableButton( KDialog::User3, true );  /* enable the locate button */
  
  if( resultList->topLevelItemCount() == 0 )
  {
    locateSearchFor->setFocus();
    enableButton( KDialog::User1, false ); /* disable the stop button */
    isFeedToListBox = false;
  }else{
    setButtonText( KDialog::User1, i18n("Feed to listbox") ); /* feed to listbox */
    isFeedToListBox = true;
  }
}

void LocateDlg::processStdout()
{
  remaining += QString::fromLocal8Bit( locateProc->readAllStandardOutput() );

  QStringList list = QStringList::split("\n", remaining );
  int items = list.size();

  for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
  {
    if( --items == 0 && !remaining.endsWith('\n') )
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
        lastItem = new QTreeWidgetItem( resultList, lastItem );
      else
        lastItem = new QTreeWidgetItem( resultList );

      lastItem->setText( 0, *it );
    }
  }
}

void LocateDlg::processStderr()
{
  collectedErr += QString::fromLocal8Bit( locateProc->readAllStandardError() );
}

void LocateDlg::slotRightClick(QTreeWidgetItem *item, const QPoint &pos)
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


  QAction * result = popup.exec( pos );

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

void LocateDlg::slotDoubleClick(QTreeWidgetItem *item)
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
  if( Krusader::actCopy->shortcut().contains( QKeySequence( e->key() | e->modifiers() ) ) )
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

void LocateDlg::operate( QTreeWidgetItem *item, int task )
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

      QString first = QString::null;
      if( list.count() != 0 )
        first = list.first();
      if( first != ( findPattern = dlg.pattern() ) )
        list.push_front( dlg.pattern() );
        
      group.writeEntry( "Find Options", (long long)(findOptions = dlg.options() ) );
      group.writeEntry( "Find Patterns", list );

      if( !( findOptions & KFind::FromCursor ) && resultList->topLevelItemCount() )
        resultList->setCurrentItem( ( findOptions & KFind::FindBackwards ) ?
                                    resultList->topLevelItem( resultList->topLevelItemCount()-1 ) : resultList->topLevelItem( 0 ) );

      findCurrentItem = resultList->currentItem();
      
      if( find() && findCurrentItem )
      {
        resultList->selectionModel()->clearSelection(); // HACK: QT 4 is not able to paint the focus frame because of a bug
        resultList->setCurrentItem( findCurrentItem );
      }
      else
        KMessageBox::information( this, i18n( "Search string not found!" ) );
        
      resultList->scrollTo( resultList->currentIndex() );
    }
    break;      
  case FIND_NEXT_ID:
  case FIND_PREV_ID:
    {
      if( task == FIND_PREV_ID )
        findOptions ^= KFind::FindBackwards;
      
      findCurrentItem = resultList->currentItem();
      nextLine();

      if( find() && findCurrentItem )
      {
        resultList->selectionModel()->clearSelection(); // HACK: QT 4 is not able to paint the focus frame because of a bug
        resultList->setCurrentItem( findCurrentItem );
      }
      else
        KMessageBox::information( this, i18n( "Search string not found!" ) );

      resultList->scrollTo( resultList->currentIndex() );

      if( task == FIND_PREV_ID )
        findOptions ^= KFind::FindBackwards;
    }
    break;
  case COPY_SELECTED_TO_CLIPBOARD:
    {
      KUrl::List urls;

      QTreeWidgetItemIterator it(resultList);
      while (*it) {
        if( (*it)->isSelected() )
           urls.push_back( KUrl( (*it)->text( 0 ) ) );

        it++;
      }

      if( urls.count() == 0 )
        return;

      QMimeData *mimeData = new QMimeData;
      mimeData->setImageData( FL_LOADICON( "file" ) );
      urls.populateMimeData(mimeData);

      QApplication::clipboard()->setMimeData( mimeData, QClipboard::Clipboard );
    }
    break;
  }
}

void LocateDlg::nextLine()
{
  if( findOptions & KFind::FindBackwards )
    findCurrentItem = resultList->itemAbove( findCurrentItem );
  else
    findCurrentItem = resultList->itemBelow( findCurrentItem );
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
                &ok, this );
     if ( ! ok)
       return;
  }
    
  KUrl::List urlList;
  QTreeWidgetItemIterator it(resultList);
  while (*it) {
    QTreeWidgetItem * item = *it;
    urlList.push_back( KUrl( item->text( 0 ) ) );
    it++;
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
  setDefaultButton( KDialog::User3 ); // KDE HACK, it's a bug, that the default button is lost somehow
  locateSearchFor->lineEdit()->setFocus();
  locateSearchFor->lineEdit()->selectAll();
}

#include "locate.moc"
