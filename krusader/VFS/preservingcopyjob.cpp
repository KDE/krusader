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
#include <kio/jobclasses.h>
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
    time_t mtime = (*it).mtime;
  
    if( mtime != 0 && mtime != ((time_t) -1 ) ) {       /* is it correct? */
      fileAttributes[ (*it).uSource ] = mtime;
    }
    else if( (*it).uSource.isLocalFile() ) {
      KDE_struct_stat stat_p;
      KDE_lstat( (*it).uSource.path(-1).local8Bit(),&stat_p);    /* getting the date information */
      
      fileAttributes[ (*it).uSource ] = stat_p.st_mtime;      
    }
  }
}

void PreservingCopyJob::slotResult( Job *job ) {
  if( job->inherits( "KIO::StatJob" ) ) {       /* Unfortunately KIO forgets to set times when the file is in the */
    KURL url = ((KIO::SimpleJob *)job)->url();  /* base directory. That's why we capture every StatJob and set the */
                                                /* time manually. */
    KIO::UDSEntry entry = static_cast<KIO::StatJob*>(job)->statResult();      
    KFileItem kfi(entry, url );
    
    fileAttributes[ url ] = kfi.time( KIO::UDS_MODIFICATION_TIME );
  }

  CopyJob::slotResult( job );
}

void PreservingCopyJob::slotCopyingDone( KIO::Job *, const KURL &from, const KURL &to, bool, bool)
{
  if( fileAttributes.count( from ) ) {
    time_t mtime = fileAttributes[ from ];
    fileAttributes.remove( from );
   
    if( !to.isLocalFile() || mtime == 0 || mtime == ((time_t) -1 ) )
      return;
    
    struct utimbuf timestamp;

    timestamp.actime  = time( 0 );
    timestamp.modtime = mtime;

    utime( (const char *)( to.path( -1 ).local8Bit() ), &timestamp );
  }
}

KIO::CopyJob * PreservingCopyJob::createCopyJob( PreserveMode pmode, const KURL::List& src, const KURL& dest, CopyMode mode, bool asMethod, bool showProgressInfo )
{
  if( ! dest.isLocalFile() )
    pmode = PM_NONE;
  if( mode == KIO::CopyJob::Link )
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
