/***************************************************************************
                        synchronizer.cpp  -  description
                             -------------------
    copyright            : (C) 2003 by Csaba Karai
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

#include "synchronizer.h"
#include "../VFS/ftp_vfs.h"
#include "../VFS/normal_vfs.h"
#include "../VFS/vfile.h"
#include "../krusader.h"
#include "../krservices.h"
#include <kurl.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qapplication.h>
#include <qregexp.h>
#include <qdir.h>
#include <kio/job.h>
#include <kdialogbase.h>
#include <kio/observer.h>
#include <kio/renamedlg.h>
#include <kio/skipdlg.h>
#include <unistd.h>
#include <qeventloop.h>
#include <qprogressdialog.h>

#define  DISPLAY_UPDATE_PERIOD        2

Synchronizer::Synchronizer() : displayUpdateCount( 0 ), markEquals( true ), markDiffers ( true ),
                               markCopyToLeft( true ), markCopyToRight( true ), markDeletable( true )
{
  resultList.setAutoDelete( true );
}

void Synchronizer::reset()
{
  displayUpdateCount = 0;
  markEquals = markDiffers = markCopyToLeft = markCopyToRight = markDeletable = true;
  leftURL = rightURL = KURL();
  compareFinished = compareResult = statusLineChanged = errorPrinted = stopped = false;
  recurseSubDirs = followSymLinks = ignoreDate = asymmetric = cmpByContent = autoScroll = false;
  markEquals = markDiffers = markCopyToLeft = markCopyToRight = markDeletable = markDuplicates = markSingles = false;
  leftCopyEnabled = rightCopyEnabled = deleteEnabled = overWrite = autoSkip = paused = false;
  leftCopyNr = rightCopyNr = deleteNr = 0;
  leftCopySize = rightCopySize = deleteSize = 0;
  scannedDirs = fileCount = 0;
  leftBaseDir = rightBaseDir = QString::null;
  inclusionFilter.clear();
  exclusionFilter.clear();
  resultList.clear();
}

int Synchronizer::compare( QString leftURL, QString rightURL, QString filter, bool subDirs,
                            bool symLinks, bool igDate, bool asymm, bool cmpByCnt, bool autoSc )
{
  resultList.clear();

  recurseSubDirs = subDirs;
  followSymLinks = symLinks;
  ignoreDate     = igDate;
  asymmetric     = asymm;
  cmpByContent   = cmpByCnt;
  autoScroll     = autoSc;
  stopped = false;

  int exclusionIndex = filter.find('|');
  if( exclusionIndex > -1 )
  {
    QString inclusionExp = filter.left( exclusionIndex );
    QString exclusionExp = filter.mid( exclusionIndex+1 );
    
    inclusionFilter = QStringList::split(" ",inclusionExp);
    exclusionFilter = QStringList::split(" ",exclusionExp);
    
    if( inclusionFilter.count() == 0 )
      inclusionFilter.push_back( QString("*") );
  }
  else
    inclusionFilter = QStringList::split(" ",filter);
  
  if( !leftURL.endsWith("/" ))  leftURL+="/";
  if( !rightURL.endsWith("/" )) rightURL+="/";

  scannedDirs = fileCount = 0;
  
  compareDirectory( 0, leftBaseDir = leftURL, rightBaseDir = rightURL, "" );

  emit statusInfo( i18n( "File number:%1" ).arg( fileCount ) );
  return fileCount;
}

void Synchronizer::compareDirectory( SynchronizerFileItem *parent, QString leftURL,
                                     QString rightURL, QString dir, QString addName,
                                     QString addDir, time_t addLTime, time_t addRTime )
{
  vfs   * left_directory  = getDirectory( leftURL );
  vfs   * right_directory = getDirectory( rightURL );
  vfile * left_file;
  vfile * right_file;
  QString file_name;

  if( left_directory == 0 || right_directory == 0 )
  {
    if( left_directory )
      delete left_directory;
    if( right_directory )
      delete right_directory;    
    return;
  }

  if( !dir.isEmpty() )
    parent = addDuplicateItem( parent, addName, addDir, 0, 0, addLTime, addRTime, true, !checkName( addName ) );

  /* walking through in the left directory */  
  for( left_file=left_directory->vfs_getFirstFile(); left_file != 0 && !stopped ;
       left_file=left_directory->vfs_getNextFile() )
  {
    if ( left_file->vfile_isDir() || left_file->vfile_isSymLink() )
      continue;

    file_name =  left_file->vfile_getName();

    if( !checkName( file_name ) )
      continue;

    if( (right_file = right_directory->vfs_search( file_name )) == 0 )
      addLeftOnlyItem( parent, file_name, dir, left_file->vfile_getSize(), left_file->vfile_getTime_t() );
    else
    {
      if( right_file->vfile_isDir() || right_file->vfile_isSymLink() )
        continue;

      addDuplicateItem( parent, file_name, dir, left_file->vfile_getSize(), right_file->vfile_getSize(),
                        left_file->vfile_getTime_t(), right_file->vfile_getTime_t() );
    }
  }

  /* walking through in the right directory */
  for( right_file=right_directory->vfs_getFirstFile(); right_file != 0 && !stopped ;
       right_file=right_directory->vfs_getNextFile() )
  {
    if( right_file->vfile_isDir() || right_file->vfile_isSymLink() )
      continue;

    file_name =  right_file->vfile_getName();

    if( !checkName( file_name ) )
      continue;
    
    if( left_directory->vfs_search( file_name ) == 0 )
      addRightOnlyItem( parent, file_name, dir, right_file->vfile_getSize(), right_file->vfile_getTime_t() );
  }

  /* walking through the subdirectories */
  if( recurseSubDirs )
  {
    for( left_file=left_directory->vfs_getFirstFile(); left_file != 0 && !stopped ;
         left_file=left_directory->vfs_getNextFile() )
    {
      if ( left_file->vfile_isDir() && ( followSymLinks || !left_file->vfile_isSymLink()) )
      {
        file_name =  left_file->vfile_getName();
        
        if( (right_file = right_directory->vfs_search( file_name )) == 0 )
          addSingleDirectory( parent, file_name, dir, left_file->vfile_getTime_t(), true );
        else
          compareDirectory( parent, leftURL+file_name+"/", rightURL+file_name+"/",
                            dir.isEmpty() ? file_name : dir+"/"+file_name, file_name,
                            dir, left_file->vfile_getTime_t(), right_file->vfile_getTime_t() );
      }
    }

    /* walking through the the right side subdirectories */
    for( right_file=right_directory->vfs_getFirstFile(); right_file != 0 && !stopped ;
         right_file=right_directory->vfs_getNextFile() )
    {
      if ( right_file->vfile_isDir() && (followSymLinks || !right_file->vfile_isSymLink()) )
      {
        file_name =  right_file->vfile_getName();

        if( left_directory->vfs_search( file_name ) == 0 )
          addSingleDirectory( parent, file_name, dir, right_file->vfile_getTime_t(), false );
      }
    }
  }

  if( parent && parent->isTemporary() )
    delete parent;
    
  delete left_directory;
  delete right_directory;
}

