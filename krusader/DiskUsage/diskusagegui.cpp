/***************************************************************************
                       diskusagegui.cpp  -  description
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

#include "diskusagegui.h"
#include "../kicons.h"
#include "../krusader.h"
#include "../VFS/vfs.h"
#include "../Dialogs/krdialogs.h"

#include <qtimer.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <klocale.h>

DiskUsageGUI::DiskUsageGUI( KUrl openDir, QWidget* parent ) 
  : QDialog( parent ), exitAtFailure( true )
{  
  setCaption( i18n("Krusader::Disk Usage") );
  
  baseDirectory = openDir;
  if( !newSearch() )
    return;
  
  QGridLayout *duGrid = new QGridLayout( this );
  duGrid->setSpacing( 6 );
  duGrid->setContentsMargins( 11, 11, 11, 11 );
  
  QWidget *duTools = new QWidget( this );
  QHBoxLayout *duHBox = new QHBoxLayout( duTools );
  duTools->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    
  btnNewSearch = new QToolButton( duTools );
  btnNewSearch->setIconSet( QIcon(krLoader->loadIcon("fileopen",KIconLoader::Desktop)) );
  duHBox->addWidget( btnNewSearch );
  btnNewSearch->setToolTip( i18n( "Start new disk usage search" ) );
  
  btnRefresh = new QToolButton( duTools );
  btnRefresh->setIconSet( QIcon(krLoader->loadIcon("reload",KIconLoader::Desktop)) );
  duHBox->addWidget( btnRefresh );
  btnRefresh->setToolTip( i18n( "Refresh" ) );

  btnDirUp = new QToolButton( duTools );
  btnDirUp->setIconSet( QIcon(krLoader->loadIcon("up",KIconLoader::Desktop)) );
  duHBox->addWidget( btnDirUp );
  btnDirUp->setToolTip( i18n( "Parent directory" ) );
  
  QWidget * separatorWidget = new QWidget( duTools );
  separatorWidget->setMinimumWidth( 10 );
  duHBox->addWidget( separatorWidget );
  
  btnLines = new QToolButton( duTools );
  btnLines->setIconSet( QIcon(krLoader->loadIcon("leftjust",KIconLoader::Desktop)) );
  btnLines->setToggleButton( true );
  duHBox->addWidget( btnLines );
  btnLines->setToolTip( i18n( "Line view" ) );

  btnDetailed = new QToolButton( duTools );
  btnDetailed->setIconSet( QIcon(krLoader->loadIcon("view_detailed",KIconLoader::Desktop)) );
  btnDetailed->setToggleButton( true );
  duHBox->addWidget( btnDetailed );
  btnDetailed->setToolTip( i18n( "Detailed view" ) );

  btnFilelight = new QToolButton( duTools );
  btnFilelight->setIconSet( QIcon(krLoader->loadIcon("kr_diskusage",KIconLoader::Desktop)) );
  btnFilelight->setToggleButton( true );
  duHBox->addWidget( btnFilelight );
  btnFilelight->setToolTip( i18n( "Filelight view" ) );
    
  QWidget *spacerWidget = new QWidget( duTools );
  duHBox->addWidget( spacerWidget );
  QHBoxLayout *hboxlayout = new QHBoxLayout( spacerWidget );
  QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed );
  hboxlayout->addItem( spacer );
  
  duGrid->addWidget( duTools, 0, 0 );
  
  diskUsage = new DiskUsage( "DiskUsage", this );
  duGrid->addWidget( diskUsage, 1, 0 );
  
  status = new KSqueezedTextLabel( this );
  status->setFrameShape( QLabel::StyledPanel );
  status->setFrameShadow( QLabel::Sunken );  
  duGrid->addWidget( status, 2, 0 );
  
  connect( diskUsage, SIGNAL( status( QString ) ), this, SLOT( setStatus( QString ) ) );
  connect( diskUsage, SIGNAL( viewChanged( int ) ), this, SLOT( slotViewChanged( int ) ) );
  connect( diskUsage, SIGNAL( newSearch() ), this,  SLOT( newSearch() ) );
  connect( diskUsage, SIGNAL( loadFinished( bool ) ), this,  SLOT( slotLoadFinished( bool ) ) );
  connect( btnNewSearch, SIGNAL( clicked() ), this, SLOT( newSearch() ) );
  connect( btnRefresh, SIGNAL( clicked() ), this, SLOT( loadUsageInfo() ) );
  connect( btnDirUp, SIGNAL( clicked() ), diskUsage, SLOT( dirUp() ) );
  connect( btnLines, SIGNAL( clicked() ), this, SLOT( selectLinesView() ) );
  connect( btnDetailed, SIGNAL( clicked() ), this, SLOT( selectListView() ) );
  connect( btnFilelight, SIGNAL( clicked() ), this, SLOT( selectFilelightView() ) );  
  
  KConfigGroup group( krConfig, "DiskUsage" ); 
  
  int view = group.readEntry( "View",  VIEW_LINES );
  if( view < VIEW_LINES || view > VIEW_FILELIGHT )
    view = VIEW_LINES;    
  diskUsage->setView( view );
  
  sizeX = group.readEntry( "Window Width",  QFontMetrics(font()).width("W") * 70 );
  sizeY = group.readEntry( "Window Height", QFontMetrics(font()).height() * 25 );    
  resize( sizeX, sizeY );
  
  if( group.readEntry( "Window Maximized",  false ) )
    showMaximized();
  else  
    show();

  exec();
}

DiskUsageGUI::~DiskUsageGUI()
{
}

void DiskUsageGUI::slotLoadFinished( bool result )
{
  if( exitAtFailure && !result )
    reject();
  else
    exitAtFailure = false;
}

void DiskUsageGUI::enableButtons( bool isOn )
{
  btnNewSearch->setEnabled( isOn );
  btnRefresh->setEnabled( isOn );
  btnDirUp->setEnabled( isOn );
  btnLines->setEnabled( isOn );
  btnDetailed->setEnabled( isOn );
  btnFilelight->setEnabled( isOn );
}

void DiskUsageGUI::resizeEvent( QResizeEvent *e )
{   
  if( !isMaximized() )
  {
    sizeX = e->size().width();
    sizeY = e->size().height();
  }
  QDialog::resizeEvent( e );
}

void DiskUsageGUI::reject()
{
  KConfigGroup group( krConfig, "DiskUsage" ); 
  group.writeEntry("Window Width", sizeX );
  group.writeEntry("Window Height", sizeY );
  group.writeEntry("Window Maximized", isMaximized() );
  group.writeEntry("View", diskUsage->getActiveView() );
  
  QDialog::reject();
}

void DiskUsageGUI::loadUsageInfo()
{
  diskUsage->load( baseDirectory );
}

void DiskUsageGUI::setStatus( QString stat )
{
  status->setText( stat );
}

void DiskUsageGUI::slotViewChanged( int view )
{
  if( view == VIEW_LOADER )
  {
    enableButtons( false );
    return;
  }
  enableButtons( true );

  btnLines->setOn( false );
  btnDetailed->setOn( false );
  btnFilelight->setOn( false );
  
  switch( view )
  {
  case VIEW_LINES:
    btnLines->setOn( true );
    break;
  case VIEW_DETAILED:
    btnDetailed->setOn( true );
    break;
  case VIEW_FILELIGHT:
    btnFilelight->setOn( true );
    break;
  case VIEW_LOADER:
    break;
  }
}

bool DiskUsageGUI::newSearch()
{ 
  // ask the user for the copy dest
  
  KUrl tmp = KChooseDir::getDir(i18n( "Viewing the usage of directory:" ), baseDirectory, baseDirectory);
  if (tmp.isEmpty()) return false;
  baseDirectory = tmp;
  
  QTimer::singleShot( 0, this, SLOT( loadUsageInfo() ) );
  return true;
}

#include "diskusagegui.moc"
