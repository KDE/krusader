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
#include <QGridLayout>
#include <QScrollArea>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurl.h>

#define PAGE_GENERAL   0
#define PAGE_PACKERS   1
#define PAGE_CHECKSUM  2

KgDependencies::KgDependencies( bool first, QWidget* parent ) :
      KonfiguratorPage( first, parent )
{
  QGridLayout *kgDependenciesLayout = new QGridLayout( this );
  kgDependenciesLayout->setSpacing( 6 );

  //  ---------------------------- GENERAL TAB -------------------------------------
  tabWidget = new QTabWidget( this );

  QWidget *general_tab = new QWidget( tabWidget );
  QScrollArea* general_scroll = new QScrollArea( tabWidget );
  general_scroll->setWidget( general_tab ); // this also sets scrollacrea as the new parent for widget
  general_scroll->setWidgetResizable( true ); // let the widget use every space available
  tabWidget->insertTab( general_scroll, i18n( "General" ) );

  QGridLayout *pathsGrid = new QGridLayout( general_tab );
  pathsGrid->setSpacing( 6 );
  pathsGrid->setContentsMargins( 11, 11, 11, 11 );
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
  QWidget *packers_tab = new QWidget( tabWidget );
  QScrollArea* packers_scroll = new QScrollArea( tabWidget );
  packers_scroll->setWidget( packers_tab ); // this also sets scrollacrea as the new parent for widget
  packers_scroll->setWidgetResizable( true ); // let the widget use every space available
  tabWidget->insertTab( packers_scroll, i18n( "Packers" ) );

  QGridLayout *archGrid1 = new QGridLayout( packers_tab );
  archGrid1->setSpacing( 6 );
  archGrid1->setContentsMargins( 11, 11, 11, 11 );
  archGrid1->setAlignment( Qt::AlignTop );

  addApplication( "7z",    archGrid1, 0, packers_tab, PAGE_PACKERS, "7za" );
  addApplication( "arj",   archGrid1, 1, packers_tab, PAGE_PACKERS );
  addApplication( "bzip2", archGrid1, 2, packers_tab, PAGE_PACKERS );
  addApplication( "cpio",  archGrid1, 3, packers_tab, PAGE_PACKERS );
  addApplication( "dpkg",  archGrid1, 4, packers_tab, PAGE_PACKERS );
  addApplication( "gzip",  archGrid1, 5, packers_tab, PAGE_PACKERS );
  addApplication( "lha",   archGrid1, 6, packers_tab, PAGE_PACKERS );
  addApplication( "rar",   archGrid1, 7, packers_tab, PAGE_PACKERS );
  addApplication( "tar",   archGrid1, 8, packers_tab, PAGE_PACKERS );
  addApplication( "unace", archGrid1, 9, packers_tab, PAGE_PACKERS );
  addApplication( "unarj", archGrid1,10, packers_tab, PAGE_PACKERS );
  addApplication( "unrar", archGrid1,11, packers_tab, PAGE_PACKERS );
  addApplication( "unzip", archGrid1,12, packers_tab, PAGE_PACKERS );
  addApplication( "zip",   archGrid1,13, packers_tab, PAGE_PACKERS );

  //  ---------------------------- CHECKSUM TAB -------------------------------------
  QWidget *checksum_tab = new QWidget( tabWidget );
  QScrollArea* checksum_scroll = new QScrollArea( tabWidget );
  checksum_scroll->setWidget( checksum_tab ); // this also sets scrollacrea as the new parent for widget
  checksum_scroll->setWidgetResizable( true ); // let the widget use every space available
  tabWidget->insertTab( checksum_scroll, i18n( "Checksum Utilities" ) );

  QGridLayout *archGrid2 = new QGridLayout( checksum_tab );
  archGrid2->setSpacing( 6 );
  archGrid2->setContentsMargins( 11, 11, 11, 11 );
  archGrid2->setAlignment( Qt::AlignTop );

  addApplication( "md5sum",         archGrid2, 0, checksum_tab, PAGE_CHECKSUM );
  addApplication( "sha1sum",        archGrid2, 1, checksum_tab, PAGE_CHECKSUM );
  addApplication( "sha224sum",      archGrid2, 2, checksum_tab, PAGE_CHECKSUM );
  addApplication( "sha256sum",      archGrid2, 3, checksum_tab, PAGE_CHECKSUM );
  addApplication( "sha384sum",      archGrid2, 4, checksum_tab, PAGE_CHECKSUM );
  addApplication( "sha512sum",      archGrid2, 5, checksum_tab, PAGE_CHECKSUM );
  addApplication( "md5deep",        archGrid2, 6, checksum_tab, PAGE_CHECKSUM );
  addApplication( "sha1deep",       archGrid2, 7, checksum_tab, PAGE_CHECKSUM );
  addApplication( "sha256deep",     archGrid2, 8, checksum_tab, PAGE_CHECKSUM );
  addApplication( "tigerdeep",      archGrid2, 9, checksum_tab, PAGE_CHECKSUM );
  addApplication( "whirlpooldeep",  archGrid2, 10, checksum_tab, PAGE_CHECKSUM );
  addApplication( "cfv",            archGrid2, 11, checksum_tab, PAGE_CHECKSUM );

  kgDependenciesLayout->addWidget( tabWidget, 0, 0 );
}

void KgDependencies::addApplication( QString name, QGridLayout *grid, int row, QWidget *parent, int page, QString additionalList )
{
  QString dflt = KrServices::fullPathName( name ); /* try to autodetect the full path name */

  if( dflt.isEmpty() ) {
    QStringList list = additionalList.split( ',' );
    for( int i=0; i != list.count(); i++ )
      if( !KrServices::fullPathName( list[ i ] ).isEmpty() ) {
        dflt = KrServices::fullPathName( list[ i ] );
        break;
      }
  }

  addLabel( grid, row, 0, name, parent );

  KonfiguratorURLRequester *fullPath = createURLRequester( "Dependencies", name, dflt, parent, false, page );
  connect( fullPath->extension(), SIGNAL( applyManually( QObject *, QString, QString ) ),
           this, SLOT( slotApply( QObject *, QString, QString ) ) );
  grid->addWidget( fullPath, row, 1 );
}

void KgDependencies::slotApply( QObject *obj, QString cls, QString name )
{
  KonfiguratorURLRequester *urlRequester = (KonfiguratorURLRequester *) obj;

  KConfigGroup group( krConfig, cls );
  group.writeEntry( name, urlRequester->url().pathOrUrl() );

  QString usedPath = KrServices::fullPathName( name );

  if( urlRequester->url() != usedPath )
  {
    group.writeEntry( name, usedPath );
    if( usedPath.isEmpty() )
      KMessageBox::error( this, i18n( "The %1 path is incorrect, no valid path found.",
                                      urlRequester->url().pathOrUrl() ) );
    else
      KMessageBox::error( this, i18n( "The %1 path is incorrect, %2 used instead.",
                                      urlRequester->url().pathOrUrl() ).arg( usedPath ) );
    urlRequester->setUrl( KUrl( usedPath ) );
  }
}

int KgDependencies::activeSubPage() {
  return tabWidget->currentPageIndex();
}

#include "kgdependencies.moc"
