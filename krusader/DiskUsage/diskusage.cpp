/***************************************************************************
                         diskusage.cpp  -  description
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

#include <qlayout.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kmimetype.h>
#include <kmessagebox.h>
#include <kglobalsettings.h>
#include <qpushbutton.h>
#include <qhbox.h>
#include <qvaluestack.h>
#include <qptrstack.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qpixmapcache.h>
#include <qgroupbox.h>
#include <time.h> 
#include "diskusage.h"
#include "../VFS/krpermhandler.h"
#include "../VFS/krvfshandler.h"
#include "../kicons.h"
#include "../defaults.h"
#include "../krusader.h"
#include "../krusaderview.h"
#include "../Panel/listpanel.h"
#include "../Panel/panelfunc.h"
#include "filelightParts/Config.h"

#include "dulines.h"
#include "dulistview.h"
#include "dufilelight.h"

// these are the values that will exist in the menu
#define DELETE_ID            90
#define EXCLUDE_ID           91
#define PARENT_DIR_ID        92
#define NEW_SEARCH_ID        93
#define REFRESH_ID           94
#define STEP_INTO_ID         95
#define INCLUDE_ALL_ID       96
#define VIEW_POPUP_ID        97
#define LINES_VIEW_ID        98
#define DETAILED_VIEW_ID     99
#define FILELIGHT_VIEW_ID   100
#define NEXT_VIEW_ID        101
#define PREVIOUS_VIEW_ID    102
#define ADDITIONAL_POPUP_ID 103

LoaderWidget::LoaderWidget( QWidget *parent, const char *name ) : QScrollView( parent, name ), cancelled( false )
{
  viewport()->setEraseColor( Qt::white );
  widget = new QWidget( parent );

  QGridLayout *loaderLayout = new QGridLayout( widget );
  loaderLayout->setSpacing( 0 );
  loaderLayout->setMargin( 0 );  
  
  QGroupBox *loaderBox = new QGroupBox( widget, "loaderGroupBox" );
  loaderBox->setFrameShape( QGroupBox::Box );
  loaderBox->setFrameShadow( QGroupBox::Sunken );
  loaderBox->setColumnLayout(0, Qt::Vertical );
  loaderBox->layout()->setSpacing( 0 );
  loaderBox->layout()->setMargin( 0 );
  loaderBox->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  loaderBox->setFrameStyle( QFrame::Panel + QFrame::Raised );
  loaderBox->setLineWidth( 2 );
  
  QGridLayout *synchGrid = new QGridLayout( loaderBox->layout() );
  synchGrid->setSpacing( 6 );
  synchGrid->setMargin( 11 );
  
  QLabel *titleLabel = new QLabel( i18n( "Loading Usage Information" ), loaderBox, "titleLabel" );
  titleLabel->setAlignment( Qt::AlignHCenter );
  synchGrid->addMultiCellWidget( titleLabel, 0, 0, 0, 1 );
  
  QLabel *filesLabel = new QLabel( i18n( "Files:" ), loaderBox, "filesLabel" );
  filesLabel->setFrameShape( QLabel::StyledPanel );
  filesLabel->setFrameShadow( QLabel::Sunken );  
  synchGrid->addWidget( filesLabel, 1, 0 );
  
  QLabel *directoriesLabel = new QLabel( i18n( "Directories:" ), loaderBox, "directoriesLabel" );
  directoriesLabel->setFrameShape( QLabel::StyledPanel );
  directoriesLabel->setFrameShadow( QLabel::Sunken );  
  synchGrid->addWidget( directoriesLabel, 2, 0 );
  
  QLabel *totalSizeLabel = new QLabel( i18n( "Total Size:" ), loaderBox, "totalSizeLabel" );
  totalSizeLabel->setFrameShape( QLabel::StyledPanel );
  totalSizeLabel->setFrameShadow( QLabel::Sunken );  
  synchGrid->addWidget( totalSizeLabel, 3, 0 );

  files = new QLabel( loaderBox, "files" );
  files->setFrameShape( QLabel::StyledPanel );
  files->setFrameShadow( QLabel::Sunken );  
  files->setAlignment( Qt::AlignRight );
  synchGrid->addWidget( files, 1, 1 );
  
  directories = new QLabel( loaderBox, "directories" );
  directories->setFrameShape( QLabel::StyledPanel );
  directories->setFrameShadow( QLabel::Sunken );  
  directories->setAlignment( Qt::AlignRight );
  synchGrid->addWidget( directories, 2, 1 );
  
  totalSize = new QLabel( loaderBox, "totalSize" );
  totalSize->setFrameShape( QLabel::StyledPanel );
  totalSize->setFrameShadow( QLabel::Sunken );  
  totalSize->setAlignment( Qt::AlignRight );
  synchGrid->addWidget( totalSize, 3, 1 );

  int width;
  searchedDirectory = new KSqueezedTextLabel( loaderBox, "searchedDirectory" );
  searchedDirectory->setFrameShape( QLabel::StyledPanel );
  searchedDirectory->setFrameShadow( QLabel::Sunken );  
  searchedDirectory->setMinimumWidth( width = QFontMetrics(searchedDirectory->font()).width("W") * 30 );
  searchedDirectory->setMaximumWidth( width );
  synchGrid->addMultiCellWidget( searchedDirectory, 4, 4, 0, 1 );
  
  QFrame *line = new QFrame( loaderBox, "duLine" );
  line->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  synchGrid->addMultiCellWidget( line, 5, 5, 0, 1 );

  QHBox *hbox = new QHBox( loaderBox, "hbox" );
  QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );    
  hbox->layout()->addItem( spacer );
  QPushButton *cancelButton = new QPushButton( hbox, "cancelButton" );
  cancelButton->setText( i18n( "Cancel"  ) );
  synchGrid->addWidget( hbox, 6, 1 );
  
  loaderLayout->addWidget( loaderBox, 0, 0 );

  addChild( widget );
  
  connect( cancelButton, SIGNAL( clicked() ), this, SLOT( slotCancelled() ) );    
}

void LoaderWidget::resizeEvent ( QResizeEvent *e ) 
{
  QScrollView::resizeEvent( e );

  int x = ( viewport()->width() - widget->width() ) / 2;
  int y = ( viewport()->height() - widget->height() ) / 2;
  if( x < 0 ) x=0;
  if( y < 0 ) y=0;
  
  moveChild( widget, x, y );
}

void LoaderWidget::init()
{
  cancelled = false;
}
  
void LoaderWidget::setCurrentURL( KURL url )
{
  searchedDirectory->setText( url.prettyURL(1,KURL::StripFileProtocol) );
}
  
void LoaderWidget::setValues( int fileNum, int dirNum, KIO::filesize_t total )
{
  files->setText( QString("%1").arg( fileNum ) );
  directories->setText( QString("%1").arg( dirNum ) );
  totalSize->setText( QString("%1").arg( KRpermHandler::parseSize( total ).stripWhiteSpace() ) );
}
  
void LoaderWidget::slotCancelled()
{
  cancelled = true;
}

DiskUsage::DiskUsage( QString confGroup, QWidget *parent, char *name ) : QWidgetStack( parent, name ), 
                      currentDirectory( 0 ), root( 0 ), configGroup( confGroup ), loading( false ), abortLoading( false )
{
  listView = new DUListView( this, "DU ListView" );
  lineView = new DULines( this, "DU LineView" );
  filelightView = new DUFilelight( this, "Filelight canvas" );
  loaderView = new LoaderWidget( this, "Loading view" );
  
  addWidget( listView );
  addWidget( lineView );
  addWidget( filelightView );
  addWidget( loaderView );   
   
  setView( VIEW_LINES );
      
  Filelight::Config::read();  
  propertyMap.setAutoDelete( true );
  
  setFocusPolicy( StrongFocus );
}

DiskUsage::~DiskUsage()
{
  if( root )
    delete root;
    
  if( listView )         // don't remove these lines. The module will crash at exit if removed
    delete listView;
  if( lineView )
    delete lineView;
  if( filelightView )
    delete filelightView;
}

bool DiskUsage::load( KURL baseDir )
{
  if( loading )
  {
    stopLoad();
    urlToLoad = baseDir;
    QTimer::singleShot( 0, this, SLOT( loadAfterStop() ) );
    return false;
  }
  
  int fileNum = 0, dirNum = 0;
  bool result = true;
  KIO::filesize_t totalSize = 0;
  Directory *parent = 0;
      
  emit status( i18n( "Loading the disk usage information..." ) );
  
  clear();
  
  baseURL = baseDir;
  baseURL.setPath( baseDir.path( -1 ) );

  root = new Directory( baseURL.fileName().ascii(), baseDir.prettyURL( 0, KURL::StripFileProtocol ) );

  QValueStack<QString> directoryStack;
  QPtrStack<Directory> parentStack;
  directoryStack.push( "" );
  parentStack.push( root );
  
  vfs *searchVfs = KrVfsHandler::getVfs( baseDir );
  if( searchVfs == 0 )
    return false;
  
  searchVfs->vfs_setQuiet( true );
  searchVfs->vfs_enableRefresh( false );
  
  int lastView = activeView;
  setView( VIEW_LOADER );
  
  loading = true;
  
  loaderView->init();  
  
  while( !directoryStack.isEmpty() )
  {      
    QString dirToCheck = directoryStack.pop();
    parent = parentStack.pop();
    
    contentMap.insert( dirToCheck, parent );
    
    KURL url = baseDir;
    
    if( !dirToCheck.isEmpty() )
      url.addPath( dirToCheck );
    
#if defined(BSD)
    if ( url.isLocalFile() && url.path().left( 7 ) == "/procfs" )
      continue;
#else
    if ( url.isLocalFile() && url.path().left( 5 ) == "/proc" )
      continue;
#endif
    
    loaderView->setCurrentURL( url );
    
    if( !searchVfs->vfs_refresh( url ) )
      continue;

    dirNum++;

    vfile * file = searchVfs->vfs_getFirstFile();
    while( file )
    {      
      fileNum++;
      File *newItem = 0;
      
      if( file->vfile_isDir() && !file->vfile_isSymLink() )
      {
        newItem = new Directory( parent, file->vfile_getName(), dirToCheck, file->vfile_getSize(), 
                                 file->vfile_getMode(), file->vfile_getOwner(), file->vfile_getGroup(), 
                                 file->vfile_getPerm(), file->vfile_getTime_t(), file->vfile_isSymLink(),
                                 file->vfile_getMime() );
        directoryStack.push( (dirToCheck.isEmpty() ? "" : dirToCheck + "/" )+ file->vfile_getName() );
        parentStack.push( dynamic_cast<Directory *>( newItem ) );
      }
      else
      {
        newItem = new File( parent, file->vfile_getName(), dirToCheck, file->vfile_getSize(), 
                            file->vfile_getMode(), file->vfile_getOwner(), file->vfile_getGroup(), 
                            file->vfile_getPerm(), file->vfile_getTime_t(), file->vfile_isSymLink(),
                            file->vfile_getMime() );
        totalSize += file->vfile_getSize();
      }      
      parent->append( newItem );
          
      loaderView->setValues( fileNum, dirNum, totalSize );       
      file = searchVfs->vfs_getNextFile();
    }
    
    loaderView->setValues( fileNum, dirNum, totalSize );    
    qApp->processEvents();
    
    if( loaderView->wasCancelled() || abortLoading )
    {
      result = false;
      break;
    }
  }
  delete searchVfs;
  
  setView( lastView );
  
  calculateSizes();
  changeDirectory( root );
  
  loading = abortLoading = false;
  
  emit loadFinished( result );  
  return result;
}

void DiskUsage::loadAfterStop()
{
  load( urlToLoad );
}

void DiskUsage::stopLoad()
{
  abortLoading = true;
}

void DiskUsage::dirUp()
{
  if( currentDirectory != 0 && currentDirectory->parent() != 0 )
    changeDirectory( (Directory *)(currentDirectory->parent()) );
}

Directory * DiskUsage::getDirectory( QString dir )
{
  while( dir.endsWith( "/" ) )
    dir.truncate( dir.length() - 1 );
    
  if( dir.isEmpty() )
    return root;
    
  return contentMap.find( dir );
}

File * DiskUsage::getFile( QString path )
{
  if( path == "" )
    return root;
    
  QString dir = path;
      
  int ndx = path.findRev( '/' );
  QString file = path.mid( ndx + 1 );
  
  if( ndx == -1 )
    dir = "";
  else
    dir.truncate( ndx );
    
  Directory *dirEntry = getDirectory( dir );    
  if( dirEntry == 0 )
    return 0;
    
  for( Iterator<File> it = dirEntry->iterator(); it != dirEntry->end(); ++it )
    if( (*it)->fileName() == file )
      return *it;
  
  return 0;  
}

void DiskUsage::clear()
{
  baseURL = KURL();
  emit clearing();
  propertyMap.clear();
  contentMap.clear();
  if( root )
    delete root;
  root = currentDirectory = 0;
}

void DiskUsage::calculateSizes( Directory *dirEntry, bool emitSig )
{
  if( dirEntry == 0 )
    dirEntry = root;

  KIO::filesize_t own = 0, total = 0;

  for( Iterator<File> it = dirEntry->iterator(); it != dirEntry->end(); ++it )
  {
    File * item = *it;
  
    if( !item->isExcluded() )
    {  
      if( item->isDir() )
        calculateSizes( dynamic_cast<Directory *>( item ), emitSig );
      else
        own += item->size();
        
      total += item->size();
    }
  }

  KIO::filesize_t oldOwn = dirEntry->ownSize(), oldTotal = dirEntry->size();
  dirEntry->setSizes( total, own );
  
  if( dirEntry == currentDirectory )
    currentSize = total;
  
  if( emitSig && ( own != oldOwn || total != oldTotal ) )
    emit changed( dirEntry );
}

void DiskUsage::exclude( File *file, bool calcPercents )
{
  if( !file->isExcluded() )
  {
    file->exclude( true );
    emit changed( file );
  
    if( file->isDir() )
    {      
      Directory *dir = dynamic_cast<Directory *>( file );
      for( Iterator<File> it = dir->iterator(); it != dir->end(); ++it )
        exclude( *it, false );
    }
  }
  
  if( calcPercents )
  {  
    calculateSizes( root, true );
    calculatePercents( true );
    createStatus();
  }
}

void DiskUsage::include( Directory *dir )
{
  if( dir == 0 ) 
    return;
      
  for( Iterator<File> it = dir->iterator(); it != dir->end(); ++it )
  {
    File *item = *it;
    
    if( item->isDir() )
      include( dynamic_cast<Directory *>( item ) );
  
    if( item->isExcluded() )
    {
      item->exclude( false );
      emit changed( item );
    }  
  }
}

void DiskUsage::includeAll()
{
  include( root );
  calculateSizes( root, true );
  calculatePercents( true );
  createStatus();
}

void DiskUsage::del( File *file, bool calcPercents )
{
  if( file == root )
    return;
 
  if( calcPercents )
  {
    krConfig->setGroup( "Advanced" );
    if ( krConfig->readBoolEntry( "Confirm Delete", _ConfirmDelete ) ) 
    {
      if ( KMessageBox::warningContinueCancel( this, i18n( "Are you sure you want to delete " ) 
                                             + file->fullPath() + "?" ) != KMessageBox::Continue )
         return ;
    }
  }
     
  if( file == currentDirectory )
    dirUp();
  
  if( file->isDir() )
  {      
    Directory *dir = dynamic_cast<Directory *>( file );
    
    Iterator<File> it;
    while( ( it = dir->iterator() ) != dir->end() )
      del( *it, false );
  
    QString path;  
    for( const Directory *d = (Directory*)file; d != root && d && d->parent() != 0; d = d->parent() )
    {
      if( !path.isEmpty() )
        path = "/" + path;
        
      path = d->fileName() + path;
    }
    
    contentMap.remove( path );
  }

  emit deleted( file );
  
  krConfig->setGroup("General");
  if( krConfig->readBoolEntry("Move To Trash",_MoveToTrash) )
  {
    KIO::Job *job = new KIO::CopyJob( vfs::fromPathOrURL( file->fullPath() ),KGlobalSettings::trashPath(),KIO::CopyJob::Move,false,false );
    connect(job,SIGNAL(result(KIO::Job*)),krApp,SLOT(changeTrashIcon()));
  }
  else
    new KIO::DeleteJob( vfs::fromPathOrURL( file->fullPath() ), false, false);

  ((Directory *)(file->parent()))->remove( file );
  delete file;
    
  if( calcPercents )
  {  
    qApp->processEvents(); // execute the delete
    
    calculateSizes( root, true );
    calculatePercents( true );
    createStatus();
    emit enteringDirectory( currentDirectory );
  }  
}

void * DiskUsage::getProperty( File *item, QString key )
{
  Properties * props = propertyMap.find( item );
  if( props == 0 )
    return 0;
  return props->find( key );
}

void DiskUsage::addProperty( File *item, QString key, void * prop )
{
  Properties * props = propertyMap.find( item );
  if( props == 0 )
  {
    props = new Properties();
    propertyMap.insert( item, props );
  }
  props->insert( key, prop );
}

void DiskUsage::removeProperty( File *item, QString key )
{
  Properties * props = propertyMap.find( item );
  if( props == 0 )
    return;  
  props->remove( key );
  if( props->count() == 0 )
    propertyMap.remove( item );
}

void DiskUsage::createStatus()
{
  Directory *dirEntry = currentDirectory;

  if( dirEntry == 0 )
    return;
  
  KURL url = baseURL;  
  if( dirEntry != root )
      url.addPath( dirEntry->directory() );
  
  emit status( i18n( "Current directory:%1,  Total size:%2,  Own size:%3" )
               .arg( url.prettyURL(-1,KURL::StripFileProtocol) )
               .arg( " "+KRpermHandler::parseSize( dirEntry->size() ) )
               .arg( " "+KRpermHandler::parseSize( dirEntry->ownSize() ) ) );
}  

void DiskUsage::changeDirectory( Directory *dir )
{
  currentDirectory = dir;
  
  currentSize = dir->size();  
  calculatePercents( true, dir );
  
  createStatus();  
  emit enteringDirectory( dir );
}

Directory* DiskUsage::getCurrentDir()
{
  return currentDirectory;
}

void DiskUsage::rightClickMenu( File *fileItem, KPopupMenu *addPopup, QString addPopupName )
{
  KPopupMenu popup( this );
  
  popup.insertTitle( i18n("Disk Usage"));
  
  if( fileItem != 0 )
  {
    popup.insertItem(  i18n("Delete"),          DELETE_ID);
    popup.setAccel( Key_Delete, DELETE_ID );    
    popup.insertItem(  i18n("Exclude"),         EXCLUDE_ID);
    popup.setAccel( CTRL + Key_E, EXCLUDE_ID );    
    popup.insertSeparator();
  }
  
  popup.insertItem(  i18n("Up one directory"),  PARENT_DIR_ID);
  popup.setAccel( SHIFT + Key_Up, PARENT_DIR_ID );    
  popup.insertItem(  i18n("New search"),        NEW_SEARCH_ID);
  popup.setAccel( CTRL + Key_N, NEW_SEARCH_ID );    
  popup.insertItem(  i18n("Refresh"),           REFRESH_ID);
  popup.setAccel( CTRL + Key_R, REFRESH_ID );  
  popup.insertItem(  i18n("Include all"),       INCLUDE_ALL_ID);
  popup.setAccel( CTRL + Key_I, INCLUDE_ALL_ID );  
  popup.insertItem(  i18n("Step into"),         STEP_INTO_ID);
  popup.setAccel( SHIFT + Key_Down, STEP_INTO_ID );    
  popup.insertSeparator();
  
  
  if( addPopup != 0 )
  {
    popup.insertItem( QPixmap(), addPopup, ADDITIONAL_POPUP_ID );
    popup.changeItem( ADDITIONAL_POPUP_ID, addPopupName );
  }  
  
  KPopupMenu viewPopup;
  viewPopup.insertItem(i18n("Lines"),      LINES_VIEW_ID);
  viewPopup.setAccel( CTRL + Key_L, LINES_VIEW_ID );    
  viewPopup.insertItem(i18n("Detailed"),   DETAILED_VIEW_ID);
  viewPopup.setAccel( CTRL + Key_D, DETAILED_VIEW_ID );    
  viewPopup.insertItem(i18n("Filelight"),  FILELIGHT_VIEW_ID);
  viewPopup.setAccel( CTRL + Key_F, FILELIGHT_VIEW_ID );    
  viewPopup.insertSeparator();
  viewPopup.insertItem(i18n("Next"),       NEXT_VIEW_ID);
  viewPopup.setAccel( SHIFT + Key_Right, NEXT_VIEW_ID );    
  viewPopup.insertItem(i18n("Previous"),   PREVIOUS_VIEW_ID);
  viewPopup.setAccel( SHIFT + Key_Left, PREVIOUS_VIEW_ID );    
  
  popup.insertItem( QPixmap(), &viewPopup, VIEW_POPUP_ID );
  popup.changeItem( VIEW_POPUP_ID, i18n( "View" ) );
    
  int result=popup.exec(QCursor::pos());

  executeAction( result, fileItem );
}

void DiskUsage::executeAction( int action, File * fileItem )  
{
  // check out the user's option
  switch ( action )
  {
  case DELETE_ID:
    if( fileItem )
      del( fileItem );
    break;
  case EXCLUDE_ID:
    if( fileItem )
      exclude( fileItem );
    break;
  case PARENT_DIR_ID:
    dirUp();
    break;
  case NEW_SEARCH_ID:
    emit newSearch();
    break;    
  case REFRESH_ID:
    load( baseURL );
    break;
  case INCLUDE_ALL_ID:
    includeAll();
    break;
  case STEP_INTO_ID:
    {
      QString uri;
      if( fileItem && fileItem->isDir() )
        uri = fileItem->fullPath();
      else
        uri = currentDirectory->fullPath();      
      ACTIVE_FUNC->delayedOpenUrl(vfs::fromPathOrURL( uri ));
    }
    break;
  case LINES_VIEW_ID:
    setView( VIEW_LINES );
    break;
  case DETAILED_VIEW_ID:
    setView( VIEW_DETAILED );
    break;
  case FILELIGHT_VIEW_ID:
    setView( VIEW_FILELIGHT );
    break;
  case NEXT_VIEW_ID:
    setView( ( activeView + 1 ) % 3 );
    break;
  case PREVIOUS_VIEW_ID:
    setView( ( activeView + 2 ) % 3 );
    break;
  }  
  setFocus();
}

void DiskUsage::keyPressEvent( QKeyEvent *e )
{
  if( activeView != VIEW_LOADER )
  {
    switch ( e->key() )
    {
    case Key_E:
      if( e->state() == ControlButton )
      {
        executeAction( EXCLUDE_ID, getCurrentFile() );
        return;
      }
    case Key_D:
      if( e->state() == ControlButton )
      {
        executeAction( DETAILED_VIEW_ID );
        return;
      }
    case Key_F:
      if( e->state() == ControlButton )
      {
        executeAction( FILELIGHT_VIEW_ID );
        return;
      }
    case Key_I:
      if( e->state() == ControlButton )
      {
        executeAction( INCLUDE_ALL_ID );
        return;
      }
      break;
    case Key_L:
      if( e->state() == ControlButton )
      {
        executeAction( LINES_VIEW_ID );
        return;
      }
    case Key_N:
      if( e->state() == ControlButton )
      {
        executeAction( NEW_SEARCH_ID );
        return;
      }
      break;
    case Key_R:
      if( e->state() == ControlButton )
      {
        executeAction( REFRESH_ID );
        return;
      }
      break;
    case Key_Up:
      if( e->state() == ShiftButton )
      {
        executeAction( PARENT_DIR_ID );
        return;
      }
      break;
    case Key_Down:
      if( e->state() == ShiftButton )
      {
        executeAction( STEP_INTO_ID );
        return;
      }
      break;
    case Key_Left:
      if( e->state() == ShiftButton )
      {
        executeAction( PREVIOUS_VIEW_ID );
        return;
      }
      break;  
    case Key_Right:
      if( e->state() == ShiftButton )
      {
        executeAction( NEXT_VIEW_ID );
        return;
      }
      break;  
    case Key_Delete:
      if( !e->state() )
      {
        executeAction( DELETE_ID, getCurrentFile() );
        return;
      }
    case Key_Plus:
      if( activeView == VIEW_FILELIGHT )
      {
        filelightView->zoomIn();
        return;
      }
      break;
    case Key_Minus:
      if( activeView == VIEW_FILELIGHT )
      {
        filelightView->zoomOut();
        return;
      }
      break;
    }
  }
  QWidgetStack::keyPressEvent( e );
}

QPixmap DiskUsage::getIcon( QString mime )
{
  QPixmap icon;
  
  if ( !QPixmapCache::find( mime, icon ) ) 
  {
    // get the icon.
    if ( mime == "Broken Link !" )
      icon = FL_LOADICON( "file_broken" );
    else 
      icon = FL_LOADICON( KMimeType::mimeType( mime ) ->icon( QString::null, true ) );
      
    // insert it into the cache
    QPixmapCache::insert( mime, icon );
  }
  return icon;
}

void DiskUsage::calculatePercents( bool emitSig, Directory *dirEntry )
{
  if( dirEntry == 0 )
    dirEntry = root;
    
  for( Iterator<File> it = dirEntry->iterator(); it != dirEntry->end(); ++it )
  {    
    File *item = *it;
  
    if( !item->isExcluded() )
    {
      int newPerc;
      
      if( dirEntry->size() == 0 && item->size() == 0 )
        newPerc = 0;
      else if( dirEntry->size() == 0 )
        newPerc = -1;
      else
        newPerc = (int)((double)item->size() / (double)currentSize * 10000. + 0.5);
      
      int oldPerc = item->intPercent();
      item->setPercent( newPerc );
    
      if( emitSig && newPerc != oldPerc )
        emit changed( item );
  
      if( item->isDir() )
        calculatePercents( emitSig, dynamic_cast<Directory *>( item ) );
    }
  }
}

QString DiskUsage::getToolTip( File *item )
{
  KMimeType::Ptr mimePtr = KMimeType::mimeType( item->mime() );
  QString mime = mimePtr->comment();
    
  time_t tma = item->time();
  struct tm* t=localtime((time_t *)&tma);
  QDateTime tmp(QDate(t->tm_year+1900, t->tm_mon+1, t->tm_mday), QTime(t->tm_hour, t->tm_min));
  QString date = KGlobal::locale()->formatDateTime(tmp);    
  
  QString str = "<qt><h5><table><tr><td>" + i18n( "Name:" ) +  "</td><td>" + item->fileName() + "</td></tr>"+
                "<tr><td>" + i18n( "Type:" ) +  "</td><td>" + mime + "</td></tr>"+
                "<tr><td>" + i18n( "Size:" ) +  "</td><td>" + KRpermHandler::parseSize( item->size() ) + "</td></tr>";

  if( item->isDir() )
    str +=      "<tr><td>" + i18n( "Own size:" ) +  "</td><td>" + KRpermHandler::parseSize( item->ownSize() ) + "</td></tr>";
                                
  str +=        "<tr><td>" + i18n( "Last modified:" ) +  "</td><td>" + date + "</td></tr>"+
                "<tr><td>" + i18n( "Permissions:" ) +  "</td><td>" + item->perm() + "</td></tr>"+
                "<tr><td>" + i18n( "Owner:" ) +  "</td><td>" + item->owner() + " - " + item->group() + "</td></tr>"+
                "</table></h5></qt>";
  str.replace( " ", "&nbsp;" );
  return str;
}

void DiskUsage::setView( int view )
{
  switch( view )
  {
  case VIEW_LINES:
    raiseWidget( lineView );
    break;
  case VIEW_DETAILED:
    raiseWidget( listView );
    break;
  case VIEW_FILELIGHT:
    raiseWidget( filelightView );
    break;
  case VIEW_LOADER:
    raiseWidget( loaderView );
    break;    
  }
  
  setFocus();  
  emit viewChanged( activeView = view );
}

File * DiskUsage::getCurrentFile()
{
  File * file = 0;
  
  switch( activeView )
  {
  case VIEW_LINES:
    file = lineView->getCurrentFile();
    break;
  case VIEW_DETAILED:
    file = listView->getCurrentFile();
    break;
  case VIEW_FILELIGHT:
    file = filelightView->getCurrentFile();
    break;
  }
  
  return file;
}

bool DiskUsage::event( QEvent * e )
{
  if ( e->type() == QEvent::AccelOverride ) 
  {
    QKeyEvent* ke = (QKeyEvent*) e;
    
    if ( ke->state() == NoButton || ke->state() == Keypad ) 
    {
      switch ( ke->key() ) 
      {
        case Key_Delete:
        case Key_Plus:
        case Key_Minus:
          ke->accept();
          break;
      }
    }else if( ke->state() == ShiftButton )
    {
      switch ( ke->key() ) 
      {        
        case Key_Left:
        case Key_Right:
        case Key_Up:
        case Key_Down:
          ke->accept();
          break;
      }
    }else if ( ke->state() & ControlButton ) 
    {
      switch ( ke->key() )       
      {
        case Key_D:
        case Key_E:
        case Key_F:
        case Key_I:
        case Key_L:
        case Key_N:
        case Key_R:
          ke->accept();
          break;
      }
    }
  }
  return QWidgetStack::event( e );    
}

#include "diskusage.moc"
