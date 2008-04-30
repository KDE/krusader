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
#include <kmenu.h>
#include <klocale.h>
#include <kinputdialog.h>
#include <QPixmap>
#include <QMouseEvent>

#define SCHEME_POPUP_ID    6730

DUFilelight::DUFilelight( DiskUsage *usage )
  : RadialMap::Widget( usage ), diskUsage( usage ), currentDir( 0 ), refreshNeeded( true )
{
   setFocusPolicy( Qt::StrongFocus );

   connect( diskUsage, SIGNAL( enteringDirectory( Directory * ) ), this, SLOT( slotDirChanged( Directory * ) ) );
   connect( diskUsage, SIGNAL( clearing() ), this, SLOT( clear() ) );
   connect( diskUsage, SIGNAL( changed( File * ) ), this, SLOT( slotChanged( File * ) ) );
   connect( diskUsage, SIGNAL( deleted( File * ) ), this, SLOT( slotChanged( File * ) ) );
   connect( diskUsage, SIGNAL( changeFinished()  ), this, SLOT( slotRefresh() ) );
   connect( diskUsage, SIGNAL( deleteFinished()  ), this, SLOT( slotRefresh() ) );
   connect( diskUsage, SIGNAL( currentChanged( int ) ), this, SLOT( slotAboutToShow( int ) ) );
}

void DUFilelight::slotDirChanged( Directory *dir )
{
  if( diskUsage->currentWidget() != this )
    return;
    
  if( currentDir != dir )
  {
    currentDir = dir;
    
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

File * DUFilelight::getCurrentFile()
{
  const RadialMap::Segment * focus = focusSegment();
     
  if( !focus || focus->isFake() || focus->file() == currentDir )
    return 0;
    
  return (File *)focus->file();
}

void DUFilelight::mousePressEvent( QMouseEvent *event )
{
   if( event->button() == Qt::RightButton )
   {
     File * file = 0;
     
     const RadialMap::Segment * focus = focusSegment();
     
     if( focus && !focus->isFake() && focus->file() != currentDir )
       file = (File *)focus->file();

     KMenu filelightPopup;
     filelightPopup.addAction( i18n("Zoom In"),  this, SLOT( zoomIn() ), Qt::Key_Plus );
     filelightPopup.addAction( i18n("Zoom Out"), this, SLOT( zoomOut() ), Qt::Key_Minus );
     
     KMenu schemePopup;           
     schemePopup.addAction( i18n("Rainbow"),       this, SLOT( schemeRainbow() ) );
     schemePopup.addAction( i18n("High Contrast"), this, SLOT( schemeHighContrast() ) );
     schemePopup.addAction( i18n("KDE"),           this, SLOT( schemeKDE() ) );

     filelightPopup.addMenu( &schemePopup );
     schemePopup.setTitle( i18n( "Scheme" ) );     

     filelightPopup.addAction( i18n("Increase contrast"), this, SLOT( increaseContrast() ) );
     filelightPopup.addAction( i18n("Decrease contrast"), this, SLOT( decreaseContrast() ) );
          
     QAction * act = filelightPopup.addAction( i18n("Use anti-aliasing" ), this, SLOT( changeAntiAlias() ) );
     act->setCheckable( true );
     act->setChecked( Filelight::Config::antiAliasFactor > 1 );
     
     act = filelightPopup.addAction( i18n("Show small files" ), this, SLOT( showSmallFiles() ) );
     act->setCheckable( true );
     act->setChecked( Filelight::Config::showSmallFiles );
     
     act = filelightPopup.addAction( i18n("Vary label font sizes" ), this, SLOT( varyLabelFontSizes() ) );
     act->setCheckable( true );
     act->setChecked( Filelight::Config::varyLabelFontSizes );

     filelightPopup.addAction( i18n("Minimum font size"), this, SLOT( minFontSize() ) );     
          
     diskUsage->rightClickMenu( event->globalPos(), file, &filelightPopup, i18n( "Filelight" ) );
     return;     
   }else if( event->button() == Qt::LeftButton )
   {
     const RadialMap::Segment * focus = focusSegment();
     
     if( focus && !focus->isFake() && focus->file() == currentDir )
     {
       diskUsage->dirUp();
       return;
     }
     else if( focus && !focus->isFake() && focus->file()->isDir() )
     {
       diskUsage->changeDirectory( (Directory *)focus->file() );
       return;
     }
   }
   
   RadialMap::Widget::mousePressEvent( event );
}
  
void DUFilelight::setScheme( Filelight::MapScheme scheme )
{
  Filelight::Config::scheme = scheme;
  Filelight::Config::write();    
  slotRefresh();
}

void DUFilelight::increaseContrast()
{
  if( ( Filelight::Config::contrast += 10 ) > 100 )
    Filelight::Config::contrast = 100;
  
  Filelight::Config::write();    
  slotRefresh();
}

void DUFilelight::decreaseContrast()
{
  if( ( Filelight::Config::contrast -= 10 ) > 100 )
    Filelight::Config::contrast = 0;
  
  Filelight::Config::write();
  slotRefresh();
}

void DUFilelight::changeAntiAlias()
{
  Filelight::Config::antiAliasFactor = 1 + ( Filelight::Config::antiAliasFactor == 1 );
  Filelight::Config::write();    
  slotRefresh();
}

void DUFilelight::showSmallFiles()
{
  Filelight::Config::showSmallFiles = !Filelight::Config::showSmallFiles;
  Filelight::Config::write();    
  slotRefresh();
}

void DUFilelight::varyLabelFontSizes()
{
  Filelight::Config::varyLabelFontSizes = !Filelight::Config::varyLabelFontSizes;
  Filelight::Config::write();    
  slotRefresh();
}

void DUFilelight::minFontSize()
{
  bool ok = false;
  
  int result = KInputDialog::getInteger( i18n( "Krusader::Filelight" ),
    i18n( "Minimum font size" ), (int)Filelight::Config::minFontPitch, 1, 100, 1, &ok, this );

  if ( ok )
  {
    Filelight::Config::minFontPitch = (uint)result;
    
    Filelight::Config::write();    
    slotRefresh();
  }
}

void DUFilelight::slotAboutToShow( int ndx )
{ 
  QWidget *widget = diskUsage->widget( ndx );
  if( widget == this && ( diskUsage->getCurrentDir() != currentDir || refreshNeeded ) )
  {
    refreshNeeded = false;
    if( ( currentDir = diskUsage->getCurrentDir() ) != 0 )
    {
      invalidate( false );
      create( currentDir );
    }
  }
}

void DUFilelight::slotRefresh() 
{ 
  if( diskUsage->currentWidget() != this )
    return;

  refreshNeeded = false;
  if( currentDir && currentDir == diskUsage->getCurrentDir() )
  {
    invalidate( false );
    create( currentDir );
  }
}

void DUFilelight::slotChanged( File * )
{
   if( !refreshNeeded )
     refreshNeeded = true;
}

#include "dufilelight.moc"
