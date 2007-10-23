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
#include "../Dialogs/krdialogs.h"
#include <qtabwidget.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QLabel>
#include <Q3VBoxLayout>
#include <klocale.h>
#include <q3whatsthis.h>
#include <qvalidator.h>
#include <q3listview.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include "../Panel/krselectionmode.h"
#include "../Panel/listpanel.h"

#define PAGE_OPERATION     0
#define PAGE_PANEL         1
#define PAGE_PANELTOOLBAR  2
#define PAGE_MOUSE  3

KgLookFeel::KgLookFeel( bool first, QWidget* parent ) :
      KonfiguratorPage( first, parent )
{
  QGridLayout *kgLookAndFeelLayout = new QGridLayout( this );
  kgLookAndFeelLayout->setSpacing( 6 );

  tabWidget = new QTabWidget( this, "tabWidget" );

  setupOperationTab();
  setupPanelTab();
  setupPanelToolbarTab();
  setupMouseModeTab();

  kgLookAndFeelLayout->addWidget( tabWidget, 0, 0 );
}

// ---------------------------------------------------------------------------------------
//  ---------------------------- OPERATION TAB -------------------------------------
// ---------------------------------------------------------------------------------------
void KgLookFeel::setupOperationTab() {
  QWidget *tab = new QWidget( tabWidget, "tab_operation" );
  tabWidget->insertTab( tab, i18n( "Operation" ) );

  QGridLayout *lookAndFeelLayout = new QGridLayout( tab );
  lookAndFeelLayout->setSpacing( 6 );
  lookAndFeelLayout->setContentsMargins( 11, 11, 11, 11 );

  // -------------- General -----------------
  Q3GroupBox *lookFeelGrp = createFrame( i18n( "Look && Feel" ), tab );
  QGridLayout *lookFeelGrid = createGridLayout( lookFeelGrp->layout() );

  KONFIGURATOR_CHECKBOX_PARAM settings[] =
    { //   cfg_class  cfg_name                default             text                              restart tooltip
     {"Look&Feel","Warn On Exit",         _WarnOnExit,        i18n( "Warn on exit" ),           false,  i18n( "Display a warning when trying to close the main window." ) }, // KDE4: move warn on exit to the other confirmations
     {"Look&Feel","Minimize To Tray",     _MinimizeToTray,    i18n( "Minimize to tray" ),       false,  i18n( "The icon will appear in the system tray instead of the taskbar, when Krusader is minimized." ) },
     {"Look&Feel","Mark Dirs",            _MarkDirs,          i18n( "Autoselect directories" ),   false,  i18n( "When matching the select criteria, not only files will be selected, but also directories." ) },
     {"Look&Feel","Rename Selects Extension",true,          i18n( "Rename selects extension" ),   false,  i18n( "When renaming a file, the whole text is selected. If you want Total-Commander like renaming of just the name, without extension, uncheck this option." ) },
     {"Look&Feel","Fullpath Tab Names",   _FullPathTabNames,  i18n( "Use full path tab names" ), true ,  i18n( "Display the full path in the folder tabs. By default only the last part of the path is displayed." ) },
     {"Look&Feel","Fullscreen Terminal Emulator", false, i18n( "Fullscreen terminal (mc-style)"  ), false,  i18n( "Terminal is shown instead of the Krusader window (full screen).") },
    };

  cbs = createCheckBoxGroup( 2, 0, settings, 6 /*count*/, lookFeelGrp, PAGE_OPERATION );
  lookFeelGrid->addWidget( cbs, 0, 0 );

  lookAndFeelLayout->addWidget( lookFeelGrp, 0, 0 );

  // -------------- Quicksearch -----------------
  Q3GroupBox *quicksearchGroup = createFrame( i18n( "Quicksearch" ), tab );
  QGridLayout *quicksearchGrid = createGridLayout( quicksearchGroup->layout() );

  KONFIGURATOR_CHECKBOX_PARAM quicksearch[] =
   { //   cfg_class  cfg_name                default             text                              restart tooltip
     {"Look&Feel","New Style Quicksearch",  _NewStyleQuicksearch, i18n( "New style quicksearch" ), false,  i18n( "Opens a quick search dialog box." ) },
     {"Look&Feel","Case Sensitive Quicksearch",  _CaseSensitiveQuicksearch, i18n( "Case sensitive quicksearch" ), false,  i18n( "All files beginning with capital letters appear before files beginning with non-capital letters (UNIX default)." ) },
    };

  quicksearchCheckboxes = createCheckBoxGroup( 2, 0, quicksearch, 2 /*count*/, quicksearchGroup, PAGE_OPERATION );
  quicksearchGrid->addWidget( quicksearchCheckboxes, 0, 0 );
  connect( quicksearchCheckboxes->find( "New Style Quicksearch" ), SIGNAL( stateChanged( int ) ), this, SLOT( slotDisable() ) );
  slotDisable();

  lookAndFeelLayout->addWidget( quicksearchGroup, 1, 0 );
}

