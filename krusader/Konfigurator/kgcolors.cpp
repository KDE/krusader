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
#include <qhbox.h>

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
    {{"Colors","KDE Default",                 _KDEDefaultColors,    i18n( "Use the default KDE colors" ),     true ,  "" },
     {"Colors","Enable Alternate Background", _AlternateBackground, i18n( "Use alternate backround color" ),  true ,  "" }};

  generals = createCheckBoxGroup( 1, 0, generalSettings, 2, generalGrp );
  generalGrid->addWidget( generals, 1, 0 );

  connect( generals->find( "KDE Default" ), SIGNAL( stateChanged( int ) ), this, SLOT( slotDisable() ) );
  connect( generals->find( "Enable Alternate Background" ), SIGNAL( stateChanged( int ) ), this, SLOT( slotDisable() ) );

  kgColorsLayout->addWidget( generalGrp, 0 ,0 );
  QHBox *hbox = new QHBox( parent );

  //  -------------------------- COLORS GROUPBOX ----------------------------------
  
  colorsGrp = createFrame( i18n( "Colors" ), hbox, "kgColorsColorsGrp" );
  colorsGrid = createGridLayout( colorsGrp->layout() );

  colorsGrid->setSpacing( 0 );
  colorsGrid->setMargin( 5 );

  addColorSelector( "Foreground",                 i18n( "Foreground:" ),                  KGlobalSettings::textColor()                                                );
  addColorSelector( "Directory Foreground",       i18n( "Directory foreground:" ),        getColorSelector( "Foreground" )->getColor(), i18n( "Same as foreground" )  );
  addColorSelector( "Executable Foreground",      i18n( "Executable foreground:" ),       getColorSelector( "Foreground" )->getColor(), i18n( "Same as foreground" )  );
  addColorSelector( "Symlink Foreground",         i18n( "Symbolic link foreground:" ),    getColorSelector( "Foreground" )->getColor(), i18n( "Same as foreground" )  );
  addColorSelector( "Invalid Symlink Foreground", i18n( "Invalid symlink foreground:" ),  getColorSelector( "Foreground" )->getColor(), i18n( "Same as foreground" )  );
  addColorSelector( "Marked Foreground",          i18n( "Marked foreground:" ),           KGlobalSettings::highlightedTextColor()                                     );
  addColorSelector( "Marked Background",          i18n( "Marked background:" ),           KGlobalSettings::highlightColor()                                           );
  addColorSelector( "Alternate Marked Background",i18n( "Alternate marked background:" ), getColorSelector( "Marked Background" )->getColor(), i18n( "Same as marked background" )  );
  addColorSelector( "Current Foreground",         i18n( "Current foreground:" ),          Qt::white,                                    i18n( "Not used" )            );
  addColorSelector( "Current Background",         i18n( "Current background:" ),          Qt::white,                                    i18n( "Not used" )            );
  addColorSelector( "Background",                 i18n( "Background:" ),                  KGlobalSettings::baseColor()                                                );
  addColorSelector( "Alternate Background",       i18n( "Alternate background:" ),        KGlobalSettings::alternateBackgroundColor()                                 );

  connect( getColorSelector( "Foreground" ), SIGNAL( colorChanged() ), this, SLOT( slotForegroundChanged() ) );
  connect( getColorSelector( "Marked Background" ), SIGNAL( colorChanged() ), this, SLOT( slotMarkedBackgroundChanged() ) );

  //  -------------------------- PREVIEW GROUPBOX ----------------------------------

  previewGrp = createFrame( i18n( "Preview" ), hbox, "kgColorsPreviewGrp" );
  previewGrid = createGridLayout( previewGrp->layout() );

  /* TODO TODO TODO TODO TODO */
  
  kgColorsLayout->addWidget( hbox, 1 ,0 );
} 

int KgColors::addColorSelector( QString cfgName, QString name, QColor dflt, QString dfltName )
{
  int index = itemList.count();

  labelList.append( addLabel( colorsGrid, index, 0, name, colorsGrp, QString( "ColorsLabel%1" ).arg( index ).ascii() ) );
  KonfiguratorColorChooser *chooser = createColorChooser( "Colors", cfgName, dflt, colorsGrp, true );
  if( !dfltName.isEmpty() )
    chooser->setDefaultText( dfltName );
  colorsGrid->addWidget( chooser, index, 1 );

  itemList.append( chooser );
  itemNames.append( cfgName );
}

KonfiguratorColorChooser *KgColors::getColorSelector( QString name )
{
  QValueList<QString>::iterator it;
  int position = 0;
  
  for( it = itemNames.begin(); it != itemNames.end(); it++, position++ )
    if( *it == name )
      return itemList.at( position );

  return 0;
}

QLabel *KgColors::getSelectorLabel( QString name )
{
  QValueList<QString>::iterator it;
  int position = 0;

  for( it = itemNames.begin(); it != itemNames.end(); it++, position++ )
    if( *it == name )
      return labelList.at( position );

  return 0;
}

void KgColors::slotDisable()
{
  bool enabled = generals->find( "KDE Default" )->isChecked();
  bool alternateEnabled = enabled && generals->find( "Enable Alternate Background" )->isChecked();
  
  for( int i = 0; labelList.at( i ); i++ )
    labelList.at( i )->setEnabled( enabled );

  for( int j = 0; itemList.at( j ); j++ )
    itemList.at( j )->setEnabled( enabled );

  getColorSelector( "Alternate Background" )->setEnabled( alternateEnabled );
  getSelectorLabel( "Alternate Background" )->setEnabled( alternateEnabled );
  getColorSelector( "Alternate Marked Background" )->setEnabled( alternateEnabled );
  getSelectorLabel( "Alternate Marked Background" )->setEnabled( alternateEnabled );
}

void KgColors::slotForegroundChanged()
{
  QColor color = getColorSelector( "Foreground" )->getColor();

  getColorSelector( "Directory Foreground" )->setDefaultColor( color );
  getColorSelector( "Executable Foreground" )->setDefaultColor( color );
  getColorSelector( "Symlink Foreground" )->setDefaultColor( color );
  getColorSelector( "Invalid Symlink Foreground" )->setDefaultColor( color );
}

void KgColors::slotMarkedBackgroundChanged()
{
  QColor color = getColorSelector( "Marked Background" )->getColor();

  getColorSelector( "Alternate Marked Background" )->setDefaultColor( color );
}

#include "kgcolors.moc"