vfs * Synchronizer::getDirectory( QString url )
{
  if( stopped )
    return 0;
    
  vfs *v;
  
  if( url.startsWith( "/" ) || url.startsWith( "file:/" ))   /* is it normal vfs? */
  {
    v = new normal_vfs(0);
  }
  else                          /* ftp vfs */
  {
    v = new ftp_vfs(0);
  }

  bool result = v->vfs_refresh( vfs::fromPathOrURL( url ) );

  if ( !result )
  {
    KMessageBox::error(0, i18n("Error at opening URL:%1!").arg( url ));
    delete v;
    return 0;
  }

  emit statusInfo( i18n( "Scanned directories:%1" ).arg( ++scannedDirs ) );
  
  return v;
}

QString Synchronizer::getTaskTypeName( TaskType taskType )
{
  static QString names[] = {"=","!=","<-","->","DEL"};

  return names[taskType];
}

SynchronizerFileItem * Synchronizer::addItem( SynchronizerFileItem *parent, QString file_name,
                                  QString dir, bool existsLeft, bool existsRight,
                                  KIO::filesize_t leftSize, KIO::filesize_t rightSize,
                                  time_t leftDate, time_t rightDate, TaskType tsk,
                                  bool isDir, bool isTemp )
{
  bool marked = autoScroll ? isMarked( tsk, existsLeft && existsRight ) : false;
  SynchronizerFileItem *item = new SynchronizerFileItem( file_name, dir, marked, 
    existsLeft, existsRight, leftSize, rightSize, leftDate, rightDate, tsk, isDir,
    isTemp, parent );


  if( marked )
  {
    if( markParentDirectories( item ) && autoScroll )
    {
      int oldFileCount = fileCount;
      refresh( true );
      fileCount = oldFileCount;
    }
    fileCount++;
  }

  if( !isTemp )
  {     
    while( parent && parent->isTemporary() )
       setPermanent( parent );

    resultList.append( item );
    emit comparedFileData( item );

    if( marked && (displayUpdateCount++ % DISPLAY_UPDATE_PERIOD == (DISPLAY_UPDATE_PERIOD-1) ) )
      qApp->processEvents();
  }
    
  return item;
}

void Synchronizer::setPermanent( SynchronizerFileItem *item )
{
  if( item->parent() && item->parent()->isTemporary() )
    setPermanent( item->parent() );

  item->setPermanent();
  resultList.append( item );
  emit comparedFileData( item );
}

