/***************************************************************************
                        kglookfeel.cpp  -  description
                             -------------------
    copyright            : (C) 2003 by Csaba Karai
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

#include "kglookfeel.h"
#include "../krusader.h"
#include "../defaults.h"
#include <qtabwidget.h>
#include <klocale.h>
#include <qtooltip.h>
#include <qvalidator.h>

KgLookFeel::KgLookFeel( bool first, QWidget* parent,  const char* name ) :
      KonfiguratorPage( first, parent, name )
{
  QGridLayout *kgLookAndFeelLayout = new QGridLayout( parent );
  kgLookAndFeelLayout->setSpacing( 6 );
  kgLookAndFeelLayout->setMargin( 11 );

  //  ---------------------------- GENERAL TAB -------------------------------------
  QTabWidget *tabWidget = new QTabWidget( parent, "tabWidget" );

  QWidget *tab = new QWidget( tabWidget, "tab" );
  tabWidget->insertTab( tab, i18n( "General" ) );

  QGridLayout *lookAndFeelLayout = new QGridLayout( tab );
  lookAndFeelLayout->setSpacing( 6 );
  lookAndFeelLayout->setMargin( 11 );
  QGroupBox *lookFeelGrp = createFrame( i18n( "Look & Feel" ), tab, "kgLookAndFeelGrp" );
  QGridLayout *lookFeelGrid = createGridLayout( lookFeelGrp->layout() );

  KONFIGURATOR_CHECKBOX_PARAM settings[] =
  //   cfg_class  cfg_name                default             text                              restart tooltip
    {{"Look&Feel","Warn On Exit",         _WarnOnExit,        i18n( "Warn on exit" ),           false,  ""},
     {"Look&Feel","Minimize To Tray",     _MinimizeToTray,    i18n( "Minimize to tray" ),       true ,  ""},
     {"Look&Feel","Show Hidden",          _ShowHidden,        i18n( "Show hidden files" ),      false,  ""},
     {"Look&Feel","Mark Dirs",            _MarkDirs,          i18n( "Automark directories" ),   false,  ""},
     {"Look&Feel","Case Sensative Sort",  _CaseSensativeSort, i18n( "Case sensitive sorting" ), false,  ""}};

  QWidget *cbs = createCheckBoxGroup( 2, 0, settings, 5, settingCbs,  lookFeelGrp );
  lookFeelGrid->addWidget( cbs, 0, 0 );

  lookFeelGrid->addWidget( createLine( lookFeelGrp, "lookSep1" ), 1, 0 );

  QHBox *hbox = new QHBox( lookFeelGrp, "lookAndFeelHBox1" );
  new QLabel( i18n( "Panel font:" ), hbox, "lookAndFeelLabel" );
  createFontChooser( "Look&Feel", "Filelist Font", _FilelistFont, hbox, true );
  createSpacer ( hbox );
  lookFeelGrid->addWidget( hbox, 2, 0 );
  
  QHBox *hbox2 = new QHBox( lookFeelGrp, "lookAndFeelHBox2" );
  QLabel *lbl1 = new QLabel( i18n( "Filelist icon size:" ), hbox2, "lookAndFeelLabel2" );
  lbl1->setMinimumWidth( 230 );
  KONFIGURATOR_NAME_VALUE_PAIR iconSizes[] =
    {{ i18n( "16" ),  "16" },
     { i18n( "22" ),  "22" },
     { i18n( "32" ),  "32" },
     { i18n( "48" ),  "48" }};
  KonfiguratorComboBox *iconCombo = createComboBox( "Look&Feel", "Filelist Icon Size", _FilelistIconSize, iconSizes, 4, hbox2, true, true );
  iconCombo->lineEdit()->setValidator( new QRegExpValidator( QRegExp( "[1-9]\\d{0,1}" ), iconCombo ) );
  createSpacer ( hbox2 );
  lookFeelGrid->addWidget( hbox2, 3, 0 );

  lookFeelGrid->addWidget( createLine( lookFeelGrp, "lookSep1" ), 5, 0 );
  
  addLabel( lookFeelGrid, 6, 0, i18n( "Mouse Selection Mode:" ),
            lookFeelGrp, "lookAndFeelLabel4" );
            
  KONFIGURATOR_NAME_VALUE_PAIR mouseSelection[] =
    {{ i18n( "Classic (both keys combined)" ),                   "0" },
     { i18n( "Left mouse button selects" ),                      "1" },    
     { i18n( "Right button selects (Windows Commander style)" ), "2" }};
  KonfiguratorRadioButtons *mouseRadio = createRadioButtonGroup( "Look&Feel", "Mouse Selection",
      "0", 1, 0, mouseSelection, 3, lookFeelGrp, "myLook&FeelRadio", false );
  lookFeelGrid->addWidget( mouseRadio->getGroupWidget(), 7, 0 );
  for( int i=0; i!=3; i++ )
    mouseRadio->radioButtons.at(i)->setEnabled( false ); /* disable all buttons */

  lookAndFeelLayout->addWidget( lookFeelGrp, 0, 0 );
  
  //  ---------------------------- TOOLBAR TAB -------------------------------------
  tab_2 = new QWidget( tabWidget, "tab_2" );
  tabWidget->insertTab( tab_2, i18n( "Toolbar" ) );

  toolBarLayout = new QGridLayout( tab_2 );
  KonfiguratorEditToolbarWidget *editToolbar = new KonfiguratorEditToolbarWidget(krApp->factory(),tab_2);
  connect( editToolbar, SIGNAL( reload( KonfiguratorEditToolbarWidget * ) ), this, SLOT( slotReload( KonfiguratorEditToolbarWidget *) ) );
  toolBarLayout->addWidget(editToolbar->editToolbarWidget(),0,0);
  registerObject( editToolbar );

  //  -------------------------- KEY-BINDINGS TAB ----------------------------------
  tab_3 = new QWidget( tabWidget, "tab_3" );
  tabWidget->insertTab( tab_3, i18n( "Key-bindings" ) );

  keyBindingsLayout = new QGridLayout( tab_3 );
  KonfiguratorKeyChooser *keyBindings = new KonfiguratorKeyChooser(krApp->actionCollection(),tab_3);
  connect( keyBindings, SIGNAL( reload( KonfiguratorKeyChooser * ) ), this, SLOT( slotReload( KonfiguratorKeyChooser *) ) );
  keyBindingsLayout->addWidget(keyBindings->keyChooserWidget(),0,0);
  registerObject( keyBindings );
  
  kgLookAndFeelLayout->addWidget( tabWidget, 0, 0 );
}

