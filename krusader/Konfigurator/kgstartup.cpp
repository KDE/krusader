/***************************************************************************
                        kgstartup.cpp  -  description
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

#include "kgstartup.h"
#include "../defaults.h"
#include "../GUI/profilemanager.h"
#include "../krusader.h"
#include <klocale.h>
#include <klineedit.h>

KgStartup::KgStartup( bool first, QWidget* parent,  const char* name ) :
  KonfiguratorPage( first, parent, name ), profileCombo( 0 )
{
  QGridLayout *kgStartupLayout = new QGridLayout( parent );
  kgStartupLayout->setSpacing( 6 );
  kgStartupLayout->setMargin( 11 );

  //  --------------------------- PANELS GROUPBOX ----------------------------------

  QGroupBox *panelsGrp = createFrame( i18n( "Panels" ), parent, "panelsGrp" );
  QGridLayout *panelsGrid = createGridLayout( panelsGrp->layout() );

  KONFIGURATOR_NAME_VALUE_TIP savePanels[] =
  //          name                            value      tooltip
    {{ i18n( "Save the state of the tabs at exit and restore it at startup" ) , "Tabs",    i18n( "Saves the last state of the tabs at exit and restores it at startup." ) },
     { i18n( "Restore a manually saved state at startup" )                    , "None",    i18n( "In the Settings menu the state of the tabs can be saved, and this will be restored after startup." ) },
     { i18n( "Start from profile:" )                                          , "Profile", i18n( "Starts always from the following profile (at least one profile is necessary):" ) } };

  saveRadio = createRadioButtonGroup( "Startup", "Panels Save Settings",
      "Tabs", 1, 0, savePanels, 3, panelsGrp, "mySaveRadio", false );
  panelsGrid->addWidget( saveRadio, 0, 0 );
  connect( saveRadio->find( i18n( "Start from profile:" ) ), SIGNAL( stateChanged( int ) ), this, SLOT( slotDisable() ) );
  
  QStringList profileList = ProfileManager::availableProfiles( "Panel" );
  if( profileList.count() )
  {
    KONFIGURATOR_NAME_VALUE_PAIR comboItems[ profileList.count() ];
    for( int i=0; i != profileList.count(); i++ )
      comboItems[ i ].text = comboItems[ i ].value = profileList [ i ];
      
    profileCombo = createComboBox( "Startup", "Starter Profile Name", profileList[ 0 ], comboItems, profileList.count(), panelsGrp, false, false );
    panelsGrid->addWidget( profileCombo, 1, 0 );
  }
  else
    saveRadio->find( i18n( "Start from profile:" ) )->setEnabled( false );
  
  kgStartupLayout->addWidget( panelsGrp, 0, 0 );

  //  ------------------------ USERINTERFACE GROUPBOX ------------------------------

  QGroupBox *uiGrp = createFrame( i18n( "User Interface" ), parent, "uiGrp" );
  QGridLayout *uiGrid = createGridLayout( uiGrp->layout() );

  KONFIGURATOR_CHECKBOX_PARAM uiCheckBoxes[] =
  //   cfg_class  cfg_name                default               text                                   restart ToolTip
    {{"Startup","UI Save Settings",      _UiSave,               i18n( "Save settings on exit" ),       false,  i18n( "Krusader checks the state of the user interface components,\nand restores them to their condition when last shutdown." ) },
     {"Startup","Show tool bar",         _ShowToolBar,          i18n( "Show toolbar" ),                false,  i18n( "Toolbar will be visible after startup." ) },
     {"Startup","Show status bar",       _ShowStatusBar,        i18n( "Show statusbar" ),              false,  i18n( "Statusbar will be visible after startup." ) },
     {"Startup","Show FN Keys",          _ShowFNkeys,           i18n( "Show function keys" ),          false,  i18n( "Function keys will be visible after startup." ) },
     {"Startup","Show Cmd Line",         _ShowCmdline,          i18n( "Show command line" ),           false,  i18n( "Command line will be visible after startup." ) },
     {"Startup","Show Terminal Emulator",_ShowTerminalEmulator, i18n( "Show terminal emulator" ),      false,  i18n( "Terminal emulator will be visible after startup." ) },
     {"Startup","Remember Position",     _RememberPos,          i18n( "Save last position, size and panel settings" ), false,  i18n( "When launched, Krusader will resize itself to the size it was when last shutdown.\nKrusader will also appear in the same location on the screen, having panels sorted and aligned as they were.\nIf this option is disabled, you can use the menu 'Settings-Save Position'\noption to manually set Krusader's size and position at startup" )
}};

  uiCbGroup = createCheckBoxGroup( 1, 0, uiCheckBoxes, 7, uiGrp );
  connect( uiCbGroup->find( "UI Save Settings" ), SIGNAL( stateChanged( int ) ), this, SLOT( slotDisable() ) );

  uiGrid->addWidget( uiCbGroup, 1, 0 );

  slotDisable();

  kgStartupLayout->addWidget( uiGrp, 1, 0 );
}

void KgStartup::slotDisable()
{
  if( profileCombo )
     profileCombo->setEnabled( saveRadio->find( i18n( "Start from profile:" ) )->isOn() );

  bool isUiSave   = !uiCbGroup->find( "UI Save Settings" )->isChecked();

  int i=1;
  while( uiCbGroup->find( i ) )
    uiCbGroup->find( i++ )->setEnabled( isUiSave );
}

bool KgStartup::apply()
{
  Krusader::actSaveTabs->setEnabled( saveRadio->find( i18n( "Restore a manually saved state at startup" ) )->isOn() );
  return KonfiguratorPage::apply();
}

#include "kgstartup.moc"