SynchronizerFileItem * Synchronizer::addLeftOnlyItem( SynchronizerFileItem *parent, QString file_name,
                                    QString dir, KIO::filesize_t size, time_t date, bool isDir, bool isTemp )
{
  return addItem( parent, file_name, dir, true, false, size, 0, date, 0,
                  asymmetric ? TT_DELETE : TT_COPY_TO_RIGHT, isDir, isTemp );
}

SynchronizerFileItem * Synchronizer::addRightOnlyItem( SynchronizerFileItem *parent, QString file_name,
                                    QString dir, KIO::filesize_t size, time_t date, bool isDir, bool isTemp )
{
  return addItem( parent, file_name, dir, false, true, 0, size, 0, date, TT_COPY_TO_LEFT, isDir, isTemp );
}

SynchronizerFileItem * Synchronizer::addDuplicateItem( SynchronizerFileItem *parent, QString file_name,
                                     QString dir, KIO::filesize_t leftSize,
                                     KIO::filesize_t rightSize, time_t leftDate, time_t rightDate,
                                     bool isDir, bool isTemp )
{
  TaskType        task;

  do
  {
    if( isDir )
    {
      task = TT_EQUALS;
      break;
    }
    if( leftSize == rightSize )
    {
      if( cmpByContent && compareByContent(file_name, dir) )
      {
        task = TT_EQUALS;
        break;
      }
      if ( !cmpByContent && ( ignoreDate || leftDate == rightDate) )
      {
        task = TT_EQUALS;
        break;
      }
    }

    if( asymmetric )
      task = TT_COPY_TO_LEFT;
    else if( ignoreDate )
      task = TT_DIFFERS;
    else if( leftDate > rightDate )
      task = TT_COPY_TO_RIGHT;
    else if( leftDate < rightDate )
      task = TT_COPY_TO_LEFT;
    else
      task = TT_DIFFERS;

  }while( false );
  
  return addItem( parent, file_name, dir, true, true, leftSize, rightSize, leftDate, rightDate, task, isDir, isTemp );
}

void Synchronizer::addSingleDirectory( SynchronizerFileItem *parent, QString name,
                                       QString dir , time_t date, bool isLeft )
{
  QString baseDir = (isLeft ? leftBaseDir : rightBaseDir );
  QString dirName = dir.isEmpty() ? name : dir+"/"+name;
  
  vfs   * directory  = getDirectory( baseDir+ dirName );
  vfile * file;
  QString file_name;

  if( directory == 0 )
    return;

  if( isLeft )
    parent = addLeftOnlyItem( parent, name, dir, 0, date, true, !checkName( name ) );
  else
    parent = addRightOnlyItem( parent, name, dir, 0, date, true, !checkName( name ) );

  /* walking through the directory files */
  for( file=directory->vfs_getFirstFile(); file != 0 && !stopped; file = directory->vfs_getNextFile() )
  {
    if ( file->vfile_isDir() || file->vfile_isSymLink() )
      continue;

    file_name =  file->vfile_getName();

    if( !checkName( file_name ) )
      continue;

    if( isLeft )
      addLeftOnlyItem( parent, file_name, dirName, file->vfile_getSize(), file->vfile_getTime_t() );
    else
      addRightOnlyItem( parent, file_name, dirName, file->vfile_getSize(), file->vfile_getTime_t() );
  }

  /* walking through the subdirectories */
  for( file=directory->vfs_getFirstFile(); file != 0 && !stopped; file=directory->vfs_getNextFile() )
  {
    if ( file->vfile_isDir() && (followSymLinks || !file->vfile_isSymLink())  )
    {
        file_name =  file->vfile_getName();
        addSingleDirectory( parent, file_name, dirName, file->vfile_getTime_t(), isLeft );
    }
  }

  if( parent && parent->isTemporary() )
    delete parent;
  
  delete directory;
}

bool Synchronizer::checkName( QString name )
{  
  for ( QStringList::Iterator it = inclusionFilter.begin(); it != inclusionFilter.end(); ++it )
    if( QRegExp(*it,true,true).exactMatch( name ) )
    {
      for ( QStringList::Iterator it2 = exclusionFilter.begin(); it2 != exclusionFilter.end(); ++it2 )
        if( QRegExp(*it2,true,true).exactMatch( name ) )
          return false;
      
      return true;
    }
  return false;
}

void Synchronizer::setMarkFlags( bool left, bool equal, bool differs, bool right, bool dup, bool sing,
                                 bool del )
{
  markEquals      = equal;
  markDiffers     = differs;
  markCopyToLeft  = left;
  markCopyToRight = right;
  markDeletable   = del;
  markDuplicates  = dup;
  markSingles     = sing;
}

