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
#include <qtabwidget.h>

KgColors::KgColors( bool first, QWidget* parent,  const char* name ) :
      KonfiguratorPage( first, parent, name ), offset( 0 )
{
  QGridLayout *kgColorsLayout = new QGridLayout( parent );
  kgColorsLayout->setSpacing( 6 );
  kgColorsLayout->setMargin( 5 );

  //  -------------------------- GENERAL GROUPBOX ----------------------------------

  QGroupBox *generalGrp = createFrame( i18n( "General" ), parent, "kgColorsGeneralGrp" );
  QGridLayout *generalGrid = createGridLayout( generalGrp->layout() );

  generalGrid->setSpacing( 0 );
  generalGrid->setMargin( 5 );

  KONFIGURATOR_CHECKBOX_PARAM generalSettings[] =
  //  cfg_class  cfg_name                     default               text                                      restart tooltip
    {{"Colors","KDE Default",                 _KDEDefaultColors,    i18n( "Use the default KDE colors" ),     false,  "" },
     {"Colors","Enable Alternate Background", _AlternateBackground, i18n( "Use alternate backround color" ),  false,  "" },
     {"Colors","Show Current Item Always", _ShowCurrentItemAlways, i18n( "Show current item even if not focused" ),  false,  "" }};

  generals = createCheckBoxGroup( 0, 2, generalSettings, sizeof(generalSettings)/sizeof(generalSettings[0]), generalGrp );
  generalGrid->addWidget( generals, 1, 0 );

  generals->layout()->setSpacing( 5 );
  
  connect( generals->find( "KDE Default" ), SIGNAL( stateChanged( int ) ), this, SLOT( slotDisable() ) );
  connect( generals->find( "Enable Alternate Background" ), SIGNAL( stateChanged( int ) ), this, SLOT( generatePreview() ) );
  connect( generals->find( "Show Current Item Always" ), SIGNAL( stateChanged( int ) ), this, SLOT( slotDisable() ) );

  kgColorsLayout->addWidget( generalGrp, 0 ,0 );
  QHBox *hbox = new QHBox( parent );

  //  -------------------------- COLORS GROUPBOX ----------------------------------
  
  QGroupBox *colorsFrameGrp = createFrame( i18n( "Colors" ), hbox, "kgColorsColorsGrp" );
  QGridLayout *colorsFrameGrid = createGridLayout( colorsFrameGrp->layout() );
  colorsFrameGrid->setSpacing( 0 );
  colorsFrameGrid->setMargin( 3 );

  colorTabWidget = new QTabWidget( colorsFrameGrp, "colorTabWidget" );

  connect( colorTabWidget, SIGNAL( currentChanged ( QWidget * ) ), this, SLOT( generatePreview() ) );
  
  colorsGrp = new QWidget( colorTabWidget, "colorTab" );
  colorTabWidget->insertTab( colorsGrp, i18n( "Active" ) );

  colorsGrid = new QGridLayout( colorsGrp );
  colorsGrid->setSpacing( 0 );
  colorsGrid->setMargin( 2 );

  ADDITIONAL_COLOR transparent  = { i18n("Transparent"),       Qt::white, "transparent" };

  addColorSelector( "Foreground",                 i18n( "Foreground:" ),                  KGlobalSettings::textColor()                                                );
  addColorSelector( "Directory Foreground",       i18n( "Directory foreground:" ),        getColorSelector( "Foreground" )->getColor(), i18n( "Same as foreground" )  );
  addColorSelector( "Executable Foreground",      i18n( "Executable foreground:" ),       getColorSelector( "Foreground" )->getColor(), i18n( "Same as foreground" )  );
  addColorSelector( "Symlink Foreground",         i18n( "Symbolic link foreground:" ),    getColorSelector( "Foreground" )->getColor(), i18n( "Same as foreground" )  );
  addColorSelector( "Invalid Symlink Foreground", i18n( "Invalid symlink foreground:" ),  getColorSelector( "Foreground" )->getColor(), i18n( "Same as foreground" )  );
  addColorSelector( "Background",                 i18n( "Background:" ),                  KGlobalSettings::baseColor()                                                );
  ADDITIONAL_COLOR sameAsBckgnd = { i18n("Same as background"), getColorSelector( "Background" )->getColor(), "Background" };
  addColorSelector( "Alternate Background",       i18n( "Alternate background:" ),        KGlobalSettings::alternateBackgroundColor(),"", &sameAsBckgnd, 1            );
  addColorSelector( "Marked Foreground",          i18n( "Marked foreground:" ),           KGlobalSettings::highlightedTextColor(), "", &transparent, 1                );
  addColorSelector( "Marked Background",          i18n( "Marked background:" ),           KGlobalSettings::highlightColor(), "", &sameAsBckgnd, 1                     );
  ADDITIONAL_COLOR sameAsAltern = { i18n("Same as alt. background"), getColorSelector( "Alternate Background" )->getColor(), "Alternate Background" };
  addColorSelector( "Alternate Marked Background",i18n( "Alternate marked background:" ), getColorSelector( "Marked Background" )->getColor(), i18n( "Same as marked background" ), &sameAsAltern, 1 );
  addColorSelector( "Current Foreground",         i18n( "Current foreground:" ),          Qt::white,                                    i18n( "Not used" )            );
  addColorSelector( "Current Background",         i18n( "Current background:" ),          Qt::white, i18n( "Not used" ), &sameAsBckgnd, 1                             );

  connect( getColorSelector( "Foreground" ), SIGNAL( colorChanged() ), this, SLOT( slotForegroundChanged() ) );
  connect( getColorSelector( "Background" ), SIGNAL( colorChanged() ), this, SLOT( slotBackgroundChanged() ) );
  connect( getColorSelector( "Alternate Background" ), SIGNAL( colorChanged() ), this, SLOT( slotAltBackgroundChanged() ) );
  connect( getColorSelector( "Marked Background" ), SIGNAL( colorChanged() ), this, SLOT( slotMarkedBackgroundChanged() ) );

  colorsGrp = new QWidget( colorTabWidget, "colorTab" );
  colorTabWidget->insertTab( colorsGrp, i18n( "Inactive" ) );

  colorsGrid = new QGridLayout( colorsGrp );
  colorsGrid->setSpacing( 0 );
  colorsGrid->setMargin( 2 );

  offset = itemList.count();
  
  addColorSelector( "Inactive Foreground",                  i18n( "Foreground:" ),                  getColorSelector( "Foreground" )->getColor(), i18n( "Same as active" ) );
  ADDITIONAL_COLOR sameAsInactForegnd = { i18n("Same as foreground"), getColorSelector( "Inactive Foreground" )->getColor(), "Inactive Foreground" };
  addColorSelector( "Inactive Directory Foreground",        i18n( "Directory foreground:" ),        getColorSelector( "Directory Foreground" )->getColor(), i18n( "Same as active" ), &sameAsInactForegnd, 1 );
  addColorSelector( "Inactive Executable Foreground",       i18n( "Executable foreground:" ),       getColorSelector( "Executable Foreground" )->getColor(), i18n( "Same as active" ), &sameAsInactForegnd, 1 );
  addColorSelector( "Inactive Symlink Foreground",          i18n( "Symbolic link foreground:" ),    getColorSelector( "Symlink Foreground" )->getColor(), i18n( "Same as active" ), &sameAsInactForegnd, 1 );
  addColorSelector( "Inactive Invalid Symlink Foreground",  i18n( "Invalid symlink foreground:" ),  getColorSelector( "Invalid Symlink Foreground" )->getColor(), i18n( "Same as active" ), &sameAsInactForegnd, 1 );
  addColorSelector( "Inactive Background",                  i18n( "Background:" ),                  getColorSelector( "Background" )->getColor(), i18n( "Same as active" ) );
  ADDITIONAL_COLOR sameAsInactBckgnd = { i18n("Same as background"), getColorSelector( "Inactive Background" )->getColor(), "Inactive Background" };
  addColorSelector( "Inactive Alternate Background",        i18n( "Alternate background:" ),        getColorSelector( "Alternate Background" )->getColor(), i18n( "Same as active" ), &sameAsInactBckgnd, 1 );
  addColorSelector( "Inactive Marked Foreground",           i18n( "Marked foreground:" ),           getColorSelector( "Marked Foreground" )->getColor(), i18n( "Same as active" ), &transparent, 1 );
  addColorSelector( "Inactive Marked Background",           i18n( "Marked background:" ),           getColorSelector( "Marked Background" )->getColor(), i18n( "Same as active" ), &sameAsInactBckgnd, 1 );
  ADDITIONAL_COLOR sameAsInactAltern[] = {{ i18n("Same as alt. background"), getColorSelector( "Inactive Alternate Background" )->getColor(), "Inactive Alternate Background" },
                                          { i18n("Same as marked background"), getColorSelector( "Inactive Marked Background" )->getColor(), "Inactive Marked Background" } };
  addColorSelector( "Inactive Alternate Marked Background", i18n( "Alternate marked background:" ), getColorSelector( "Alternate Marked Background" )->getColor(), i18n( "Same as active" ), sameAsInactAltern, 2 );
  addColorSelector( "Inactive Current Foreground",          i18n( "Current foreground:" ),          getColorSelector( "Current Foreground" )->getColor(), i18n( "Same as active" ) );
  addColorSelector( "Inactive Current Background",          i18n( "Current background:" ),          getColorSelector( "Current Background" )->getColor(), i18n( "Same as active" ), &sameAsInactBckgnd, 1 );

  connect( getColorSelector( "Inactive Foreground" ), SIGNAL( colorChanged() ), this, SLOT( slotInactiveForegroundChanged() ) );
  connect( getColorSelector( "Inactive Background" ), SIGNAL( colorChanged() ), this, SLOT( slotInactiveBackgroundChanged() ) );
  connect( getColorSelector( "Inactive Alternate Background" ), SIGNAL( colorChanged() ), this, SLOT( slotInactiveAltBackgroundChanged() ) );
  connect( getColorSelector( "Inactive Marked Background" ), SIGNAL( colorChanged() ), this, SLOT( slotInactiveMarkedBackgroundChanged() ) );
  
  colorsFrameGrid->addWidget( colorTabWidget, 0, 0 );
  
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
  
  previewGrid->addWidget( preview, 0 ,0 );
  
  kgColorsLayout->addWidget( hbox, 1 ,0 );

  slotDisable();
} 

