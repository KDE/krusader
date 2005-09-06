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
  
  addApplication( "df",       pathsGrid, 0, general_tab );
  addApplication( "eject",    pathsGrid, 1, general_tab );
  addApplication( "kdesu",    pathsGrid, 2, general_tab );
  addApplication( "kget",     pathsGrid, 3, general_tab );
  addApplication( "kmail",    pathsGrid, 4, general_tab );
  addApplication( "diff utility",  pathsGrid, 5, general_tab );
  addApplication( "krename",  pathsGrid, 6, general_tab );
  addApplication( "krusader", pathsGrid, 7, general_tab );
  addApplication( "locate",   pathsGrid, 8, general_tab );
  addApplication( "mount",    pathsGrid, 9, general_tab );
  addApplication( "umount",   pathsGrid,10, general_tab );
  addApplication( "updatedb", pathsGrid,11, general_tab );
  addApplication( "md5sum", pathsGrid, 12, general_tab );
  addApplication( "md5deep", pathsGrid, 13, general_tab );

  //  ---------------------------- PACKERS TAB -------------------------------------
  QWidget *packers_tab = new QWidget( tabWidget, "tab_3" );
  tabWidget->insertTab( packers_tab, i18n( "Packers" ) );

  QGridLayout *archGrid = new QGridLayout( packers_tab );
  archGrid->setSpacing( 6 );
  archGrid->setMargin( 11 );
  archGrid->setAlignment( Qt::AlignTop );

  addApplication( "arj",   archGrid, 0, packers_tab );
  addApplication( "bzip2", archGrid, 1, packers_tab );
  addApplication( "cpio",  archGrid, 2, packers_tab );
  addApplication( "gzip",  archGrid, 3, packers_tab );
  addApplication( "lha",   archGrid, 4, packers_tab );
  addApplication( "rar",   archGrid, 5, packers_tab );
  addApplication( "tar",   archGrid, 6, packers_tab );
  addApplication( "unace", archGrid, 7, packers_tab );
  addApplication( "unarj", archGrid, 8, packers_tab );
  addApplication( "unrar", archGrid, 9, packers_tab );
  addApplication( "unzip", archGrid,10, packers_tab );
  addApplication( "zip",   archGrid,11, packers_tab );

  kgDependenciesLayout->addWidget( tabWidget, 0, 0 );
}

void KgDependencies::addApplication( QString name, QGridLayout *grid, int row, QWidget *parent  )
{
  QString dflt = KrServices::fullPathName( name ); /* try to autodetect the full path name */
  addLabel( grid, row, 0, name, parent, (QString( "label:" )+name).ascii() );

  KonfiguratorURLRequester *fullPath = createURLRequester( "Dependencies", name, dflt, parent, false );
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

#include "kgdependencies.moc"