bool Synchronizer::isMarked( TaskType task, bool isDuplicate )
{
  if( (isDuplicate && !markDuplicates) || (!isDuplicate && !markSingles) )
    return false;
  
  switch( task )
  {
  case TT_EQUALS:
    return markEquals;
  case TT_DIFFERS:
    return markDiffers;
  case TT_COPY_TO_LEFT:
    return markCopyToLeft;
  case TT_COPY_TO_RIGHT:
    return markCopyToRight;
  case TT_DELETE:
    return markDeletable;
  }
  return false;
}

bool Synchronizer::markParentDirectories( SynchronizerFileItem *item )
{
  if( item->parent() == 0 || item->parent()->isMarked() ) 
    return false;

  markParentDirectories( item->parent() );

  item->parent()->setMarked( true );

  fileCount++;
  emit markChanged( item->parent() );
  return true;
}

int Synchronizer::refresh(bool nostatus)
{
  fileCount = 0;
  
  SynchronizerFileItem *item = resultList.first();

  while( item )
  {
    bool marked = isMarked( item->task(), item->existsInLeft() && item->existsInRight() );
    item->setMarked( marked );

    if( marked )
    {
      markParentDirectories( item );
      fileCount++;
    }

    item = resultList.next();
  }

  item = resultList.first();
  while( item )
  {
    emit markChanged( item );
    item = resultList.next();
  }
  
  if( !nostatus )
    emit statusInfo( i18n( "File number:%1" ).arg( fileCount ) );

  return fileCount;
}

void Synchronizer::operate( SynchronizerFileItem *item,
                            void (*executeOperation)(SynchronizerFileItem *) )
{
  executeOperation( item );

  if( item->isDir() )
  {
    QString dirName = ( item->directory() == "" ) ?
                        item->name() : item->directory() + "/" + item->name() ;

    item = resultList.first();
    while( item )
    {
      if( item->directory() == dirName || item->directory().startsWith( dirName + "/" ) )
        executeOperation( item );
      
      item = resultList.next();
    }    
  }
}

void Synchronizer::excludeOperation( SynchronizerFileItem *item )
{
  item->setTask( TT_DIFFERS );
}

void Synchronizer::exclude( SynchronizerFileItem *item )
{
  if( !item->parent() || item->parent()->task() != TT_DELETE )
    operate( item, excludeOperation );  /* exclude only if the parent task is not DEL */
}

void Synchronizer::restoreOperation( SynchronizerFileItem *item )
{
  item->restoreOriginalTask();
}

void Synchronizer::restore( SynchronizerFileItem *item )
{
  operate( item, restoreOperation );

  while( ( item = item->parent() ) != 0 ) /* in case of restore, the parent directories */
  {                                          /* must be changed for being consistent */
    if( item->task() != TT_DIFFERS )  
      break;

    if( item->originalTask() == TT_DELETE ) /* if the parent original task is delete */
      break;                                    /* don't touch it */
      
    item->restoreOriginalTask();          /* restore */
  }     
}

void Synchronizer::reverseDirectionOperation( SynchronizerFileItem *item )
{
  if( item->existsInRight() && item->existsInLeft() )
  {
    if( item->task() == TT_COPY_TO_LEFT )
      item->setTask( TT_COPY_TO_RIGHT );
    else if( item->task() == TT_COPY_TO_RIGHT )
      item->setTask( TT_COPY_TO_LEFT );
  }  
}

void Synchronizer::reverseDirection( SynchronizerFileItem *item )
{
  operate( item, reverseDirectionOperation );
}

void Synchronizer::deleteLeftOperation( SynchronizerFileItem *item )
{
  if( !item->existsInRight() && item->existsInLeft() )
    item->setTask( TT_DELETE );
}

void Synchronizer::deleteLeft( SynchronizerFileItem *item )
{
  operate( item, deleteLeftOperation );
}

void Synchronizer::copyToLeftOperation( SynchronizerFileItem *item )
{
  if( item->existsInRight() )
  {
    if( !item->isDir() )
      item->setTask( TT_COPY_TO_LEFT );
    else
    {
      if( item->existsInLeft() && item->existsInRight() )
        item->setTask( TT_EQUALS );
      else if( !item->existsInLeft() && item->existsInRight() )
        item->setTask( TT_COPY_TO_LEFT );
    }
  }
}

void Synchronizer::copyToLeft( SynchronizerFileItem *item )
{
  operate( item, copyToLeftOperation );

  while( ( item = item->parent() ) != 0 ) 
  {                                       
    if( item->task() != TT_DIFFERS )
      break;

    if( item->existsInLeft() && item->existsInRight() )
      item->setTask( TT_EQUALS );
    else if( !item->existsInLeft() && item->existsInRight() )
      item->setTask( TT_COPY_TO_LEFT );
  }
}