// ----------------------------------------------------------------------------------
//  ---------------------------- PANEL TAB -------------------------------------
// ----------------------------------------------------------------------------------
void KgLookFeel::setupPanelTab() {
  QWidget* tab_panel = new QWidget( tabWidget, "tab_panel" );
  tabWidget->insertTab( tab_panel, i18n( "Panel" ) );

  QGridLayout *panelLayout = new QGridLayout( tab_panel );
  panelLayout->setSpacing( 6 );
  panelLayout->setContentsMargins( 11, 11, 11, 11 );
  Q3GroupBox *panelGrp = createFrame( i18n( "Panel settings" ), tab_panel );
  QGridLayout *panelGrid = createGridLayout( panelGrp->layout() );

  Q3HBox *hbox = new Q3HBox( panelGrp, "lookAndFeelHBox1" );
  new QLabel( i18n( "Panel font:" ), hbox, "lookAndFeelLabel" );
  createFontChooser( "Look&Feel", "Filelist Font", _FilelistFont, hbox, true, PAGE_PANEL );
  createSpacer ( hbox );
  panelGrid->addWidget( hbox, 0, 0 );

  Q3HBox *hbox2 = new Q3HBox( panelGrp, "lookAndFeelHBox2" );
  QLabel *lbl1 = new QLabel( i18n( "Filelist icon size:" ), hbox2, "lookAndFeelLabel2" );
  lbl1->setMinimumWidth( 230 );
  KONFIGURATOR_NAME_VALUE_PAIR iconSizes[] =
    {{ i18n( "16" ),  "16" },
     { i18n( "22" ),  "22" },
     { i18n( "32" ),  "32" },
     { i18n( "48" ),  "48" }};
  KonfiguratorComboBox *iconCombo = createComboBox( "Look&Feel", "Filelist Icon Size", _FilelistIconSize, iconSizes, 4, hbox2, true, true, PAGE_PANEL );
  iconCombo->lineEdit()->setValidator( new QRegExpValidator( QRegExp( "[1-9]\\d{0,1}" ), iconCombo ) );
  createSpacer ( hbox2 );
  panelGrid->addWidget( hbox2, 1, 0 );

  panelGrid->addWidget( createLine( panelGrp ), 2, 0 );

  KONFIGURATOR_CHECKBOX_PARAM panelSettings[] =
  //   cfg_class  cfg_name                default text                                  restart tooltip
    {
	 {"Look&Feel","With Icons",           _WithIcons,   i18n( "Use icons in the filenames" ), true ,  i18n( "Show the icons for filenames and folders." ) },
	 {"Look&Feel","Human Readable Size",  _HumanReadableSize, i18n( "Use human-readable file size" ), true ,  i18n( "File sizes are displayed in B, KB, MB and GB, not just in bytes." ) },
	 {"Look&Feel","Show Hidden",          _ShowHidden,        i18n( "Show hidden files" ),      false,  i18n( "Display files beginning with a dot." ) },
	 {"Look&Feel","Case Sensative Sort",  _CaseSensativeSort, i18n( "Case sensitive sorting" ), true ,  i18n( "All files beginning with capital letters appear before files beginning with non-capital letters (UNIX default)." ) },
	 {"Look&Feel","Always sort dirs by name",  false, i18n( "Always sort dirs by name"  ), true,  i18n( "Directories are sorted by name, regardless of the sort column.") },
	 {"Look&Feel","Numeric permissions",  _NumericPermissions, i18n( "Numeric Permissions"  ), true,  i18n( "Show octal numbers (0755) instead of the standard permissions (rwxr-xr-x) in the permission column.") },
    };

  KonfiguratorCheckBoxGroup *panelSett = createCheckBoxGroup( 2, 0, panelSettings, 6 /*count*/, panelGrp, PAGE_PANEL );
  
  panelGrid->addWidget( panelSett, 3, 0 );
  
// ----------------------------------------------------------------------------------
//  ---------------------------- DEFAULT PANEL TYPE -------------------------------------
// ----------------------------------------------------------------------------------
  
  panelGrid->addWidget( createLine( panelGrp ), 4, 0 );
  
  Q3HBox *hbox3 = new Q3HBox( panelGrp, "lookAndFeelHBox3" );
  new QLabel( i18n( "Default panel type:" ), hbox3, "lookAndFeelLabel3" );
  KONFIGURATOR_NAME_VALUE_PAIR panelTypes[] =
    {{ i18n( "Detailed" ),  "Detailed" },
     { i18n( "Brief" ),     "Brief" }};
  createComboBox( "Look&Feel", "Default Panel Type", _DefaultPanelType, panelTypes, 2, hbox3, false, false, PAGE_PANEL );
  createSpacer ( hbox3 );
  panelGrid->addWidget( hbox3, 5, 0 );  

  panelLayout->addWidget( panelGrp, 0, 0 );
}

