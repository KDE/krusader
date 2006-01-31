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

#define PAGE_GENERAL   0
#define PAGE_PACKERS   1
#define PAGE_CHECKSUM  2

KgDependencies::KgDependencies( bool first, QWidget* parent,  const char* name ) :
      KonfiguratorPage( first, parent, name )
{
  QGridLayout *kgDependenciesLayout = new QGridLayout( parent );
  kgDependenciesLayout->setSpacing( 6 );
  kgDependenciesLayout->setMargin( 11 );

  //  ---------------------------- GENERAL TAB -------------------------------------
  tabWidget = new QTabWidget( parent, "tabWidget" );

  QWidget *general_tab = new QWidget( tabWidget, "tab" );
  tabWidget->insertTab( general_tab, i18n( "General" ) );

  QGridLayout *pathsGrid = new QGridLayout( general_tab );
  pathsGrid->setSpacing( 6 );
  pathsGrid->setMargin( 11 );
  pathsGrid->setAlignment( Qt::AlignTop );
  
  addApplication( "df",       pathsGrid, 0, general_tab, PAGE_GENERAL );
  addApplication( "eject",    pathsGrid, 1, general_tab, PAGE_GENERAL );
  addApplication( "kdesu",    pathsGrid, 2, general_tab, PAGE_GENERAL );
  addApplication( "kget",     pathsGrid, 3, general_tab, PAGE_GENERAL );
  addApplication( "kmail",    pathsGrid, 4, general_tab, PAGE_GENERAL );
  addApplication( "diff utility",  pathsGrid, 5, general_tab, PAGE_GENERAL );
  addApplication( "krename",  pathsGrid, 6, general_tab, PAGE_GENERAL );
  addApplication( "krusader", pathsGrid, 7, general_tab, PAGE_GENERAL );
  addApplication( "locate",   pathsGrid, 8, general_tab, PAGE_GENERAL );
  addApplication( "mount",    pathsGrid, 9, general_tab, PAGE_GENERAL );
  addApplication( "umount",   pathsGrid,10, general_tab, PAGE_GENERAL );
  addApplication( "updatedb", pathsGrid,11, general_tab, PAGE_GENERAL );

  //  ---------------------------- PACKERS TAB -------------------------------------
  QWidget *packers_tab = new QWidget( tabWidget, "tab_3" );
  tabWidget->insertTab( packers_tab, i18n( "Packers" ) );

  QGridLayout *archGrid1 = new QGridLayout( packers_tab );
  archGrid1->setSpacing( 6 );
  archGrid1->setMargin( 11 );
  archGrid1->setAlignment( Qt::AlignTop );

  addApplication( "arj",   archGrid1, 0, packers_tab, PAGE_PACKERS );
  addApplication( "bzip2", archGrid1, 1, packers_tab, PAGE_PACKERS );
  addApplication( "cpio",  archGrid1, 2, packers_tab, PAGE_PACKERS );
  addApplication( "dpkg",  archGrid1, 3, packers_tab, PAGE_PACKERS );
  addApplication( "gzip",  archGrid1, 4, packers_tab, PAGE_PACKERS );
  addApplication( "lha",   archGrid1, 5, packers_tab, PAGE_PACKERS );
  addApplication( "rar",   archGrid1, 6, packers_tab, PAGE_PACKERS );
  addApplication( "tar",   archGrid1, 7, packers_tab, PAGE_PACKERS );
  addApplication( "unace", archGrid1, 8, packers_tab, PAGE_PACKERS );
  addApplication( "unarj", archGrid1, 9, packers_tab, PAGE_PACKERS );
  addApplication( "unrar", archGrid1,10, packers_tab, PAGE_PACKERS );
  addApplication( "unzip", archGrid1,11, packers_tab, PAGE_PACKERS );
  addApplication( "zip",   archGrid1,12, packers_tab, PAGE_PACKERS );

  //  ---------------------------- CHECKSUM TAB -------------------------------------
  QWidget *checksum_tab = new QWidget( tabWidget, "tab_4" );
  tabWidget->insertTab( checksum_tab, i18n( "Checksum Utilities" ) );

  QGridLayout *archGrid2 = new QGridLayout( checksum_tab );
  archGrid2->setSpacing( 6 );
  archGrid2->setMargin( 11 );
  archGrid2->setAlignment( Qt::AlignTop );

  addApplication( "md5sum",         archGrid2, 0, checksum_tab, PAGE_CHECKSUM );
  addApplication( "sha1sum",        archGrid2, 1, checksum_tab, PAGE_CHECKSUM );
  addApplication( "md5deep",        archGrid2, 2, checksum_tab, PAGE_CHECKSUM );
  addApplication( "sha1deep",       archGrid2, 3, checksum_tab, PAGE_CHECKSUM );
  addApplication( "sha256deep",     archGrid2, 4, checksum_tab, PAGE_CHECKSUM );
  addApplication( "tigerdeep",      archGrid2, 5, checksum_tab, PAGE_CHECKSUM );
  addApplication( "whirlpooldeep",  archGrid2, 6, checksum_tab, PAGE_CHECKSUM );
  addApplication( "cfv",            archGrid2, 7, checksum_tab, PAGE_CHECKSUM );

  kgDependenciesLayout->addWidget( tabWidget, 0, 0 );
}

void KgDependencies::addApplication( QString name, QGridLayout *grid, int row, QWidget *parent, int page )
{
  QString dflt = KrServices::fullPathName( name ); /* try to autodetect the full path name */
  addLabel( grid, row, 0, name, parent, (QString( "label:" )+name).ascii() );

  KonfiguratorURLRequester *fullPath = createURLRequester( "Dependencies", name, dflt, parent, false, page );
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

int KgDependencies::activeSubPage() {
  return tabWidget->currentPageIndex();
}

#include "kgdependencies.moc"
