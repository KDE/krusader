/***************************************************************************
                       dufilelight.cpp  -  description
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

#include "dufilelight.h"
#include "radialMap/radialMap.h"

DUFilelight::DUFilelight( DiskUsage *usage, const char *name )
  : RadialMap::Widget( usage, name ), diskUsage( usage ), currentDir( 0 )
{
   connect( diskUsage, SIGNAL( enteringDirectory( Directory * ) ), this, SLOT( slotDirChanged( Directory * ) ) );
   connect( diskUsage, SIGNAL( clearing() ), this, SLOT( clear() ) );
   connect( diskUsage, SIGNAL( changed( File * ) ), this, SLOT( slotChanged( File * ) ) );
   connect( this, SIGNAL( activated( const KURL& ) ), this, SLOT( slotActivated( const KURL& ) ) );
}

void DUFilelight::slotDirChanged( Directory *dir )
{
  if( currentDir != dir )
  {
    currentDir = dir;
    
    File::setBaseURL( diskUsage->getBaseURL() );
    invalidate( false );
    create( dir );
    refreshNeeded = false;
  }
}

void DUFilelight::clear()
{
  invalidate( false );
  currentDir = 0;
}

void DUFilelight::slotActivated( const KURL& url )
{
  KURL baseURL = diskUsage->getBaseURL();
  
  if( !baseURL.path().endsWith("/") )
    baseURL.setPath( baseURL.path() + "/" );
    
  QString relURL = KURL::relativeURL( baseURL, url );
  
  if( relURL.endsWith( "/" ) )
    relURL.truncate( relURL.length() - 1 );
  
  Directory * dir = diskUsage->getDirectory( relURL );
  if( dir != 0 && dir != currentDir )
  {
    currentDir = dir;
    diskUsage->changeDirectory( dir );  
  }
}

void DUFilelight::mousePressEvent( QMouseEvent *event )
{
   if( event->button() == Qt::RightButton )
   {
     File * file = 0;
     
     const RadialMap::Segment * focus = focusSegment();
     
     if( focus && !focus->isFake() && focus->file() != currentDir )
       file = (File *)focus->file();
         
     diskUsage->rightClickMenu( file );
     return;     
   }
   RadialMap::Widget::mousePressEvent( event );
}
  
void DUFilelight::slotRefresh() 
{ 
  refreshNeeded = false;
  if( currentDir )
  {
    invalidate( false );
    create( currentDir );
  }
}

void DUFilelight::slotChanged( File * )
{
   if( !refreshNeeded )
   {
     refreshNeeded = true;
     QTimer::singleShot( 0, this, SLOT( slotRefresh() ) );
   }
}

#include "dufilelight.moc"