void Synchronizer::copyToRightOperation( SynchronizerFileItem *item )
{
  if( item->existsInLeft() )
  {
    if( !item->isDir() )
      item->setTask( TT_COPY_TO_RIGHT );
    else
    {
      if( item->existsInLeft() && item->existsInRight() )
        item->setTask( TT_EQUALS );
      else if( item->existsInLeft() && !item->existsInRight() )
        item->setTask( TT_COPY_TO_RIGHT );
    }
  }
}

void Synchronizer::copyToRight( SynchronizerFileItem *item )
{
  operate( item, copyToRightOperation );

  while( ( item = item->parent() ) != 0 )
  {
    if( item->task() != TT_DIFFERS && item->task() != TT_DELETE )
      break;

    if( item->existsInLeft() && item->existsInRight() )
      item->setTask( TT_EQUALS );
    else if( item->existsInLeft() && !item->existsInRight() )
      item->setTask( TT_COPY_TO_RIGHT );
  }
}

bool Synchronizer::totalSizes( int * leftCopyNr, KIO::filesize_t *leftCopySize, int * rightCopyNr,
                               KIO::filesize_t *rightCopySize, int *deleteNr, KIO::filesize_t *deletableSize )
{
  bool hasAnythingToDo = false;
  
  *leftCopySize = *rightCopySize = *deletableSize = 0;
  *leftCopyNr = *rightCopyNr = *deleteNr = 0;
  
  SynchronizerFileItem *item = resultList.first();

  while( item )
  {
    if( item->isMarked() )
    {
      switch( item->task() )
      {
      case TT_COPY_TO_LEFT:
        *leftCopySize += item->rightSize();
        (*leftCopyNr)++;
        hasAnythingToDo = true;
        break;
      case TT_COPY_TO_RIGHT:
        *rightCopySize += item->leftSize();
        (*rightCopyNr)++;
        hasAnythingToDo = true;
        break;
      case TT_DELETE:
        *deletableSize += item->leftSize();
        (*deleteNr)++;
        hasAnythingToDo = true;
        break;
      default:
        break;
      }
    }
    item = resultList.next();
  }

  return hasAnythingToDo;
}

void Synchronizer::swapSides()
{
  QString leftTmp = leftBaseDir;
  leftBaseDir = rightBaseDir;
  rightBaseDir = leftTmp;

  KURL leftTmpURL = leftURL;
  leftURL = rightURL;
  rightURL = leftTmpURL;
  
  SynchronizerFileItem *item = resultList.first();

  while( item )
  {
    item->swap( asymmetric );    
    item = resultList.next();
  }
}

void Synchronizer::synchronize( bool leftCopyEnabled, bool rightCopyEnabled, bool deleteEnabled,
                                bool overWrite )
{
    this->leftCopyEnabled   = leftCopyEnabled;
    this->rightCopyEnabled  = rightCopyEnabled;
    this->deleteEnabled     = deleteEnabled;
    this->overWrite         = overWrite;

    autoSkip = paused = false;

    leftCopyNr = rightCopyNr = deleteNr = 0;
    leftCopySize = rightCopySize = deleteSize = 0;

    currentTask = resultList.first();
    executeTask();
}

void Synchronizer::executeTask()
{
  TaskType task;
  
  do {
    if( currentTask == 0 )
    {
      emit synchronizationFinished();
      return;
    }
    
    if( currentTask->isMarked() )
    {
      task = currentTask->task();
      
      if( leftCopyEnabled && task == TT_COPY_TO_LEFT )
        break;
      else if( rightCopyEnabled && task == TT_COPY_TO_RIGHT )
        break;
      else if( deleteEnabled && task == TT_DELETE )
        break;
    }

    currentTask = resultList.next();
  }while( true );

  QString dirName = currentTask->directory();
  if( !dirName.isEmpty() )
    dirName += "/";
  
  leftURL  = vfs::fromPathOrURL( leftBaseDir + dirName + currentTask->name() );
  rightURL = vfs::fromPathOrURL( rightBaseDir + dirName + currentTask->name() );
  
  switch( task )
  {
  case TT_COPY_TO_LEFT:
    if( currentTask->isDir() )
    {
      KIO::SimpleJob *job = KIO::mkdir( leftURL );
      connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(slotTaskFinished(KIO::Job*)));
    }
    else
    {
      KURL destURL( leftURL );
      if( !currentTask->destination().isNull() )
        destURL = vfs::fromPathOrURL( currentTask->destination() );
      
      KIO::FileCopyJob *job = KIO::file_copy(rightURL, destURL, -1,
                                overWrite || currentTask->overWrite(), false, false );
      connect(job,SIGNAL(processedSize (KIO::Job *, KIO::filesize_t )), this,
                  SLOT  (slotProcessedSize (KIO::Job *, KIO::filesize_t )));
      connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(slotTaskFinished(KIO::Job*)));
    }
    break;
  case TT_COPY_TO_RIGHT:
    if( currentTask->isDir() )
    {
      KIO::SimpleJob *job = KIO::mkdir( rightURL );
      connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(slotTaskFinished(KIO::Job*)));
    }
    else
    {
      KURL destURL( rightURL );
      if( !currentTask->destination().isNull() )
        destURL = vfs::fromPathOrURL( currentTask->destination() );

      KIO::FileCopyJob *job = KIO::file_copy(leftURL, destURL, -1,
                                overWrite || currentTask->overWrite(), false, false );
      connect(job,SIGNAL(processedSize (KIO::Job *, KIO::filesize_t )), this,
                  SLOT  (slotProcessedSize (KIO::Job *, KIO::filesize_t )));
      connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(slotTaskFinished(KIO::Job*)));
    }
    break;
  case TT_DELETE:
    {
      KIO::DeleteJob *job = KIO::del( leftURL, false );
      connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(slotTaskFinished(KIO::Job*)));
    }
    break;
  default:
    break;
  }
}