// -----------------------------------------------------------------------------------
//  -------------------------- Panel Toolbar TAB ----------------------------------
// -----------------------------------------------------------------------------------
void KgLookFeel::setupPanelToolbarTab() {
  QWidget     *tab_4 = new QWidget( tabWidget, "tab_4" );
  tabWidget->insertTab( tab_4, i18n( "Panel Toolbar" ) );

  Q3BoxLayout * panelToolbarVLayout = new Q3VBoxLayout( tab_4 );
  panelToolbarVLayout->setSpacing( 6 );
  panelToolbarVLayout->setContentsMargins( 11, 11, 11, 11 );

  KONFIGURATOR_CHECKBOX_PARAM panelToolbarActiveCheckbox[] = 
  //   cfg_class    cfg_name                default        text                          restart tooltip
    {{"Look&Feel", "Panel Toolbar visible", _PanelToolBar, i18n( "Show Panel Toolbar" ), true,   i18n( "The panel toolbar will be visible." ) }
  };

  panelToolbarActive = createCheckBoxGroup( 1, 0, panelToolbarActiveCheckbox, 1, tab_4, PAGE_PANELTOOLBAR);
  connect( panelToolbarActive->find( "Panel Toolbar visible" ), SIGNAL( stateChanged( int ) ), this, SLOT( slotEnablePanelToolbar() ) );
    
  Q3GroupBox * panelToolbarGrp = createFrame( i18n( "Visible Panel Toolbar buttons" ), tab_4 );
  QGridLayout * panelToolbarGrid = createGridLayout( panelToolbarGrp->layout() );

  KONFIGURATOR_CHECKBOX_PARAM panelToolbarCheckboxes[] = 
    {
  //   cfg_class    cfg_name                default             text                       restart tooltip
     {"Look&Feel",  "Clear Location Bar Visible",  _ClearLocation,      i18n( "Clear location bar button" ),    true ,  i18n( "Clears the location bar" ) },
     {"Look&Feel",  "Open Button Visible",  _Open,      i18n( "Open button" ),     true ,  i18n( "Opens the directory browser." ) },
     {"Look&Feel",  "Equal Button Visible", _cdOther,   i18n( "Equal button (=)" ),true ,  i18n( "Changes the panel directory to the other panel directory." ) },
     {"Look&Feel",  "Up Button Visible",    _cdUp,      i18n( "Up button (..)" ),  true ,  i18n( "Changes the panel directory to the parent directory." ) },
     {"Look&Feel",  "Home Button Visible",  _cdHome,    i18n( "Home button (~)" ), true ,  i18n( "Changes the panel directory to the home directory." ) },
     {"Look&Feel",  "Root Button Visible",  _cdRoot,    i18n( "Root button (/)" ), true ,  i18n( "Changes the panel directory to the root directory." ) },
     {"Look&Feel",  "SyncBrowse Button Visible",  _syncBrowseButton,    i18n( "Toggle-button for sync-browsing" ), true ,  i18n( "Each directory change in the panel is also performed in the other panel." ) },
    };
  
  
  pnlcbs = createCheckBoxGroup(1, 0, panelToolbarCheckboxes, 7,
                                       panelToolbarGrp, PAGE_PANELTOOLBAR);
  
  panelToolbarVLayout->addWidget( panelToolbarActive, 0, 0 );
  panelToolbarGrid->addWidget( pnlcbs, 0, 0 );
  panelToolbarVLayout->addWidget( panelToolbarGrp,    1, 0 );

  // Enable panel toolbar checkboxes
  slotEnablePanelToolbar();
}