int KgColors::addColorSelector( QString cfgName, QString name, QColor dflt, QString dfltName,
                                ADDITIONAL_COLOR *addColor, int addColNum )
{
  int index = itemList.count() - offset;

  labelList.append( addLabel( colorsGrid, index, 0, name, colorsGrp, QString( "ColorsLabel%1" ).arg( index ).ascii() ) );
  KonfiguratorColorChooser *chooser = createColorChooser( "Colors", cfgName, dflt, colorsGrp, false, addColor, addColNum );
  if( !dfltName.isEmpty() )
    chooser->setDefaultText( dfltName );
  colorsGrid->addWidget( chooser, index, 1 );

  connect( chooser, SIGNAL( colorChanged() ), this, SLOT( generatePreview() ) );
  if( !offset )
    connect( chooser, SIGNAL( colorChanged() ), this, SLOT( slotActiveChanged() ) );

  chooser->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );   
  
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

  enabled = enabled || !generals->find( "Show Current Item Always" )->isChecked();

  getColorSelector( "Inactive Current Foreground" )->setEnabled( !enabled );
  getColorSelector( "Inactive Current Background" )->setEnabled( !enabled );

  generatePreview();
}

void KgColors::slotActiveChanged()
{
  for( int i = 0; i != offset; i++ )
     itemList.at( offset + i )->setDefaultColor( itemList.at( i )->getColor() );
}

