/***************************************************************************
                    preservingcopyjob.cpp  -  description
                             -------------------
    copyright            : (C) 2005 + by Csaba Karai
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

#include "preservingcopyjob.h"
#include "../defaults.h"
#include "../krusader.h"
#include <utime.h>
#include <klargefile.h>
#include <kio/job.h>
#include <kfileitem.h>

PreservingCopyJob::PreservingCopyJob( const KURL::List& src, const KURL& dest, CopyMode mode,
  bool asMethod, bool showProgressInfo ) : KIO::CopyJob( src, dest, mode, asMethod, showProgressInfo )
{
  if( dest.isLocalFile() )
  {
    connect( this, SIGNAL( aboutToCreate (KIO::Job *, const QValueList< KIO::CopyInfo > &) ),
             this, SLOT( slotAboutToCreate (KIO::Job *, const QValueList< KIO::CopyInfo > &) ) );
    connect( this, SIGNAL( copyingDone( KIO::Job *, const KURL &, const KURL &, bool, bool) ),
             this, SLOT( slotCopyingDone( KIO::Job *, const KURL &, const KURL &, bool, bool) ) );
  }
}

void PreservingCopyJob::slotAboutToCreate( KIO::Job */*job*/, const QValueList< KIO::CopyInfo > &files )
{
  for ( QValueList< KIO::CopyInfo >::ConstIterator it = files.begin(); it != files.end(); ++it ) {
    
    time_info time_nfo;
    time_nfo.copy_finished = false;
    time_nfo.to = (*it).uDest;
    time_nfo.mtime = (*it).mtime;
    time_nfo.ctime = (*it).ctime;

    if( time_nfo.ctime == 0 || time_nfo.ctime == ((time_t) -1 ) )
      time_nfo.ctime = time_nfo.mtime;
        
    if( time_nfo.mtime == 0 || time_nfo.mtime == ((time_t) -1 ) ) {       /* is it correct? */
      KURL origURL = (*it).uSource;
      if( origURL.isLocalFile() ) {
        KDE_struct_stat stat_p;
        KDE_lstat( origURL.path(-1).local8Bit(),&stat_p);    /* getting the date information */
      
        time_nfo.mtime = stat_p.st_mtime;      
        time_nfo.ctime = stat_p.st_ctime;
      } else {
        KIO::StatJob* statJob = KIO::stat( origURL, false );
        connect( statJob, SIGNAL( result( KIO::Job* ) ), this, SLOT( slotStatResult( KIO::Job* ) ) );
        pendingJobs[ statJob ] = origURL;
      }
    }
    
    fileAttributes[ (*it).uSource ] = time_nfo;
  }
}

void PreservingCopyJob::setTime( KURL url, time_info time_nfo ) {
    if( url.isLocalFile() ) {
      struct utimbuf timestamp;

      timestamp.actime  = time_nfo.ctime;
      timestamp.modtime = time_nfo.mtime;

      utime( (const char *)( url.path( -1 ).local8Bit() ), &timestamp );
    }
}

void PreservingCopyJob::slotCopyingDone( KIO::Job *, const KURL &from, const KURL &to, bool, bool)
{
  if( fileAttributes.count( from ) )
  {
    fileAttributes[ from ].copy_finished = true;
    
    if( fileAttributes[ from ].mtime == 0 || fileAttributes[ from ].mtime == ((time_t) -1 ) || 
        fileAttributes[ from ].ctime == 0 || fileAttributes[ from ].ctime == ((time_t) -1 ) )
      return;
    
    setTime( to, fileAttributes[ from ] );
    fileAttributes.remove( from );
  }
}

void PreservingCopyJob::slotStatResult( KIO::Job* job ) {
  if( !job )
    return;
    
  if( pendingJobs.count( job ) ) {
    KURL origURL = pendingJobs[ job ];
    pendingJobs.remove( job );
    
    if( job->error() ) {
      fileAttributes.remove( origURL );
      return;  
    }
    
    if( fileAttributes.count( origURL ) )
    {
      KIO::UDSEntry entry = static_cast<KIO::StatJob*>(job)->statResult();
      
      KFileItem kfi(entry, origURL );
      time_t mtime =  kfi.time( KIO::UDS_MODIFICATION_TIME );
      time_t ctime =  kfi.time( KIO::UDS_CREATION_TIME );

      if( ctime == 0 || ctime == ((time_t) -1 ) )
        ctime = mtime;
            
      if( mtime == 0 || mtime == ((time_t) -1 ) /*|| ctime == 0 || ctime == ((time_t) -1 )*/)
        fileAttributes.remove( origURL );
      else {
        fileAttributes[ origURL ].mtime = mtime;
        fileAttributes[ origURL ].ctime = ctime;
        
        if( fileAttributes[ origURL ].copy_finished ) {
          setTime( fileAttributes[ origURL ].to, fileAttributes[ origURL ] );        
          fileAttributes.remove( origURL );
        }
      }
    }
  }
}

KIO::CopyJob * PreservingCopyJob::createCopyJob( PreserveMode pmode, const KURL::List& src, const KURL& dest, CopyMode mode, bool asMethod, bool showProgressInfo )
{
  if( ! dest.isLocalFile() )
    pmode = PM_NONE;

  switch( pmode )
  {
  case PM_PRESERVE_DATE:
    return new PreservingCopyJob( src, dest, mode, asMethod, showProgressInfo );
  case PM_DEFAULT:
    {
      QString group = krConfig->group();
      krConfig->setGroup( "Advanced" );
      bool preserve = krConfig->readBoolEntry( "PreserveDate", _PreserveDate );
      krConfig->setGroup( group );
      if( preserve )
        return new PreservingCopyJob( src, dest, mode, asMethod, showProgressInfo );
      else
        return new KIO::CopyJob( src, dest, mode, asMethod, showProgressInfo );
    }
  case PM_NONE:
  default:
    return new KIO::CopyJob( src, dest, mode, asMethod, showProgressInfo );
  }
}

#include "preservingcopyjob.moc"
