/***************************************************************************
                        kgwelcome.cpp  -  description
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

#include "kgwelcome.h"
#include <kstandarddirs.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3GridLayout>
#include <QLabel>

KgWelcome::KgWelcome( bool first, QWidget* parent,  const char* name ) :
  KonfiguratorPage( first, parent, name )
{
  Q3GridLayout *kgWelcomeLayout = new Q3GridLayout( parent );
  kgWelcomeLayout->setSpacing( 6 );

  QString pix=KGlobal::dirs()->findResource("appdata","konfig_small.jpg");
  QPixmap image0( pix );
  
  QLabel *pixmapLabel = new QLabel( parent, "pixmapLabel" );
  pixmapLabel->setPixmap( image0 );
  pixmapLabel->setScaledContents( TRUE );

  kgWelcomeLayout->addWidget( pixmapLabel, 0, 0 );
}

#include "kgwelcome.moc"
