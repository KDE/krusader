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
#include <qpushbutton.h>
#include <qhbox.h>
#include <qvaluestack.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qpixmapcache.h>

#include "diskusage.h"
#include "../VFS/krpermhandler.h"
#include "../VFS/krvfshandler.h"
#include "../kicons.h"

// these are the values that will exist in the menu
#define EXCLUDE_ID          90
#define INCLUDE_ALL_ID      91

DiskUsageDialog::DiskUsageDialog( QWidget *parent, const char *name ) : QDialog( parent, name, true ),
  cancelled( false )
{
  setCaption( i18n("Krusader::Loading Usage Information") );
  
  QGridLayout *synchGrid = new QGridLayout( this );
  synchGrid->setSpacing( 6 );
  synchGrid->setMargin( 11 );
  
  QLabel *filesLabel = new QLabel( i18n( "Files:" ), this, "filesLabel" );
  filesLabel->setFrameShape( QLabel::StyledPanel );
  filesLabel->setFrameShadow( QLabel::Sunken );  
  synchGrid->addWidget( filesLabel, 0, 0 );
  
  QLabel *directoriesLabel = new QLabel( i18n( "Directories:" ), this, "directoriesLabel" );
  directoriesLabel->setFrameShape( QLabel::StyledPanel );
  directoriesLabel->setFrameShadow( QLabel::Sunken );  
  synchGrid->addWidget( directoriesLabel, 1, 0 );
  
  QLabel *totalSizeLabel = new QLabel( i18n( "Total Size:" ), this, "totalSizeLabel" );
  totalSizeLabel->setFrameShape( QLabel::StyledPanel );
  totalSizeLabel->setFrameShadow( QLabel::Sunken );  
  synchGrid->addWidget( totalSizeLabel, 2, 0 );

  files = new QLabel( this, "files" );
  files->setFrameShape( QLabel::StyledPanel );
  files->setFrameShadow( QLabel::Sunken );  
  files->setAlignment( Qt::AlignRight );
  synchGrid->addWidget( files, 0, 1 );
  
  directories = new QLabel( this, "directories" );
  directories->setFrameShape( QLabel::StyledPanel );
  directories->setFrameShadow( QLabel::Sunken );  
  directories->setAlignment( Qt::AlignRight );
  synchGrid->addWidget( directories, 1, 1 );
  
  totalSize = new QLabel( this, "totalSize" );
  totalSize->setFrameShape( QLabel::StyledPanel );
  totalSize->setFrameShadow( QLabel::Sunken );  
  totalSize->setAlignment( Qt::AlignRight );
  synchGrid->addWidget( totalSize, 2, 1 );

  searchedDirectory = new KSqueezedTextLabel( this, "searchedDirectory" );
  searchedDirectory->setFrameShape( QLabel::StyledPanel );
  searchedDirectory->setFrameShadow( QLabel::Sunken );  
  searchedDirectory->setMinimumWidth( QFontMetrics(searchedDirectory->font()).width("W") * 40 );
  synchGrid->addMultiCellWidget( searchedDirectory, 3, 3, 0, 1 );
  
  QFrame *line = new QFrame( this, "duLine" );
  line->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  synchGrid->addMultiCellWidget( line, 4, 4, 0, 1 );

  QHBox *hbox = new QHBox( this, "hbox" );
  QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );    
  hbox->layout()->addItem( spacer );
  QPushButton *cancelButton = new QPushButton( hbox, "cancelButton" );
  cancelButton->setText( i18n( "Cancel"  ) );
  synchGrid->addWidget( hbox, 5, 1 );

  connect( cancelButton, SIGNAL( clicked() ), this, SLOT( slotCancelled() ) );    
  show();
}

void DiskUsageDialog::setCurrentURL( KURL url )
{
  searchedDirectory->setText( url.prettyURL(1,KURL::StripFileProtocol) );
}

void DiskUsageDialog::setValues( int fileNum, int dirNum, KIO::filesize_t total )
{
  files->setText( QString("%1").arg( fileNum ) );
  directories->setText( QString("%1").arg( dirNum ) );
  totalSize->setText( QString("%1").arg( KRpermHandler::parseSize( total ).stripWhiteSpace() ) );
}