// ---------------------------------------------------------------------------
//  -------------------------- Mouse TAB ----------------------------------
// ---------------------------------------------------------------------------
void KgLookFeel::setupMouseModeTab() {
  QWidget *tab_mouse = new QWidget( tabWidget, "tab_mouse" );
  tabWidget->insertTab( tab_mouse, i18n( "Selection Mode" ) );
  QGridLayout *mouseLayout = new QGridLayout( tab_mouse );
  mouseLayout->setSpacing( 6 );
  mouseLayout->setContentsMargins( 11, 11, 11, 11 );

  // -------------- General -----------------
  Q3GroupBox *mouseGeneralGroup = createFrame( i18n( "General" ), tab_mouse );
  QGridLayout *mouseGeneralGrid = createGridLayout( mouseGeneralGroup->layout() );
  mouseGeneralGrid->setSpacing( 0 );
  mouseGeneralGrid->setContentsMargins( 5, 5, 5, 5 );

  KONFIGURATOR_NAME_VALUE_TIP mouseSelection[] =
    {
    	//     name           value          tooltip
    	{ i18n( "Krusader Mode" ), "0", i18n( "Both keys allow selecting files. To select more than one file, hold the Ctrl key and click the left mouse button. Right-click menu is invoked using a short click on the right mouse button." ) },
     	{ i18n( "Konqueror Mode" ), "1", i18n( "Pressing the left mouse button selects files - you can click and select multiple files. Right-click menu is invoked using a short click on the right mouse button." ) },
     	{ i18n( "Total-Commander Mode" ), "2", i18n( "The left mouse button does not select, but sets the current file without affecting the current selection. The right mouse button selects multiple files and the right-click menu is invoked by pressing and holding the right mouse button." ) },
     	{ i18n( "Custom Selection Mode" ), "3", i18n( "Design your own selection mode!" ) }
     };
  mouseRadio = createRadioButtonGroup( "Look&Feel", "Mouse Selection", "0", 2, 2, mouseSelection, 4, mouseGeneralGroup, true, PAGE_MOUSE );
  mouseRadio->layout()->setContentsMargins( 0, 0, 0, 0 );
  mouseGeneralGrid->addWidget( mouseRadio, 0, 0 );
  connect( mouseRadio, SIGNAL( clicked(int) ), SLOT( slotSelectionModeChanged() ) );

  mouseLayout->addWidget( mouseGeneralGroup, 0, 0, 1, 2 );

  // -------------- Details -----------------
  Q3GroupBox *mouseDetailGroup = createFrame( i18n( "Details" ), tab_mouse );
  QGridLayout *mouseDetailGrid = createGridLayout( mouseDetailGroup->layout() );
  mouseDetailGrid->setSpacing( 0 );
  mouseDetailGrid->setContentsMargins( 5, 5, 5, 5 );

   KONFIGURATOR_NAME_VALUE_TIP singleOrDoubleClick[] =
    {
    	//          name            value            tooltip
    	{ i18n( "Double-click selects (classic)" ), "0", i18n( "A single click on a file will select and focus, a double click opens the file or steps into the directory." ) },
    	{ i18n( "Obey KDE's global selection policy" ), "1", i18n( "<p>Use KDE's global setting:</p><p><i>KDE Control Center -> Peripherals -> Mouse</i></p>" ) }
    };
  KonfiguratorRadioButtons *clickRadio = createRadioButtonGroup( "Look&Feel", "Single Click Selects", "0", 1, 0, singleOrDoubleClick, 2, mouseDetailGroup, true, PAGE_MOUSE );
  clickRadio->layout()->setContentsMargins( 0, 0, 0, 0 );
  mouseDetailGrid->addWidget( clickRadio, 0, 0 );
  
  KONFIGURATOR_CHECKBOX_PARAM mouseCheckboxesParam[] = 
    {
     // {cfg_class,   cfg_name,   default
     // 	text,  restart,
     // 	tooltip }
     {"Custom Selection Mode",  "QT Selection",  _QtSelection,
     	i18n( "Based on KDE's selection mode" ), true,
     	i18n( "If checked, use a mode based on KDE's style." ) },
     {"Custom Selection Mode",  "Left Selects",  _LeftSelects,
     	i18n( "Left mouse button selects" ), true,
     	i18n( "If checked, left clicking an item will select it." ) },
     {"Custom Selection Mode",  "Left Preserves", _LeftPreserves,
     	i18n( "Left mouse button preserves selection" ), true,
     	i18n( "If checked, left clicking an item will select it, but will not unselect other, already selected items." ) },
     {"Custom Selection Mode",  "ShiftCtrl Left Selects",  _ShiftCtrlLeft,
     	i18n( "Shift/Ctrl-Left mouse button selects" ), true,
     	i18n( "If checked, shift/ctrl left clicking will select items. \nNote: This is meaningless if 'Left Button Selects' is checked." ) },
     {"Custom Selection Mode",  "Right Selects",  _RightSelects,
     	i18n( "Right mouse button selects" ), true,
     	i18n( "If checked, right clicking an item will select it." ) },
     {"Custom Selection Mode",  "Right Preserves",  _RightPreserves,
     	i18n( "Right mouse button preserves selection" ), true,
     	i18n( "If checked, right clicking an item will select it, but will not unselect other, already selected items." ) },
     {"Custom Selection Mode",  "ShiftCtrl Right Selects",  _ShiftCtrlRight,
     	i18n( "Shift/Ctrl-Right mouse button selects" ), true,
     	i18n( "If checked, shift/ctrl right clicking will select items. \nNote: This is meaningless if 'Right Button Selects' is checked." ) },
     {"Custom Selection Mode",  "Space Moves Down",  _SpaceMovesDown,
     	i18n( "Spacebar moves down" ), true,
     	i18n( "If checked, pressing the spacebar will select the current item and move down. \nOtherwise, current item is selected, but remains the current item." ) },
     {"Custom Selection Mode",  "Space Calc Space",  _SpaceCalcSpace,
     	i18n( "Spacebar calculates disk space" ), true,
     	i18n( "If checked, pressing the spacebar while the current item is a folder, will (except from selecting the folder) \ncalculate space occupied by the folder (recursively)." ) },
     {"Custom Selection Mode",  "Insert Moves Down",  _InsertMovesDown,
     	i18n( "Insert moves down" ), true,
     	i18n( "If checked, pressing INSERT will select the current item, and move down to the next item. \nOtherwise, current item is not changed." ) },
     {"Custom Selection Mode",  "Immediate Context Menu",  _ImmediateContextMenu,
     	i18n( "Right clicking pops context menu immediately" ), true,
     	i18n( "If checked, right clicking will result in an immediate showing of the context menu. \nOtherwise, user needs to click and hold the right mouse button for 500ms." ) },
    };
  
  
  mouseCheckboxes = createCheckBoxGroup(1, 0, mouseCheckboxesParam, 11 /*count*/, mouseDetailGroup, PAGE_MOUSE);
  mouseDetailGrid->addWidget( mouseCheckboxes, 1, 0 );

  mouseLayout->addWidget( mouseDetailGroup, 1,0 );

  // Disable the details-button if not in custom-mode
  slotSelectionModeChanged();

  // -------------- Preview -----------------
  Q3GroupBox *mousePreviewGroup = createFrame( i18n( "Preview" ), tab_mouse );
  QGridLayout *mousePreviewGrid = createGridLayout( mousePreviewGroup->layout() );
  // TODO preview
  mousePreview = new Q3ListView( mousePreviewGroup, "mousePreview" );
  mousePreviewGrid->addWidget( mousePreview, 0 ,0 );
  mousePreviewGroup->setEnabled(false); // TODO re-enable once the preview is implemented
  // ------------------------------------------
  mouseLayout->addWidget( mousePreviewGroup, 1,1 );
}