void Synchronizer::slotTaskFinished(KIO::Job *job )
{
  do {
    if( job->error() )
    {
      if( job->error() == KIO::ERR_FILE_ALREADY_EXIST && currentTask->task() != TT_DELETE )
      {
        KIO::RenameDlg_Result result;
        QString newDest;

        if( autoSkip )
          break;  
      
        if ( currentTask->task() == TT_COPY_TO_LEFT )
        {
          result = Observer::self()->open_RenameDlg ( job, i18n("File Already Exists"),
            rightURL.prettyURL(0,KURL::StripFileProtocol), leftURL.prettyURL(0,KURL::StripFileProtocol),
            (KIO::RenameDlg_Mode)( KIO::M_OVERWRITE | KIO::M_SKIP | KIO::M_MULTI ), newDest,
            currentTask->rightSize(), currentTask->leftSize(), (time_t)-1, (time_t)-1,
            currentTask->rightDate(), currentTask->leftDate());
        }
        else
        {
          result = Observer::self()->open_RenameDlg ( job, i18n("File Already Exists"),
            leftURL.prettyURL(0,KURL::StripFileProtocol), rightURL.prettyURL(0,KURL::StripFileProtocol),
            (KIO::RenameDlg_Mode)( KIO::M_OVERWRITE | KIO::M_SKIP | KIO::M_MULTI ), newDest,
            currentTask->leftSize(), currentTask->rightSize(), (time_t)-1, (time_t)-1,
            currentTask->leftDate(), currentTask->rightDate());          
        }

        switch ( result )
        {
        case KIO::R_RENAME:
          currentTask->setDestination( newDest );
          executeTask();
          return;
        case KIO::R_OVERWRITE:
          currentTask->setOverWrite();
          executeTask();
          return;
        case KIO::R_OVERWRITE_ALL:
          overWrite = true;
          executeTask();
          return;
        case KIO::R_AUTO_SKIP:
          autoSkip = true;
        case KIO::R_SKIP:
        default:
          break;
        }
        break;
      }

      if( job->error() != KIO::ERR_DOES_NOT_EXIST || currentTask->task() != TT_DELETE )
      {
        if( autoSkip )
          break;
          
        QString error;

        switch( currentTask->task() )
        {
        case TT_COPY_TO_LEFT:
          error = i18n("Error at copying file %1 to %2!")
                       .arg( rightURL.prettyURL(0,KURL::StripFileProtocol) )
                       .arg( leftURL .prettyURL(0,KURL::StripFileProtocol) );
          break;
        case TT_COPY_TO_RIGHT:
          error = i18n("Error at copying file %1 to %2!")
                       .arg( leftURL.prettyURL(0,KURL::StripFileProtocol) )
                       .arg( rightURL .prettyURL(0,KURL::StripFileProtocol) );
          break;
        case TT_DELETE:
          error = i18n("Error at deleting file %1!").arg( leftURL.prettyURL(0,KURL::StripFileProtocol) );
          break;
        default:
          break;
        }

        KIO::SkipDlg_Result result = Observer::self()->open_SkipDlg( job, true, error );

        switch( result )
        {
        case KIO::S_CANCEL:
          executeTask();  /* simply retry */
          return;
        case KIO::S_AUTO_SKIP:
          autoSkip = true;
        default:
          break;
        }
      }
    }
  }while( false );

  switch( currentTask->task() )
  {
  case TT_COPY_TO_LEFT:
    leftCopyNr++;
    leftCopySize += currentTask->rightSize();
    break;
  case TT_COPY_TO_RIGHT:
    rightCopyNr++;
    rightCopySize += currentTask->leftSize();
    break;
  case TT_DELETE:
    deleteNr++;
    deleteSize += currentTask->leftSize();
    break;
  default:
    break;
  }

  emit processedSizes( leftCopyNr, leftCopySize, rightCopyNr, rightCopySize, deleteNr, deleteSize );
    
  currentTask = resultList.next();

  if( paused )
    emit pauseAccepted();
  else
    executeTask();
}