void DiskUsageDialog::slotCancelled()
{
  cancelled = true;
}

void DiskUsageDialog::reject()
{
  cancelled = true;
  QDialog::reject();
}

DiskUsage::DiskUsage()
{
  contentMap.setAutoDelete( true );
  propertyMap.setAutoDelete( true );
}

bool DiskUsage::load( KURL baseDir, QWidget *parent )
{
  int fileNum = 0, dirNum = 0;
  KIO::filesize_t totalSize = 0;
  bool result = true;
  baseURL = baseDir;
  
  emit status( i18n( "Loading the disk usage information..." ) );
  
  clear();
    
  DiskUsageDialog *duDlg = new DiskUsageDialog( parent, "DuProgressDialog" );
  
  QValueStack<QString> directoryStack;
  directoryStack.push( "" );
  
  vfs *searchVfs = KrVfsHandler::getVfs( baseDir );
  if( searchVfs == 0 )
  {    
    delete duDlg;
    return false;
  }
  
  searchVfs->vfs_setQuiet( true );
  searchVfs->vfs_enableRefresh( false );
  
  while( !directoryStack.isEmpty() )
  {
    QString dirToCheck = directoryStack.pop();
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
    
    duDlg->setCurrentURL( url );
    
    if( !searchVfs->vfs_refresh( url ) )
      continue;

    dirNum++;

    QPtrList< DiskUsageItem > *dirContent = new QPtrList< DiskUsageItem >();
    dirContent->setAutoDelete( true );
          
    vfile * file = searchVfs->vfs_getFirstFile();
    while( file )
    {      
      fileNum++;
      DiskUsageItem *newItem = new DiskUsageItem( file->vfile_getName(), dirToCheck, file->vfile_getSize(), 
                               file->vfile_getMode(), file->vfile_getOwner(), file->vfile_getGroup(), 
                               file->vfile_getPerm(), file->vfile_getTime_t(), file->vfile_isSymLink(),
                               file->vfile_getMime() );
      dirContent->append( newItem );
      
      if( file->vfile_isDir() && !file->vfile_isSymLink() )
        directoryStack.push( (dirToCheck.isEmpty() ? "" : dirToCheck + "/" )+ file->vfile_getName() );
      else
        totalSize += file->vfile_getSize();
    
      duDlg->setValues( fileNum, dirNum, totalSize );       
      file = searchVfs->vfs_getNextFile();
    }
    
    contentMap.insert( dirToCheck, dirContent );
    
    duDlg->setValues( fileNum, dirNum, totalSize );    
    qApp->processEvents();
    
    if( duDlg->wasCancelled() )
    {
      result = false;
      break;
    }
  }
  delete searchVfs;
  delete duDlg;
  
  calculateSizes();
  
  changeDirectory( "" );  
  return result;
}

QPtrList<DiskUsageItem> * DiskUsage::getDirectory( QString dir )
{
  return contentMap.find( dir );
}

DiskUsageItem * DiskUsage::getItem( QString path )
{
  if( path == "" )
    return 0;
    
  QString dir = path;
      
  int ndx = path.findRev( '/' );
  QString file = path.mid( ndx + 1 );
  
  if( ndx == -1 )
    dir = "";
  else
    dir.truncate( ndx );
    
  QPtrList<DiskUsageItem> * itemList = getDirectory( dir );    
  if( itemList == 0 )
    return 0;

  DiskUsageItem *item = itemList->first();
  while( item )
  {
    if( item->name() == file )
      return item;
      
    item = itemList->next();
  }
  return 0;  
}

void DiskUsage::clear()
{
  emit clearing();
  propertyMap.clear();
  contentMap.clear();
}

void DiskUsage::calculateSizes()
{
  calculateDirSize( "", totalSize, ownSize );
}

