/***************************************************************************
                         packjob.cpp  -  description
                             -------------------
    copyright            : (C) 2009 + by Csaba Karai
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

#include "packjob.h"
#include "krarchandler.h"
#include <qtimer.h>
#include <qdir.h>
#include <klocale.h>
#include <kmimetype.h>

PackJob::PackJob( const KUrl &srcUrl, const KUrl &destUrl, const QStringList & fileNames, const QString &type, const QMap<QString, QString> &packProps ) : AbstractThreadedJob()
{
  start( new PackThread( srcUrl, destUrl, fileNames, type, packProps ) );
}

PackJob * PackJob::createPacker( const KUrl &srcUrl, const KUrl &destUrl, const QStringList & fileNames, const QString &type, const QMap<QString, QString> &packProps )
{
  return new PackJob( srcUrl, destUrl, fileNames, type, packProps );
}

PackThread::PackThread( const KUrl &srcUrl, const KUrl &destUrl, const QStringList & fileNames,
        const QString &type, const QMap<QString, QString> &packProps ) : 
    AbstractJobThread(), _sourceUrl( srcUrl ), _destUrl( destUrl ), _fileNames( fileNames ), 
        _type( type ), _packProperties( packProps )
{
}


void PackThread::slotStart()
{
  KUrl newSource = downloadIfRemote( _sourceUrl, _fileNames );
  if( newSource.isEmpty() )
    return;

  KIO::filesize_t totalSize = 0;
  unsigned long totalDirs = 0, totalFiles = 0;

  calcSpaceLocal( newSource, _fileNames, totalSize, totalDirs, totalFiles );

  QString arcFile = tempFileIfRemote( _destUrl, _type );
  QString arcDir = newSource.path( KUrl::RemoveTrailingSlash );

  setProgressTitle( i18n("Processed files" ) );

  QString save = QDir::currentPath();
  QDir::setCurrent( arcDir );
  bool result = KRarcHandler::pack( _fileNames, _type, arcFile, totalFiles + totalDirs, _packProperties, observer() );
  QDir::setCurrent( save );

  if( isExited() )
    return;
  if( !result )
  {
    sendError( KIO::ERR_INTERNAL, i18n( "Error at packing" ) );
    return;
  }

  if( !uploadTempFiles() )
    return;

  sendSuccess();
}

TestArchiveJob::TestArchiveJob( const KUrl &srcUrl, const QStringList & fileNames ) : AbstractThreadedJob()
{
  start( new TestArchiveThread( srcUrl, fileNames ) );
}

TestArchiveJob * TestArchiveJob::testArchives( const KUrl &srcUrl, const QStringList & fileNames )
{
  return new TestArchiveJob( srcUrl, fileNames );
}

TestArchiveThread::TestArchiveThread( const KUrl &srcUrl, const QStringList & fileNames ) : AbstractJobThread(),
   _sourceUrl( srcUrl ), _fileNames( fileNames )
{
}

void TestArchiveThread::slotStart()
{
  KUrl newSource = downloadIfRemote( _sourceUrl, _fileNames );
  if( newSource.isEmpty() )
    return;

  for ( int i = 0; i < _fileNames.count(); ++i ) {
    QString arcName = _fileNames[ i ];
    if ( arcName.isEmpty() )
      continue;
    if ( arcName == ".." )
      continue; // safety

    KUrl url = newSource;
    url.addPath( arcName );

    QString path = url.path( KUrl::RemoveTrailingSlash );

    KMimeType::Ptr mt = KMimeType::findByUrl( url );
    QString mime = mt ? mt->name() : QString();
    bool encrypted = false;
    QString type = KRarcHandler::getType( encrypted, path, mime );

    // check we that archive is supported
    if ( !KRarcHandler::arcSupported( type ) ) {
      sendError( KIO::ERR_NO_CONTENT, i18n( "%1, unsupported archive type.", arcName ) );
      return ;
    }

    QString password = encrypted ? getPassword( path ) : QString();

    // test the archive
    if ( !KRarcHandler::test( path, type, password, 0, observer() ) ) {
      sendError( KIO::ERR_NO_CONTENT, i18n( "%1, test failed!", arcName ) );
      return ;
    }
  }

  sendMessage( i18n( "Archive tests passed." ) );
  sendSuccess();
}


UnpackJob::UnpackJob( const KUrl &srcUrl, const KUrl &destUrl, const QStringList & fileNames ) : AbstractThreadedJob()
{
  start( new UnpackThread( srcUrl, destUrl, fileNames ) );
}

UnpackJob * UnpackJob::createUnpacker( const KUrl &srcUrl, const KUrl &destUrl, const QStringList & fileNames )
{
  return new UnpackJob( srcUrl, destUrl, fileNames );
}

UnpackThread::UnpackThread( const KUrl &srcUrl, const KUrl &destUrl, const QStringList & fileNames ) : 
    AbstractJobThread(), _sourceUrl( srcUrl ), _destUrl( destUrl ), _fileNames( fileNames )
{
}

void UnpackThread::slotStart()
{
  KUrl newSource = downloadIfRemote( _sourceUrl, _fileNames );
  if( newSource.isEmpty() )
    return;

  QString localDest = tempDirIfRemote( _destUrl );

  for ( int i = 0; i < _fileNames.count(); ++i ) {
    QString arcName = _fileNames[ i ];
    if ( arcName.isEmpty() )
      continue;
    if ( arcName == ".." )
      continue; // safety

    KUrl url = newSource;
    url.addPath( arcName );

    QString path = url.path( KUrl::RemoveTrailingSlash );

    KMimeType::Ptr mt = KMimeType::findByUrl( url );
    QString mime = mt ? mt->name() : QString();
    bool encrypted = false;
    QString type = KRarcHandler::getType( encrypted, path, mime );

    // check we that archive is supported
    if ( !KRarcHandler::arcSupported( type ) ) {
      sendError( KIO::ERR_NO_CONTENT, i18n( "%1, unsupported archive type.", arcName ) );
      return ;
    }

    QString password = encrypted ? getPassword( path ) : QString();

    setProgressTitle( i18n("Processed files" ) );
    // unpack the files
    bool result = KRarcHandler::unpack( path, type, password, localDest, observer() );

    if( isExited() )
      return;
    if( !result )
    {
      sendError( KIO::ERR_INTERNAL, i18n( "Error at unpacking" ) );
      return;
    }
  }

  if( !uploadTempFiles() )
    return;

  sendSuccess();
}