void Synchronizer::slotProcessedSize( KIO::Job * , KIO::filesize_t size)
{
  KIO::filesize_t dl = 0, dr = 0, dd = 0;

  switch( currentTask->task() )
  {
  case TT_COPY_TO_LEFT:
    dl = size;
    break;
  case TT_COPY_TO_RIGHT:
    dr = size;
    break;
  case TT_DELETE:
    dd = size;
    break;
  default:
    break;
  }
  
  emit processedSizes( leftCopyNr, leftCopySize+dl, rightCopyNr, rightCopySize+dr, deleteNr, deleteSize+dd );
}

void Synchronizer::pause()
{
  paused = true;
}

void Synchronizer::resume()
{
  paused = false;
  executeTask();
}

QString Synchronizer::leftBaseDirectory()
{
  return leftBaseDir;
}

QString Synchronizer::rightBaseDirectory()
{
  return rightBaseDir;
}

bool Synchronizer::compareByContent( QString file_name, QString dir )
{
  if( !dir.isEmpty() )
    dir += "/";

  leftURL   = vfs::fromPathOrURL( leftBaseDir + dir + file_name );
  rightURL  = vfs::fromPathOrURL( rightBaseDir + dir + file_name );

  leftReadJob = KIO::get( leftURL, false, false );
  rightReadJob = KIO::get( rightURL, false, false );

  connect(leftReadJob, SIGNAL(data(KIO::Job *, const QByteArray &)),
            this, SLOT(slotDataReceived(KIO::Job *, const QByteArray &)));
  connect(rightReadJob, SIGNAL(data(KIO::Job *, const QByteArray &)),
            this, SLOT(slotDataReceived(KIO::Job *, const QByteArray &)));
  connect(leftReadJob, SIGNAL(result(KIO::Job*)),
            this, SLOT(slotFinished(KIO::Job *)));
  connect(rightReadJob, SIGNAL(result(KIO::Job*)),
            this, SLOT(slotFinished(KIO::Job *)));

  compareArray = QByteArray();
  compareFinished = errorPrinted = statusLineChanged = false;
  compareResult = true;
  waitWindow = 0;

  rightReadJob->suspend();

  timer = new QTimer( this );
  connect( timer, SIGNAL(timeout()), SLOT(putWaitWindow()) );
  timer->start( 1500, true );
   
  while( !compareFinished )
    qApp->processEvents();

  timer->stop();
  delete timer;

  if( waitWindow )
    delete waitWindow;

  if( statusLineChanged )
    emit statusInfo( i18n( "Scanned directories:%1" ).arg( scannedDirs ) );
                                                                                                        
  return compareResult;
}

void Synchronizer::slotDataReceived(KIO::Job *job, const QByteArray &data)
{
  int bufferLen = compareArray.size();
  int dataLen   = data.size();

  do
  {
    if( bufferLen == 0 )
    {
      compareArray.duplicate( data.data(), dataLen );
      break;
    }

    int minSize   = ( dataLen < bufferLen ) ? dataLen : bufferLen;

    for( int i = 0; i != minSize; i++ )
      if( data[i] != compareArray[i] )
      {
        abortContentComparing();
        return;
      }

    if( minSize == bufferLen )
    {
      compareArray.duplicate( data.data() + bufferLen, dataLen - bufferLen );
      if( dataLen == bufferLen )
        return;
      break;
    }
    else
    {
      compareArray.duplicate( compareArray.data() + dataLen, bufferLen - dataLen );
      return;
    }

  }while ( false );

  KIO::TransferJob *otherJob = ( job == leftReadJob ) ? rightReadJob : leftReadJob;
  
  if( otherJob == 0 )
  {
    if( compareArray.size() )
      abortContentComparing();
  }
  else
  {    
    ((KIO::TransferJob *)job)->suspend();
    otherJob->resume();
  }
}