void KgLookFeel::slotReload( KonfiguratorKeyChooser * oldChooser )
{
  removeObject( oldChooser );
  delete oldChooser;

  KonfiguratorKeyChooser *keyBindings = new KonfiguratorKeyChooser(krApp->actionCollection(),tab_3);
  connect( keyBindings, SIGNAL( reload( KonfiguratorKeyChooser * ) ), this, SLOT( slotReload( KonfiguratorKeyChooser *) ) );
  keyBindingsLayout->addWidget(keyBindings->keyChooserWidget(),0,0);
  registerObject( keyBindings );
  keyBindings->keyChooserWidget()->show();
}

void KgLookFeel::slotReload( KonfiguratorEditToolbarWidget * oldEditToolbar )
{
  removeObject( oldEditToolbar );
  delete oldEditToolbar;

  KonfiguratorEditToolbarWidget *editToolbar = new KonfiguratorEditToolbarWidget(krApp->factory(),tab_2);
  connect( editToolbar, SIGNAL( reload( KonfiguratorEditToolbarWidget * ) ), this, SLOT( slotReload( KonfiguratorEditToolbarWidget *) ) );
  toolBarLayout->addWidget(editToolbar->editToolbarWidget(),0,0);
  registerObject( editToolbar );
  editToolbar->editToolbarWidget()->show();
}

#include "kglookfeel.moc"
