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
#include "../Panel/krcolorcache.h"
#include <klocale.h>
#include <kglobalsettings.h>
#include <qhbox.h>
#include <qheader.h>

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
    {{"Colors","KDE Default",                 _KDEDefaultColors,    i18n( "Use the default KDE colors" ),     false,  "" },
     {"Colors","Enable Alternate Background", _AlternateBackground, i18n( "Use alternate backround color" ),  false,  "" },
     {"Colors","Show Current Item Always", _ShowCurrentItemAlways, i18n( "Show current item even if not focussed" ),  false,  "" }};

  generals = createCheckBoxGroup( 0, 2, generalSettings, sizeof(generalSettings)/sizeof(generalSettings[0]), generalGrp );
  generalGrid->addWidget( generals, 1, 0 );

  connect( generals->find( "KDE Default" ), SIGNAL( stateChanged( int ) ), this, SLOT( slotDisable() ) );
  connect( generals->find( "Enable Alternate Background" ), SIGNAL( stateChanged( int ) ), this, SLOT( generatePreview() ) );

  kgColorsLayout->addWidget( generalGrp, 0 ,0 );
  QHBox *hbox = new QHBox( parent );

  //  -------------------------- COLORS GROUPBOX ----------------------------------
  
  colorsGrp = createFrame( i18n( "Colors" ), hbox, "kgColorsColorsGrp" );
  colorsGrid = createGridLayout( colorsGrp->layout() );

  colorsGrid->setSpacing( 0 );
  colorsGrid->setMargin( 5 );

  ADDITIONAL_COLOR transparent = { i18n("Transparent"), Qt::white, "transparent" };

  addColorSelector( "Foreground",                 i18n( "Foreground:" ),                  KGlobalSettings::textColor()                                                );
  addColorSelector( "Directory Foreground",       i18n( "Directory foreground:" ),        getColorSelector( "Foreground" )->getColor(), i18n( "Same as foreground" )  );
  addColorSelector( "Executable Foreground",      i18n( "Executable foreground:" ),       getColorSelector( "Foreground" )->getColor(), i18n( "Same as foreground" )  );
  addColorSelector( "Symlink Foreground",         i18n( "Symbolic link foreground:" ),    getColorSelector( "Foreground" )->getColor(), i18n( "Same as foreground" )  );
  addColorSelector( "Invalid Symlink Foreground", i18n( "Invalid symlink foreground:" ),  getColorSelector( "Foreground" )->getColor(), i18n( "Same as foreground" )  );
  addColorSelector( "Background",                 i18n( "Background:" ),                  KGlobalSettings::baseColor()                                                );
  addColorSelector( "Alternate Background",       i18n( "Alternate background:" ),        KGlobalSettings::alternateBackgroundColor()                                 );
  addColorSelector( "Marked Foreground",          i18n( "Marked foreground:" ),           KGlobalSettings::highlightedTextColor(), "", &transparent, 1                );
  addColorSelector( "Marked Background",          i18n( "Marked background:" ),           KGlobalSettings::highlightColor()                                           );
  addColorSelector( "Alternate Marked Background",i18n( "Alternate marked background:" ), getColorSelector( "Marked Background" )->getColor(), i18n( "Same as marked background" )  );
  addColorSelector( "Current Foreground",         i18n( "Current foreground:" ),          Qt::white,                                    i18n( "Not used" )            );
  addColorSelector( "Current Background",         i18n( "Current background:" ),          Qt::white,                                    i18n( "Not used" )            );

  connect( getColorSelector( "Foreground" ), SIGNAL( colorChanged() ), this, SLOT( slotForegroundChanged() ) );
  connect( getColorSelector( "Marked Background" ), SIGNAL( colorChanged() ), this, SLOT( slotMarkedBackgroundChanged() ) );

  //  -------------------------- PREVIEW GROUPBOX ----------------------------------

  previewGrp = createFrame( i18n( "Preview" ), hbox, "kgColorsPreviewGrp" );
  previewGrid = createGridLayout( previewGrp->layout() );

  preview = new QListView( previewGrp, "colorPreView" );
  
  preview->setShowSortIndicator(false);
  preview->setSorting(-1);
  preview->setEnabled( false );

  preview->addColumn( i18n("Colors") );
  preview->header()->setStretchEnabled( true, 0 );

  pwMark2   = new PreviewItem( preview, i18n( "Marked 2" ) );
  pwMark1   = new PreviewItem( preview, i18n( "Marked 1" ) );
  pwCurrent = new PreviewItem( preview, i18n( "Current" ) );
  pwInvLink = new PreviewItem( preview, i18n( "Invalid symlink" ) );
  pwSymLink = new PreviewItem( preview, i18n( "Symbolic link" ) );
  pwApp     = new PreviewItem( preview, i18n( "Application" ) );
  pwFile    = new PreviewItem( preview, i18n( "File" ) );
  pwDir     = new PreviewItem( preview, i18n( "Directory" ) );
  
  generatePreview();

  previewGrid->addWidget( preview, 0 ,0 );
  
  kgColorsLayout->addWidget( hbox, 1 ,0 );
  slotDisable();
} 