void Synchronizer::slotFinished(KIO::Job *job)
{
  KIO::TransferJob *otherJob = ( job == leftReadJob ) ? rightReadJob : leftReadJob;
  
  if( job == leftReadJob )
    leftReadJob = 0;
  else
    rightReadJob = 0;

  if( otherJob )
    otherJob->resume();

  if( job->error() && job->error() != KIO::ERR_USER_CANCELED && !errorPrinted )
  {
    timer->stop();
    errorPrinted = true;
    KMessageBox::error(0, i18n("IO error at comparing file %1 with %2!")
                       .arg( leftURL. prettyURL(0,KURL::StripFileProtocol) )
                       .arg( rightURL.prettyURL(0,KURL::StripFileProtocol) ) );
    abortContentComparing();
  }
  
  if( leftReadJob == 0 && rightReadJob == 0 )
  {
    if( compareArray.size() )
      compareResult = false;
    compareFinished = true;
  }
}

void Synchronizer::abortContentComparing()
{
  if( leftReadJob )
    leftReadJob->kill( false );
  if( rightReadJob )
    rightReadJob->kill( false );

  compareResult = false;
}

void Synchronizer::putWaitWindow()
{
  if( autoScroll )
  {
    waitWindow = new QProgressDialog( 0, "SynchronizerWait", true );
    waitWindow->setLabelText( i18n( "Comparing file %1..." ).arg( leftURL.fileName() ) );
    waitWindow->setTotalSteps( 100 );
    waitWindow->setAutoClose( false );
    waitWindow->show();

    if( leftReadJob )
      connect(leftReadJob, SIGNAL(percent (KIO::Job *, unsigned long)),
              this, SLOT(comparePercent(KIO::Job *, unsigned long)));
  }
  else
  {
    emit statusInfo ( i18n( "Comparing file %1..." ).arg( leftURL.fileName() ) );
    statusLineChanged = true;
  }
}

void Synchronizer::comparePercent(KIO::Job *, unsigned long percent)
{
  waitWindow->setProgress( percent );
}

void Synchronizer::synchronizeWithKGet()
{
  bool isLeftLocal = vfs::fromPathOrURL( leftBaseDirectory() ).isLocalFile();
  QProgressDialog *progDlg = 0;
  int  processedCount = 0, totalCount = 0;
  
  SynchronizerFileItem *item = resultList.first();  
  for(; item; item = resultList.next() )
    if( item->isMarked() )
      totalCount++;
  
  item = resultList.first();
  while( item )
  {
    if( item->isMarked() )
    {
      KURL downloadURL, destURL;
      QString dirName     = item->directory().isEmpty() ? "" : item->directory() + "/";
      QString destDir;
      
      if( progDlg == 0 )
      {
        progDlg = new QProgressDialog( i18n( "Feeding the URL-s to kget" ), i18n("Cancel"), 
                     totalCount, krApp, "Synchronizer Progress Dlg", true );
        progDlg->setCaption( i18n( "Krusader::Synchronizer" ) );
        progDlg->setAutoClose( false );
        progDlg->show();
        qApp->processEvents();
      }
    
      if( item->task() == TT_COPY_TO_RIGHT && !isLeftLocal )
      {
        downloadURL = vfs::fromPathOrURL( leftBaseDirectory() + dirName + item->name() );
        destDir     = rightBaseDirectory() + dirName;
        destURL     = vfs::fromPathOrURL( destDir + item->name() );
      }
      if( item->task() == TT_COPY_TO_LEFT && isLeftLocal )
      {
        downloadURL = vfs::fromPathOrURL( rightBaseDirectory() + dirName + item->name() );
        destDir     = leftBaseDirectory() + dirName;
        destURL     = vfs::fromPathOrURL( destDir + item->name() );
      }
      
      if( item->isDir() )
        destDir += item->name();

      // creating the directory system
      for( int i=0; i >= 0 ; i= destDir.find('/',i+1) )
        if( !QDir( destDir.left(i) ).exists() )
          QDir().mkdir( destDir.left(i) );            
        
      if( !item->isDir() && !downloadURL.isEmpty() )
      {
        if( QFile( destURL.path() ).exists() )
          QFile( destURL.path() ).remove();
        
        QString source = downloadURL.prettyURL();
        if( source.contains( '@' ) >= 2 ) /* is this an ftp proxy URL? */
        {
          int lastAt = source.findRev( '@' );
          QString startString = source.left( lastAt );
          QString endString = source.mid( lastAt );
          startString.replace( "@", "%40" );
          source = startString+endString;
        }
        
        KProcess p;

        p << KrServices::fullPathName( "kget" ) << source << destURL.path();
        if (!p.start(KProcess::Block))
          KMessageBox::error(0,i18n("Error executing ")+KrServices::fullPathName( "kget" )+" !");
        else
          p.detach();
      }
      
      progDlg->setProgress( ++processedCount );
      qApp->processEvents();
      
      if( progDlg->wasCanceled() )
        break;
    }
    item = resultList.next();
  }
  
  if( progDlg )
    delete progDlg;
}
