/***************************************************************************
                                krsearchmod.cpp
                            -------------------
   copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
   email                : krusader@users.sourceforge.net
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

#include "krsearchmod.h"
#include "krquery.h"
#include "../krusader.h"
#include "../resources.h"
#include "../VFS/vfile.h"
#include "../VFS/krpermhandler.h"
#include "../VFS/krarchandler.h"

#include <klocale.h>
#include <qdir.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <klargefile.h>
#include <kurlrequesterdlg.h> 

#include <kmimetype.h>

KRSearchMod::KRSearchMod( const KRQuery* q )
{
  stopSearch = false; /// ===> added
  query = new KRQuery( *q );
  query->normalize();
   
  remote_vfs = 0;
}

KRSearchMod::~KRSearchMod()
{
  delete query;
  if( remote_vfs )
    delete remote_vfs;
}

void KRSearchMod::start()
{
  // search every dir that needs to be searched
  for ( unsigned int i = 0; i < query->whereToSearch.count(); ++i )
      scanURL( query->whereToSearch [ i ] );
  
  emit finished();
}

void KRSearchMod::stop()
{
  stopSearch = true;
}

bool KRSearchMod::checkPerm( QString perm )
{
  QString q_perm = query->perm;
  for ( int i = 0; i < 9; ++i )
    if ( q_perm[ i ] != '?' && q_perm[ i ] != perm[ i + 1 ] ) return false;
  return true;
}

bool KRSearchMod::checkType( QString mime )
{
  QString type = query->type;
  if ( type == mime ) return true;
  if ( type == i18n( "Archives" ) )
    return KRarcHandler::arcSupported( mime.right( 4 ) );
  if ( type == i18n( "Directories" ) ) return mime.contains( "directory" );
  if ( type == i18n( "Image Files" ) ) return mime.contains( "image/" );
  if ( type == i18n( "Text Files" ) ) return mime.contains( "text/" );
  if ( type == i18n( "Video Files" ) ) return mime.contains( "video/" );
  if ( type == i18n( "Audio Files" ) ) return mime.contains( "audio/" );
  if ( type == i18n( "Custom" ) ) return query->customType.contains( mime );
  return false;
}

bool KRSearchMod::fileMatch( const QString name )
{
  unsigned int len;
  for ( unsigned int i = 0; i < query->matches.count(); ++i )
    {
    QRegExp( *query->matches.at( i ), query->matchesCaseSensitive, true ).match( name, 0, ( int* ) & len );
    if ( len == name.length() ) return true;
    }
  return false;
}

void KRSearchMod::scanURL( KURL url )
{
  if( stopSearch ) return;
  
  unScannedUrls.push( url );    
  while ( !unScannedUrls.isEmpty() )
  {
    KURL urlToCheck = unScannedUrls.pop();
    
    if ( query->whereNotToSearch.contains( urlToCheck ) )
      continue;
    
    if( scannedUrls.contains( urlToCheck ) )
      continue;    
    scannedUrls.push( urlToCheck );
    
    emit searching( urlToCheck.prettyURL(0,KURL::StripFileProtocol) );
    
    if ( urlToCheck.isLocalFile() )
      scanLocalDir( urlToCheck );
    else
      scanRemoteDir( urlToCheck );
  
    qApp->processEvents(); // do a last one, in case passes%50 != 0
  }
}
  
void KRSearchMod::scanLocalDir( KURL urlToScan )
{
  int passes = 0;
  const int NO_OF_PASSES = 50;
    
  QString dir = urlToScan.path( 1 );

  DIR* d = opendir( dir.local8Bit() );
  if ( !d ) return ;

  struct dirent* dirEnt;
  QString name;
  unsigned long size;
  time_t mtime;
  KURL url;

  while ( ( dirEnt = readdir( d ) ) != NULL )
  {
    name = QString::fromLocal8Bit( dirEnt->d_name );
    url = vfs::fromPathOrURL( dir + name );
    
    QString mime = QString::null;
    
    // we dont scan the ".",".." enteries
    if ( name == "." || name == ".." ) continue;

    KDE_struct_stat stat_p;
    KDE_lstat( ( dir + name ).local8Bit(), &stat_p );
    if ( query->recurse )
    {
      if ( S_ISLNK( stat_p.st_mode ) && query->followLinks )
        unScannedUrls.push( vfs::fromPathOrURL( QDir( dir + name ).canonicalPath() ) );
      else if ( S_ISDIR( stat_p.st_mode ) )
        unScannedUrls.push( url );
    }
    if ( query->inArchive )
    {
      mime = KMimeType::findByURL( url, stat_p.st_mode, true, false ) ->name();
      QString type = mime.right( 4 );
      if ( mime.contains( "-rar" ) ) type = "-rar";

      if ( KRarcHandler::arcSupported( type ) )
      {
        KURL archiveURL = url;
        
        if ( type == "-tbz" || type == "-tgz" || type == "tarz" || type == "-tar" )
          archiveURL.setProtocol( "tar" );
        else
          archiveURL.setProtocol( "krarc" );
          
        unScannedUrls.push( archiveURL );
      }
    }
    // see if the name matches
    if ( !fileMatch( name ) ) continue;
    // check that the size fit
    size = stat_p.st_size;
    if ( query->minSize && size < query->minSize ) continue;
    if ( query->maxSize && size > query->maxSize ) continue;
    // check the time frame
    mtime = stat_p.st_mtime;
    if ( query->olderThen && mtime > query->olderThen ) continue;
    if ( query->newerThen && mtime < query->newerThen ) continue;
    // check the type
    if ( !query->type.isEmpty() )
    {
      if ( mime.isEmpty() )
        mime = KMimeType::findByURL( url, stat_p.st_mode, true, false ) ->name();
      if ( !checkType( mime ) ) continue;
    }
    // check owner name
    if ( !query->owner.isEmpty() &&
         stat_p.st_uid != KRpermHandler::user2uid( query->owner ) ) continue;
    // check group name
    if ( !query->group.isEmpty() &&
         stat_p.st_gid != KRpermHandler::group2gid( query->group ) ) continue;
    // check permission
    if ( !query->perm.isEmpty() &&
         !checkPerm( KRpermHandler::mode2QString( stat_p.st_mode ) ) ) continue;
    // check if it contains the text (avoid the /dev diretory).
    // grep code from KFind ( the copyright goes with our thanx to the unknown author )
    if ( !query->contain.isEmpty() && !dir.startsWith( "/dev" ) )
    {
      QFile qf( dir + name );

      qf.open( IO_ReadOnly );
      QTextStream text( &qf );
      text.setEncoding( QTextStream::Locale );
      QString line;
      bool found = false;
      while ( !stopSearch && !text.atEnd() )
        {
        line = text.readLine();
        if ( line.isNull() ) break;
        if ( query->containWholeWord )
        {
          int ndx = 0;

          while ( ( ndx = line.find( query->contain, ndx, query->containCaseSensetive ) ) != -1 )
          {
            QChar before = line.at( ndx - 1 );
            QChar after = line.at( ndx + query->contain.length() );

            if ( !before.isLetterOrNumber() && !after.isLetterOrNumber() &&
                 after != '_' && before != '_' )
            {
              found = true;
              break;
            }
            ndx++;
          }
        }
        else if ( line.find( query->contain, 0, query->containCaseSensetive ) != -1 )
          found = true;

        if ( found )
          break;
        }
      if ( !found ) continue;
    }

    // if we got here - we got a winner
    results.append( dir + name );
    //  kdWarning() << "Found: " << (dir+name).local8Bit() << endl;
    emit found( name, dir, ( KIO::filesize_t ) stat_p.st_size, stat_p.st_mtime, KRpermHandler::mode2QString( stat_p.st_mode ) );
    if ( passes++ % NO_OF_PASSES == 0 ) qApp->processEvents();
  }
  // clean up
  closedir( d );
}

void KRSearchMod::scanRemoteDir( KURL url )
{
  int passes = 0;
  const int NO_OF_PASSES = 50;

  if( remote_vfs == 0 )
    remote_vfs = new ftp_vfs( 0 );
    
  if ( !remote_vfs->vfs_refresh( url ) ) return ;

  for ( vfile * vf = remote_vfs->vfs_getFirstFile(); vf != 0 ; vf = remote_vfs->vfs_getNextFile() )
  {
    QString name = vf->vfile_getName();

    if ( query->recurse )
    {      
      if( ( vf->vfile_isSymLink() && query->followLinks ) || vf->vfile_isDir() )
      {
        KURL recurseURL = remote_vfs->vfs_getOrigin();
        recurseURL.addPath( name );
        unScannedUrls.push( recurseURL );
      }
    }
    // see if the name matches
    if ( !fileMatch( name ) ) continue;
    // check that the size fit
    KIO::filesize_t size = vf->vfile_getSize();
    if ( query->minSize && size < query->minSize ) continue;
    if ( query->maxSize && size > query->maxSize ) continue;
    // check the time frame
    time_t mtime = vf->vfile_getTime_t();
    if ( query->olderThen && mtime > query->olderThen ) continue;
    if ( query->newerThen && mtime < query->newerThen ) continue;
    // check owner name
    if ( !query->owner.isEmpty() &&
         vf->vfile_getOwner() != query->owner ) continue;
    // check group name
    if ( !query->group.isEmpty() &&
         vf->vfile_getGroup() != query->group ) continue;
    //check permission
    if ( !query->perm.isEmpty() && !checkPerm( vf->vfile_getPerm() ) ) continue;

    if ( !query->contain.isEmpty() )
    {
      /* TODO: search in remote vfs is not yet implemented */
    }
    
    // if we got here - we got a winner
    results.append( remote_vfs->vfs_getOrigin().prettyURL( 1 ) + name );
    emit found( name, remote_vfs->vfs_getOrigin().prettyURL( -1 ), size, vf->vfile_getTime_t(), vf->vfile_getPerm() );
    if ( passes++ % NO_OF_PASSES == 0 ) qApp->processEvents();
  }
}

#include "krsearchmod.moc"