int KgColors::addColorSelector( QString cfgName, QString name, QColor dflt, QString dfltName,
                                ADDITIONAL_COLOR *addColor, int addColNum )
{
  int index = itemList.count();

  labelList.append( addLabel( colorsGrid, index, 0, name, colorsGrp, QString( "ColorsLabel%1" ).arg( index ).ascii() ) );
  KonfiguratorColorChooser *chooser = createColorChooser( "Colors", cfgName, dflt, colorsGrp, false, addColor, addColNum );
  if( !dfltName.isEmpty() )
    chooser->setDefaultText( dfltName );
  colorsGrid->addWidget( chooser, index, 1 );

  connect( chooser, SIGNAL( colorChanged() ), this, SLOT( generatePreview() ) );
  
  itemList.append( chooser );
  itemNames.append( cfgName );

  return index;
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
  
  for( int i = 0; labelList.at( i ); i++ )
    labelList.at( i )->setEnabled( !enabled );

  for( int j = 0; itemList.at( j ); j++ )
    itemList.at( j )->setEnabled( !enabled );

  generals->find("Enable Alternate Background")->setEnabled( enabled );
  generals->find("Show Current Item Always")->setEnabled( !enabled );
  generatePreview();
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

void KgColors::generatePreview()
{
  if( generals->find( "KDE Default" )->isChecked() )
  {
    QColor bck    = KGlobalSettings::baseColor();
    QColor altBck = KGlobalSettings::alternateBackgroundColor();
    if( !generals->find("Enable Alternate Background")->isChecked() )
      altBck = bck;
    QColor fore   = KGlobalSettings::textColor();

    pwFile->setColor( fore, altBck );
    pwDir->setColor( fore, bck );
    pwApp->setColor( fore, bck );
    pwSymLink->setColor( fore, altBck );
    pwInvLink->setColor( fore, bck );
    pwCurrent->setColor( fore, altBck );
    pwMark1->setColor( KGlobalSettings::highlightedTextColor(), KGlobalSettings::highlightColor() );
    pwMark2->setColor( KGlobalSettings::highlightedTextColor(), KGlobalSettings::highlightColor() );
  }
  else
  {
    QColor  bck   = getColorSelector( "Background" )->getColor();
    QColor altBck = getColorSelector( "Alternate Background" )->getColor();

    QColor currentFore;
    QColor currentBck = altBck;

    pwFile->setColor( currentFore = getColorSelector( "Foreground" )->getColor(), altBck );
    pwDir->setColor( getColorSelector( "Directory Foreground" )->getColor(), bck );
    pwApp->setColor( getColorSelector( "Executable Foreground" )->getColor(), bck );
    pwSymLink->setColor( getColorSelector( "Symlink Foreground" )->getColor(), altBck );
    pwInvLink->setColor( getColorSelector( "Invalid Symlink Foreground" )->getColor(), bck );

    if( getColorSelector( "Current Foreground" )->currentItem() != 1 )
      currentFore = getColorSelector( "Current Foreground" )->getColor();
    if( getColorSelector( "Current Background" )->currentItem() != 1 )
      currentBck = getColorSelector( "Current Background" )->getColor();
      
    pwCurrent->setColor( currentFore, currentBck );

    QColor markFore = getColorSelector( "Marked Foreground" )->getColor();    
    if( getColorSelector( "Marked Foreground" )->currentItem() == 2 )
      markFore = currentFore;    
    pwMark1->setColor( markFore, getColorSelector( "Marked Background" )->getColor() );
    pwMark2->setColor( markFore, getColorSelector( "Alternate Marked Background" )->getColor() );
  }
}

bool KgColors::apply()
{
  bool result = KonfiguratorPage::apply();
  KrColorCache::getColorCache().refreshColors();
  return result;
}

#include "kgcolors.moc"

