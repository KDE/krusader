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
#include <qptrstack.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qpixmapcache.h>
#include <time.h> 
#include "diskusage.h"
#include "../VFS/krpermhandler.h"
#include "../VFS/krvfshandler.h"
#include "../kicons.h"
#include "filelightParts/Config.h"

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

DiskUsage::DiskUsage() : root( 0 )
{
  Filelight::Config::read();  
  propertyMap.setAutoDelete( true );
}

DiskUsage::~DiskUsage()
{
  if( root )
    delete root;
}

bool DiskUsage::load( KURL baseDir, QWidget *parentWidget )
{
  int fileNum = 0, dirNum = 0;
  bool result = true;
  baseURL = baseDir;
  KIO::filesize_t totalSize = 0;
  Directory *parent = 0;
      
  emit status( i18n( "Loading the disk usage information..." ) );
  
  clear();
  root = new Directory();
    
  DiskUsageDialog *duDlg = new DiskUsageDialog( parentWidget, "DuProgressDialog" );
  
  QValueStack<QString> directoryStack;
  QPtrStack<Directory> parentStack;
  directoryStack.push( "" );
  parentStack.push( root );
  
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
    
    duDlg->setCurrentURL( url );
    
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
          
      duDlg->setValues( fileNum, dirNum, totalSize );       
      file = searchVfs->vfs_getNextFile();
    }
    
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
  changeDirectory( root );  
  return result;
}

Directory * DiskUsage::getDirectory( QString dir )
{
  return contentMap.find( dir );
}

File * DiskUsage::getFile( QString path )
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
  emit clearing();
  propertyMap.clear();
  contentMap.clear();
  if( root )
    delete root;
  root = 0;
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
  if( !dirEntry->directory().isEmpty() )
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

void DiskUsage::rightClickMenu( File *fileItem )
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
    exclude( fileItem );
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
    }
  
    if( item->isDir() )
      calculatePercents( emitSig, dynamic_cast<Directory *>( item ) );
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

#include "diskusage.moc"