void KgLookFeel::slotDisable()
{
  bool isNewStyleQuickSearch = quicksearchCheckboxes->find( "New Style Quicksearch" )->isChecked();
  quicksearchCheckboxes->find( "Case Sensitive Quicksearch" )->setEnabled( isNewStyleQuickSearch );
}

void KgLookFeel::slotEnablePanelToolbar()
{
  bool enableTB = panelToolbarActive->find("Panel Toolbar visible")->isChecked();
  pnlcbs->find( "Root Button Visible"     )->setEnabled(enableTB);
  pnlcbs->find( "Home Button Visible"     )->setEnabled(enableTB);
  pnlcbs->find( "Up Button Visible"       )->setEnabled(enableTB);
  pnlcbs->find( "Equal Button Visible"    )->setEnabled(enableTB);
  pnlcbs->find( "Open Button Visible"     )->setEnabled(enableTB);  
  pnlcbs->find("SyncBrowse Button Visible")->setEnabled(enableTB);  
}

void KgLookFeel::slotSelectionModeChanged() {
  bool enable = mouseRadio->find( i18n("Custom Selection Mode") )->isChecked();
  mouseCheckboxes->find( "QT Selection" )->setEnabled( enable );
  mouseCheckboxes->find( "Left Selects" )->setEnabled( enable );
  mouseCheckboxes->find( "Left Preserves" )->setEnabled( enable );
  mouseCheckboxes->find( "ShiftCtrl Left Selects" )->setEnabled( enable );
  mouseCheckboxes->find( "Right Selects" )->setEnabled( enable );
  mouseCheckboxes->find( "Right Preserves" )->setEnabled( enable );
  mouseCheckboxes->find( "ShiftCtrl Right Selects" )->setEnabled( enable );
  mouseCheckboxes->find( "Space Moves Down" )->setEnabled( enable );
  mouseCheckboxes->find( "Space Calc Space" )->setEnabled( enable );
  mouseCheckboxes->find( "Insert Moves Down" )->setEnabled( enable );
  mouseCheckboxes->find( "Immediate Context Menu" )->setEnabled( enable );
}

int KgLookFeel::activeSubPage() {
	return tabWidget->currentPageIndex();
}

#include "kglookfeel.moc"

