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
#include <klocale.h>
#include <klineedit.h>

KgStartup::KgStartup( bool first, QWidget* parent,  const char* name ) :
  KonfiguratorPage( first, parent, name )
{
  QGridLayout *kgStartupLayout = new QGridLayout( parent );
  kgStartupLayout->setSpacing( 6 );
  kgStartupLayout->setMargin( 11 );

  //  --------------------------- PANELS GROUPBOX ----------------------------------

  QGroupBox *panelsGrp = createFrame( i18n( "Panels" ), parent, "panelsGrp" );
  QGridLayout *panelsGrid = createGridLayout( panelsGrp->layout() );

  KONFIGURATOR_NAME_VALUE_TIP savePanels[] =
  //          name                                    value   tooltip
    {{ i18n( "Save settings on exit" )             ,  "true",  i18n( "When Krusader launches, the panels current directory will be the same as it was when Krusader was shutdown." ) },
     { i18n( "Start with the following settings:" ),  "false", i18n( "Defines a startup status for each panel." ) } };

  saveRadio = createRadioButtonGroup( "Startup", "Panels Save Settings",
      "false", 1, 0, savePanels, 2, panelsGrp, "mySaveRadio", false );
  panelsGrid->addMultiCellWidget( saveRadio, 0, 0, 0, 3 );
  connect( saveRadio->find( i18n( "Start with the following settings:" ) ), SIGNAL( stateChanged( int ) ), this, SLOT( slotDisable() ) );
  
  KONFIGURATOR_NAME_VALUE_PAIR opCombo[] =
    {{ i18n( "homepage" ),              i18n( "homepage" )              },
     { i18n( "work dir" ),              i18n( "work dir" )              },
     { i18n( "the last place it was" ), i18n( "the last place it was" ) }};
    
  leftPanelLbl = new QLabel( i18n( "Left panel starts at" ), panelsGrp, "leftPanelLbl" );
  panelsGrid->addWidget( leftPanelLbl, 1, 0 );
  leftOrigin = createComboBox( "Startup", "Left Panel Origin", i18n( "homepage" ), opCombo, 3, panelsGrp, false );
  connect( leftOrigin, SIGNAL( highlighted( int ) ), this, SLOT( slotDisable() ) );
  connect( leftOrigin, SIGNAL( activated( int ) ), this, SLOT( slotDisable() ) );
  panelsGrid->addWidget( leftOrigin, 1, 1 ); 
  leftPanelLbl2 = new QLabel( i18n( "Homepage:" ), panelsGrp, "leftPanelLbl2" );
  panelsGrid->addWidget( leftPanelLbl2, 1, 2 );
  leftHomePage = createURLRequester( "Startup", "Left Panel Homepage", _LeftHomepage, panelsGrp, false );
  panelsGrid->addWidget( leftHomePage, 1, 3 );
  
  rightPanelLbl = new QLabel( i18n( "Right panel starts at" ), panelsGrp, "rightPanelLbl" );
  panelsGrid->addWidget( rightPanelLbl, 2, 0 );
  rightOrigin = createComboBox( "Startup", "Right Panel Origin", i18n( "homepage" ), opCombo, 3, panelsGrp, false );
  connect( rightOrigin, SIGNAL( highlighted( int ) ), this, SLOT( slotDisable() ) );
  connect( rightOrigin, SIGNAL( activated( int ) ), this, SLOT( slotDisable() ) );
  panelsGrid->addWidget( rightOrigin, 2, 1 );
  rightPanelLbl2 = new QLabel( i18n( "Homepage:" ), panelsGrp, "rightPanelLbl2" );
  panelsGrid->addWidget( rightPanelLbl2, 2, 2 );
  rightHomePage = createURLRequester( "Startup", "Right Panel Homepage", _RightHomepage, panelsGrp, false );
  panelsGrid->addWidget( rightHomePage, 2, 3 );

  kgStartupLayout->addWidget( panelsGrp, 0, 0 );

  //  ------------------------ USERINTERFACE GROUPBOX ------------------------------

  QGroupBox *uiGrp = createFrame( i18n( "User Interface" ), parent, "uiGrp" );
  QGridLayout *uiGrid = createGridLayout( uiGrp->layout() );

  KONFIGURATOR_CHECKBOX_PARAM uiCheckBoxes[] =
  //   cfg_class  cfg_name                default               text                                      restart ToolTip
    {{"Startup","UI Save Settings",      _UiSave,               i18n( "Save settings on exit" ),          false,  i18n( "Krusader checks the state of the user interface components,\nand restores them to the condition they were during shutdown." ) },
     {"Startup","Show tool bar",         _ShowToolBar,          i18n( "Toolbar visible" ),                false,  i18n( "Toolbar will be visible after startup." ) },
     {"Startup","Show status bar",       _ShowStatusBar,        i18n( "Statusbar visible" ),              false,  i18n( "Statusbar will be visible after startup." ) },
     {"Startup","Show FN Keys",          _ShowFNkeys,           i18n( "Function keys visible" ),          false,  i18n( "Function keys will be visible after startup." ) },
     {"Startup","Show Cmd Line",         _ShowCmdline,          i18n( "Command-line visible" ),           false,  i18n( "Command-line will be visible after startup." ) },
     {"Startup","Show Terminal Emulator",_ShowTerminalEmulator, i18n( "Terminal Emulator visible" ),      false,  i18n( "Terminal Emulator will be visible after startup." ) },
     {"Startup","Remember Position",     _RememberPos,          i18n( "Save last position and size" ), false,  i18n( "When launched, Krusader will resize itself to the last size it was when last shutdown.\nKrusader will also appear in the same location on the screen.\nIf this disabled, you can use the setting menu 'Save Position' option to manualy set Krusader's size and position on startup" )
}};

  uiCbGroup = createCheckBoxGroup( 1, 0, uiCheckBoxes, 7, uiGrp );
  connect( uiCbGroup->find( "UI Save Settings" ), SIGNAL( stateChanged( int ) ), this, SLOT( slotDisable() ) );

  uiGrid->addWidget( uiCbGroup, 1, 0 );

  slotDisable();

  kgStartupLayout->addWidget( uiGrp, 1, 0 );
}

void KgStartup::slotDisable()
{
  bool isDontSave = saveRadio->find( i18n( "Start with the following settings:" ) )->isChecked();
  bool isUiSave   = !uiCbGroup->find( "UI Save Settings" )->isChecked();
  bool isLeftHp   = leftOrigin->currentText() ==i18n("homepage");
  bool isRightHp  = rightOrigin->currentText()==i18n("homepage");

  leftPanelLbl->setEnabled( isDontSave );
  leftOrigin->setEnabled( isDontSave );
  leftPanelLbl2->setEnabled( isDontSave && isLeftHp );
  leftHomePage->lineEdit()->setEnabled( isDontSave && isLeftHp );
  leftHomePage->button()->setEnabled( isDontSave && isLeftHp );

  rightPanelLbl->setEnabled( isDontSave );
  rightOrigin->setEnabled( isDontSave );
  rightPanelLbl2->setEnabled( isDontSave && isRightHp );
  rightHomePage->lineEdit()->setEnabled( isDontSave && isRightHp );
  rightHomePage->button()->setEnabled( isDontSave && isRightHp );

  int i=1;
  while( uiCbGroup->find( i ) )
    uiCbGroup->find( i++ )->setEnabled( isUiSave );
}

#include "kgstartup.moc"
