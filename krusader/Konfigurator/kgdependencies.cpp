/***************************************************************************
                       kgdependencies.cpp  -  description
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

#include "kgdependencies.h"
#include "../krservices.h"
#include "../krusader.h"
#include <qtabwidget.h>
#include <klocale.h>
#include <qhbox.h>
#include <kmessagebox.h>

KgDependencies::KgDependencies( bool first, QWidget* parent,  const char* name ) :
      KonfiguratorPage( first, parent, name )
{
  QGridLayout *kgDependenciesLayout = new QGridLayout( parent );
  kgDependenciesLayout->setSpacing( 6 );
  kgDependenciesLayout->setMargin( 11 );

  //  ---------------------------- GENERAL TAB -------------------------------------
  QTabWidget *tabWidget = new QTabWidget( parent, "tabWidget" );

  QWidget *general_tab = new QWidget( tabWidget, "tab" );
  tabWidget->insertTab( general_tab, i18n( "General" ) );

  QGridLayout *pathsGrid = new QGridLayout( general_tab );
  pathsGrid->setSpacing( 6 );
  pathsGrid->setMargin( 11 );
  pathsGrid->setAlignment( Qt::AlignTop );
  
  addLabel( pathsGrid, 0, 0, "df", general_tab, "dfName" );
  KonfiguratorURLRequester *dfPath = createURLRequester( "Dependencies", "df", "", general_tab, false );
  pathsGrid->addWidget( dfPath, 0, 1 );

  addLabel( pathsGrid, 1, 0, "eject", general_tab, "ejectName" );
  KonfiguratorURLRequester *ejectPath = createURLRequester( "Dependencies", "eject", "", general_tab, false );
  pathsGrid->addWidget( ejectPath, 1, 1 );

  addApplication( "kdesu", pathsGrid, 2, general_tab );

  addLabel( pathsGrid, 3, 0, "kmail", general_tab, "kmailName" );
  KonfiguratorURLRequester *kmailPath = createURLRequester( "Dependencies", "kmail", "", general_tab, false );
  pathsGrid->addWidget( kmailPath, 3, 1 );

  addLabel( pathsGrid, 4, 0, "kompare", general_tab, "kompareName" );
  KonfiguratorURLRequester *komparePath = createURLRequester( "Dependencies", "kompare", "", general_tab, false );
  pathsGrid->addWidget( komparePath, 4, 1 );

  addLabel( pathsGrid, 5, 0, "krename", general_tab, "krenameName" );
  KonfiguratorURLRequester *krenamePath = createURLRequester( "Dependencies", "krename", "", general_tab, false );
  pathsGrid->addWidget( krenamePath, 5, 1 );

  addApplication( "krusader", pathsGrid, 6, general_tab );

  addLabel( pathsGrid, 7, 0, "mount", general_tab, "mountName" );
  KonfiguratorURLRequester *mountPath = createURLRequester( "Dependencies", "mount", "", general_tab, false );
  pathsGrid->addWidget( mountPath, 7, 1 );

  addLabel( pathsGrid, 8, 0, "umount", general_tab, "umountName" );
  KonfiguratorURLRequester *umountPath = createURLRequester( "Dependencies", "umount", "", general_tab, false );
  pathsGrid->addWidget( umountPath, 8, 1 );

  //  ---------------------------- PACKERS TAB -------------------------------------
  QWidget *packers_tab = new QWidget( tabWidget, "tab_3" );
  tabWidget->insertTab( packers_tab, i18n( "Packers" ) );

  QGridLayout *archGrid = new QGridLayout( packers_tab );
  archGrid->setSpacing( 6 );
  archGrid->setMargin( 11 );
  archGrid->setAlignment( Qt::AlignTop );

  addLabel( archGrid, 0, 0, "bzip2", packers_tab, "bzip2Name" );
  KonfiguratorURLRequester *bzip2Path = createURLRequester( "Dependencies", "bzip2", "", packers_tab, false );
  archGrid->addWidget( bzip2Path, 0, 1 );

  addLabel( archGrid, 1, 0, "cpio", packers_tab, "cpioName" );
  KonfiguratorURLRequester *cpioPath = createURLRequester( "Dependencies", "cpio", "", packers_tab, false );
  archGrid->addWidget( cpioPath, 1, 1 );

  addLabel( archGrid, 2, 0, "gzip", packers_tab, "gzipName" );
  KonfiguratorURLRequester *gzipPath = createURLRequester( "Dependencies", "gzip", "", packers_tab, false );
  archGrid->addWidget( gzipPath, 2, 1 );

  addLabel( archGrid, 3, 0, "rar", packers_tab, "rarName" );
  KonfiguratorURLRequester *rarPath = createURLRequester( "Dependencies", "rar", "", packers_tab, false );
  archGrid->addWidget( rarPath, 3, 1 );

  addLabel( archGrid, 4, 0, "tar", packers_tab, "tarName" );
  KonfiguratorURLRequester *tarPath = createURLRequester( "Dependencies", "tar", "", packers_tab, false );
  archGrid->addWidget( tarPath, 4, 1 );

  addLabel( archGrid, 5, 0, "unace", packers_tab, "unaceName" );
  KonfiguratorURLRequester *unacePath = createURLRequester( "Dependencies", "unace", "", packers_tab, false );
  archGrid->addWidget( unacePath, 5, 1 );

  addLabel( archGrid, 6, 0, "unarj", packers_tab, "unarjName" );
  KonfiguratorURLRequester *unarjPath = createURLRequester( "Dependencies", "unarj", "", packers_tab, false );
  archGrid->addWidget( unarjPath, 6, 1 );

  addLabel( archGrid, 7, 0, "unrar", packers_tab, "unrarName" );
  KonfiguratorURLRequester *unrarPath = createURLRequester( "Dependencies", "unrar", "", packers_tab, false );
  archGrid->addWidget( unrarPath, 7, 1 );

  addLabel( archGrid, 8, 0, "unzip", packers_tab, "unzipName" );
  KonfiguratorURLRequester *unzipPath = createURLRequester( "Dependencies", "unzip", "", packers_tab, false );
  archGrid->addWidget( unzipPath, 8, 1 );

  addLabel( archGrid, 9, 0, "zip", packers_tab, "zipName" );
  KonfiguratorURLRequester *zipPath = createURLRequester( "Dependencies", "zip", "", packers_tab, false );
  archGrid->addWidget( zipPath, 9, 1 );

  kgDependenciesLayout->addWidget( tabWidget, 0, 0 );
}

void KgDependencies::addApplication( QString name, QGridLayout *grid, int row, QWidget *parent  )
{
  KrServices::fullPathName( name ); /* try to autodetect the full path name */
  addLabel( grid, row, 0, name, parent, (QString( "label:" )+name).ascii() );

  KonfiguratorURLRequester *fullPath = createURLRequester( "Dependencies", name, "", parent, false );
  connect( fullPath->extension(), SIGNAL( applyManually( QObject *, QString, QString ) ),
           this, SLOT( slotApply( QObject *, QString, QString ) ) );
  grid->addWidget( fullPath, row, 1 );
}

void KgDependencies::slotApply( QObject *obj, QString cls, QString name )
{
  KonfiguratorURLRequester *urlRequester = (KonfiguratorURLRequester *) obj;

  krConfig->setGroup( cls );
  krConfig->writeEntry( name, urlRequester->url() );

  QString usedPath = KrServices::fullPathName( name );

  if( urlRequester->url() != usedPath )
  {
    krConfig->writeEntry( name, usedPath );
    if( usedPath.isEmpty() )
      KMessageBox::error( this, i18n( "The %1 path is incorrect, no valid path found." )
                          .arg( urlRequester->url() ) );
    else
      KMessageBox::error( this, i18n( "The %1 path is incorrect, %2 used instead." )
                          .arg( urlRequester->url() ).arg( usedPath ) );
    urlRequester->setURL( usedPath );
  }
}