void KgColors::slotForegroundChanged()
{
  QColor color = getColorSelector( "Foreground" )->getColor();

  getColorSelector( "Directory Foreground" )->setDefaultColor( color );
  getColorSelector( "Executable Foreground" )->setDefaultColor( color );
  getColorSelector( "Symlink Foreground" )->setDefaultColor( color );
  getColorSelector( "Invalid Symlink Foreground" )->setDefaultColor( color );
}

void KgColors::slotBackgroundChanged()
{
  QColor color = getColorSelector( "Background" )->getColor();

  getColorSelector( "Alternate Background" )->changeAdditionalColor( 0, color );
  getColorSelector( "Marked Background" )->changeAdditionalColor( 0, color );
  getColorSelector( "Current Background" )->changeAdditionalColor( 0, color );
}

void KgColors::slotAltBackgroundChanged()
{
  QColor color = getColorSelector( "Alternate Background" )->getColor();
  getColorSelector( "Alternate Marked Background" )->changeAdditionalColor( 0, color );
}

void KgColors::slotMarkedBackgroundChanged()
{
  QColor color = getColorSelector( "Marked Background" )->getColor();
  getColorSelector( "Alternate Marked Background" )->setDefaultColor( color );
}

void KgColors::slotInactiveForegroundChanged()
{
  QColor color = getColorSelector( "Inactive Foreground" )->getColor();

  getColorSelector( "Inactive Directory Foreground" )->changeAdditionalColor( 0, color );
  getColorSelector( "Inactive Executable Foreground" )->changeAdditionalColor( 0, color );
  getColorSelector( "Inactive Symlink Foreground" )->changeAdditionalColor( 0, color );
  getColorSelector( "Inactive Invalid Symlink Foreground" )->changeAdditionalColor( 0, color );
}