void DiskUsage::calculateDirSize( QString dir, KIO::filesize_t &total, KIO::filesize_t &own, bool emitSig )
{
  own = total = 0;
  
  QPtrList<DiskUsageItem> *dirPtr = getDirectory( dir );
  
  if( dirPtr == 0 )
    return;
    
  DiskUsageItem *item = dirPtr->first();
  while( item )
  {
    if( !item->isExcluded() )
    {  
      if( item->isDir() && !item->isSymLink() )
      {
        KIO::filesize_t childOwn = 0, childTotal = 0;
        KIO::filesize_t oldOwn = item->ownSize(), oldTotal = item->size();
        
        calculateDirSize( ( dir.isEmpty() ? "" : dir + "/" ) + item->name(), childTotal, childOwn, emitSig );
        
        item->setSizes( childTotal, childOwn );
        if( emitSig && ( childOwn != oldOwn || childTotal != oldTotal ) )
          emit changed( item );
      
        total += childTotal;
      }
      else
      {
        own += item->size();
        total += item->size();
      }
    }
    
    item = dirPtr->next();
  }
}

void DiskUsage::excludeDir( QString dir )
{
  QPtrList<DiskUsageItem> *dirPtr = getDirectory( dir );
  if( dirPtr == 0 )
    return;
    
  DiskUsageItem *item = dirPtr->first();
  while( item )
  {
    if( !item->isExcluded() )
    {
      item->exclude( true );
      emit changed( item );
    }
  
    if( item->isDir() && !item->isSymLink() )
      excludeDir( ( dir.isEmpty() ? "" : dir + "/" ) + item->name() );
    
    item = dirPtr->next();
  }
}

void DiskUsage::exclude( QString dir, QString name )
{
  QPtrList<DiskUsageItem> *dirPtr = getDirectory( dir );
  if( dirPtr == 0 )
    return;
    
  DiskUsageItem *item = dirPtr->first();
  while( item )
  {
    if( item->name() == name )
    {
      if( !item->isExcluded() )
      {
        item->exclude( true );
        emit changed( item );
      }
        
      if( item->isDir() && !item->isSymLink() )
        excludeDir( ( dir.isEmpty() ? "" : dir + "/" ) + item->name() );
    
      break;
    }
    item = dirPtr->next();
  }
  calculateDirSize( "", totalSize, ownSize, true );
  calculatePercents( true );
  createStatus();
}

void DiskUsage::includeDir( QString dir )
{
  QPtrList<DiskUsageItem> *dirPtr = getDirectory( dir );
  if( dirPtr == 0 )
    return;
    
  DiskUsageItem *item = dirPtr->first();
  while( item )
  {
    if( item->isExcluded() )
    {
      item->exclude( false );
      emit changed( item );
    }
  
    if( item->isDir() && !item->isSymLink() )
      includeDir( ( dir.isEmpty() ? "" : dir + "/" ) + item->name() );
    
    item = dirPtr->next();
  }
}

void DiskUsage::includeAll()
{
  includeDir( "" );
  calculateDirSize( "", totalSize, ownSize, true );
  calculatePercents( true );
  createStatus();
}

void * DiskUsage::getProperty( DiskUsageItem *item, QString key )
{
  Properties * props = propertyMap.find( item );
  if( props == 0 )
    return 0;
  return props->find( key );
}

void DiskUsage::addProperty( DiskUsageItem *item, QString key, void * prop )
{
  Properties * props = propertyMap.find( item );
  if( props == 0 )
  {
    props = new Properties();
    propertyMap.insert( item, props );
  }
  props->insert( key, prop );
}

void DiskUsage::removeProperty( DiskUsageItem *item, QString key )
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
  QString dir = currentDirectory;

  KIO::filesize_t totalDirSize = totalSize;
  KIO::filesize_t ownDirSize = ownSize;
  
  if( !dir.isEmpty() )
  {
    DiskUsageItem *item = getItem( dir );
    if( item )
    {
      totalDirSize = item->size();
      ownDirSize = item->ownSize();
    }
  }   
  
  KURL url = baseURL;  
  if( !dir.isEmpty() )
      url.addPath( dir );
  
  emit status( i18n( "Current directory:%1,  Total size:%2,  Own size:%3" )
               .arg( url.prettyURL(-1,KURL::StripFileProtocol) )
               .arg( " "+KRpermHandler::parseSize( totalDirSize ) )
               .arg( " "+KRpermHandler::parseSize( ownDirSize ) ) );
}  

