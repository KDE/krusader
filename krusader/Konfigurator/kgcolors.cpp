/***************************************************************************
                          kgcolors.cpp  -  description
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

#include "kgcolors.h"
#include "../defaults.h"
#include <klocale.h>
#include <kglobalsettings.h>

KgColors::KgColors( bool first, QWidget* parent,  const char* name ) :
      KonfiguratorPage( first, parent, name )
{
  QGridLayout *kgColorsLayout = new QGridLayout( parent );
  kgColorsLayout->setSpacing( 6 );
  kgColorsLayout->setMargin( 5 );

  //  -------------------------- GENERAL GROUPBOX ----------------------------------

  QGroupBox *generalGrp = createFrame( i18n( "General" ), parent, "kgColorsGeneralGrp" );
  QGridLayout *generalGrid = createGridLayout( generalGrp->layout() );

  KONFIGURATOR_CHECKBOX_PARAM generalSettings[] =
  //  cfg_class  cfg_name                     default               text                                      restart tooltip
    {{"Colors","KDE Default",                 _KDEDefaultColors,    i18n( "Use the default colors of KDE" ),  true ,  "" },
     {"Colors","Enable Alternate Background", _AlternateBackground, i18n( "Use alternate backround color" ),  true ,  "" }};

  KonfiguratorCheckBoxGroup *generals = createCheckBoxGroup( 1, 0, generalSettings, 2, generalGrp );
  generalGrid->addWidget( generals, 1, 0 );

  kgColorsLayout->addWidget( generalGrp, 0 ,0 );

  //  -------------------------- COLORS GROUPBOX ----------------------------------

  QGroupBox *colorsGrp = createFrame( i18n( "Colors" ), parent, "kgColorsColorsGrp" );
  QGridLayout *colorsGrid = createGridLayout( colorsGrp->layout() );

  colorsGrid->setSpacing( 0 );
  colorsGrid->setMargin( 5 );

  addLabel( colorsGrid, 0, 0, i18n( "Foreground:" ), colorsGrp, "ColorsLabel1" );
  foreground = createColorChooser( "Colors", "Foreground", KGlobalSettings::textColor(), colorsGrp, true );
  colorsGrid->addWidget( foreground, 0, 1 );

  addLabel( colorsGrid, 1, 0, i18n( "Directory foreground:" ), colorsGrp, "ColorsLabel2" );
  directoryForeground = createColorChooser( "Colors", "Directory Foreground", foreground->getColor(), colorsGrp, true );
  directoryForeground->setDefaultText( i18n( "Same as foreground" ) );
  colorsGrid->addWidget( directoryForeground, 1, 1 );

  addLabel( colorsGrid, 2, 0, i18n( "Executable foreground:" ), colorsGrp, "ColorsLabel3" );
  executableForeground = createColorChooser( "Colors", "Executable Foreground", foreground->getColor(), colorsGrp, true );
  executableForeground->setDefaultText( i18n( "Same as foreground" ) );
  colorsGrid->addWidget( executableForeground, 2, 1 );

  addLabel( colorsGrid, 3, 0, i18n( "Symbolic link foreground:" ), colorsGrp, "ColorsLabel4" );
  symlinkForeground = createColorChooser( "Colors", "Symlink Foreground", foreground->getColor(), colorsGrp, true );
  symlinkForeground->setDefaultText( i18n( "Same as foreground" ) );
  colorsGrid->addWidget( symlinkForeground, 3, 1 );

  addLabel( colorsGrid, 4, 0, i18n( "Invalid Symlink foreground:" ), colorsGrp, "ColorsLabel5" );
  invalidSymlinkForeground = createColorChooser( "Colors", "Invalid Symlink Foreground", foreground->getColor(), colorsGrp, true );
  invalidSymlinkForeground->setDefaultText( i18n( "Same as foreground" ) );
  colorsGrid->addWidget( invalidSymlinkForeground, 4, 1 );

  addLabel( colorsGrid, 5, 0, i18n( "Marked foreground:" ), colorsGrp, "ColorsLabel6" );
  markedForeground = createColorChooser( "Colors", "Marked Foreground", KGlobalSettings::highlightedTextColor(), colorsGrp, true );
  colorsGrid->addWidget( markedForeground, 5, 1 );

  addLabel( colorsGrid, 6, 0, i18n( "Marked background:" ), colorsGrp, "ColorsLabel7" );
  markedBackround = createColorChooser( "Colors", "Marked Background", KGlobalSettings::highlightColor(), colorsGrp, true );
  colorsGrid->addWidget( markedBackround, 6, 1 );

  addLabel( colorsGrid, 7, 0, i18n( "Alternate marked background:" ), colorsGrp, "ColorsLabel7" );
  alternateMarkedBackround = createColorChooser( "Colors", "Alternate Marked Background", markedBackround->getColor(), colorsGrp, true );
  alternateMarkedBackround->setDefaultText( i18n( "Same as marked background" ) );
  colorsGrid->addWidget( alternateMarkedBackround, 7, 1 );

  addLabel( colorsGrid, 8, 0, i18n( "Current foreground:" ), colorsGrp, "ColorsLabel8" );
  currentForeground = createColorChooser( "Colors", "Current Foreground", Qt::white, colorsGrp, true );
  currentForeground->setDefaultText( i18n( "Not used" ) );
  colorsGrid->addWidget( currentForeground, 8, 1 );

  addLabel( colorsGrid, 9, 0, i18n( "Current background:" ), colorsGrp, "ColorsLabel9" );
  currentBackround = createColorChooser( "Colors", "Current Background", Qt::white, colorsGrp, true );
  currentBackround->setDefaultText( i18n( "Not used" ) );
  colorsGrid->addWidget( currentBackround, 9, 1 );

  addLabel( colorsGrid, 10, 0, i18n( "Background:" ), colorsGrp, "ColorsLabel10" );
  backround = createColorChooser( "Colors", "Background", KGlobalSettings::baseColor(), colorsGrp, true );
  colorsGrid->addWidget( backround, 10, 1 );

  addLabel( colorsGrid, 11, 0, i18n( "Alternate background:" ), colorsGrp, "ColorsLabel11" );
  alternateBackround = createColorChooser( "Colors", "Alternate Background", KGlobalSettings::alternateBackgroundColor(), colorsGrp, true );
  colorsGrid->addWidget( alternateBackround, 11, 1 );

  kgColorsLayout->addWidget( colorsGrp, 1 ,0 );

} 
 
#include "kgcolors.moc"