void KgColors::slotInactiveBackgroundChanged()
{
  QColor color = getColorSelector( "Inactive Background" )->getColor();

  getColorSelector( "Inactive Alternate Background" )->changeAdditionalColor( 0, color );
  getColorSelector( "Inactive Marked Background" )->changeAdditionalColor( 0, color );
  getColorSelector( "Inactive Current Background" )->changeAdditionalColor( 0, color );
}

void KgColors::slotInactiveAltBackgroundChanged()
{
  QColor color = getColorSelector( "Inactive Alternate Background" )->getColor();
  getColorSelector( "Inactive Alternate Marked Background" )->changeAdditionalColor( 0, color );
}

void KgColors::slotInactiveMarkedBackgroundChanged()
{
  QColor color = getColorSelector( "Inactive Marked Background" )->getColor();
  getColorSelector( "Inactive Alternate Marked Background" )->changeAdditionalColor( 1, color );
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
    bool isInactive = colorTabWidget->currentPageIndex() == 1;
    QString prefix="";

    if( isInactive )
      prefix = "Inactive ";
    
    QColor  bck   = getColorSelector( prefix + "Background" )->getColor();
    QColor altBck = getColorSelector( prefix + "Alternate Background" )->getColor();

    QColor currentFore;
    QColor currentBck = altBck;

    pwFile->setColor( currentFore = getColorSelector( prefix + "Foreground" )->getColor(), altBck );
    pwDir->setColor( getColorSelector( prefix + "Directory Foreground" )->getColor(), bck );
    pwApp->setColor( getColorSelector( prefix + "Executable Foreground" )->getColor(), bck );
    pwSymLink->setColor( getColorSelector( prefix + "Symlink Foreground" )->getColor(), altBck );
    pwInvLink->setColor( getColorSelector( prefix + "Invalid Symlink Foreground" )->getColor(), bck );

    if( isInactive )
    {
      if( generals->find( "Show Current Item Always" )->isChecked() )
      {
        if( getColorSelector( "Inactive Current Foreground" )->currentItem() != 1 )
          currentFore = getColorSelector( "Inactive Current Foreground" )->getColor();
        else
        {
          if( getColorSelector( "Current Foreground" )->currentItem() != 1 )
            currentFore = getColorSelector( "Current Foreground" )->getColor();
        }
        if( getColorSelector( "Inactive Current Background" )->currentItem() != 1 )
          currentBck = getColorSelector( "Inactive Current Background" )->getColor();
        else
        {
          if( getColorSelector( "Current Background" )->currentItem() != 1 )
            currentBck = getColorSelector( "Current Background" )->getColor();
        }
      }
    }
    else
    {    
      if( getColorSelector( "Current Foreground" )->currentItem() != 1 )
        currentFore = getColorSelector( "Current Foreground" )->getColor();
      if( getColorSelector( "Current Background" )->currentItem() != 1 )
        currentBck = getColorSelector( "Current Background" )->getColor();
    }
      
    pwCurrent->setColor( currentFore, currentBck );

    QColor markFore = getColorSelector( prefix + "Marked Foreground" )->getColor();

    if( !isInactive || getColorSelector( "Inactive Marked Foreground" )->currentItem() == 1 )
      if( getColorSelector( "Marked Foreground" )->currentItem() == 2 )
        markFore = currentFore;
    
    pwMark1->setColor( markFore, getColorSelector( prefix + "Marked Background" )->getColor() );
    pwMark2->setColor( markFore, getColorSelector( prefix + "Alternate Marked Background" )->getColor() );
  }
}

bool KgColors::apply()
{
  bool result = KonfiguratorPage::apply();
  KrColorCache::getColorCache().refreshColors();
  return result;
}

#include "kgcolors.moc"