void DiskUsage::changeDirectory( QString dir )
{
  currentDirectory = dir;
  calculatePercents();  
  createStatus();
  
  emit directoryChanged( dir );
}

QString DiskUsage::getCurrentDir()
{
  return currentDirectory;
}

void DiskUsage::rightClickMenu( DiskUsageItem *duItem )
{
  KPopupMenu popup;
  popup.insertTitle(i18n("Disk Usage"));

  popup.insertItem(i18n("Exclude"),     EXCLUDE_ID);
  popup.insertItem(i18n("Include All"), INCLUDE_ALL_ID);

  int result=popup.exec(QCursor::pos());

  // check out the user's option
  switch (result)
  {
  case EXCLUDE_ID:
    exclude( duItem->directory(), duItem->name() );
    break;
  case INCLUDE_ALL_ID:
    includeAll();
    break;
  }
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

void DiskUsage::calculatePercents( bool emitSig )
{
  KIO::filesize_t currentSize = totalSize;
  
  if( !currentDirectory.isEmpty() )
  {
    DiskUsageItem *dirItem = getItem( currentDirectory );
    if( dirItem == 0 )
      currentSize = 0;
    else
      currentSize = dirItem->size();
  }
  
  calculatePercentsDir( currentDirectory, currentSize, emitSig );
}  

void DiskUsage::calculatePercentsDir( QString dir, KIO::filesize_t currentSize, bool emitSig )
{
  QPtrList<DiskUsageItem> *dirPtr = getDirectory( dir );
  if( dirPtr == 0 )
    return;
    
  DiskUsageItem *item = dirPtr->first();
  while( item )
  {
    if( !item->isExcluded() )
    {
      int newPerc;
      
      if( currentSize == 0 && item->size() == 0 )
        newPerc = 0;
      else if( currentSize == 0 )
        newPerc = -1;
      else
        newPerc = (int)((double)item->size() / (double)currentSize * 10000. + 0.5);
      
      int oldPerc = item->intPercent();
      item->setPercent( newPerc );
    
      if( emitSig && newPerc != oldPerc )
        emit changed( item );
    }
  
    if( item->isDir() && !item->isSymLink() )
      calculatePercentsDir( ( dir.isEmpty() ? "" : dir + "/" ) + item->name(), currentSize, emitSig );
    
    item = dirPtr->next();
  }
}

QString DiskUsage::getToolTip( DiskUsageItem *item )
{
  KMimeType::Ptr mimePtr = KMimeType::mimeType( item->mime() );
  QString mime = mimePtr->comment();
    
  time_t tma = item->time();
  struct tm* t=localtime((time_t *)&tma);
  QDateTime tmp(QDate(t->tm_year+1900, t->tm_mon+1, t->tm_mday), QTime(t->tm_hour, t->tm_min));
  QString date = KGlobal::locale()->formatDateTime(tmp);    
  
  QString str = "<qt><h5><table><tr><td>" + i18n( "Name:" ) +  "</td><td>" + item->name() + "</td></tr>"+
                "<tr><td>" + i18n( "Type:" ) +  "</td><td>" + mime + "</td></tr>"+
                "<tr><td>" + i18n( "Size:" ) +  "</td><td>" + KRpermHandler::parseSize( item->size() ) + "</td></tr>";

  if( item->isDir() && !item->isSymLink() )
    str +=      "<tr><td>" + i18n( "Own size:" ) +  "</td><td>" + KRpermHandler::parseSize( item->ownSize() ) + "</td></tr>";
                                
  str +=        "<tr><td>" + i18n( "Last modified:" ) +  "</td><td>" + date + "</td></tr>"+
                "<tr><td>" + i18n( "Permissions:" ) +  "</td><td>" + item->perm() + "</td></tr>"+
                "<tr><td>" + i18n( "Owner:" ) +  "</td><td>" + item->owner() + " - " + item->group() + "</td></tr>"+
                "</table></h5></qt>";
  str.replace( " ", "&nbsp;" );
  return str;
}

#include "diskusage.moc"
