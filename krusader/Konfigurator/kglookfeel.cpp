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
#include <klocale.h>
#include <qtooltip.h>
#include <qvalidator.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include "../Panel/krselectionmode.h"

KgLookFeel::KgLookFeel( bool first, QWidget* parent,  const char* name ) :
      KonfiguratorPage( first, parent, name )
{
  QGridLayout *kgLookAndFeelLayout = new QGridLayout( parent );
  kgLookAndFeelLayout->setSpacing( 6 );
  kgLookAndFeelLayout->setMargin( 11 );

  //  ---------------------------- GENERAL TAB -------------------------------------
  QTabWidget *tabWidget = new QTabWidget( parent, "tabWidget" );

  QWidget *tab = new QWidget( tabWidget, "tab" );
  tabWidget->insertTab( tab, i18n( "Operation" ) );

  QGridLayout *lookAndFeelLayout = new QGridLayout( tab );
  lookAndFeelLayout->setSpacing( 6 );
  lookAndFeelLayout->setMargin( 11 );
  QGroupBox *lookFeelGrp = createFrame( i18n( "Look && Feel" ), tab, "kgLookAndFeelGrp" );
  QGridLayout *lookFeelGrid = createGridLayout( lookFeelGrp->layout() );

  KONFIGURATOR_CHECKBOX_PARAM settings[] =
  //   cfg_class  cfg_name                default             text                              restart tooltip
    {{"Look&Feel","Warn On Exit",         _WarnOnExit,        i18n( "Warn on exit" ),           false,  i18n( "Display a warning when you try to exit Krusader." ) },
     {"Look&Feel","Minimize To Tray",     _MinimizeToTray,    i18n( "Minimize to tray" ),       false,  i18n( "When minimizing Krusader, its icon will appear in the system tray instead of the taskbar." ) },
     {"Look&Feel","Show Hidden",          _ShowHidden,        i18n( "Show hidden files" ),      false,  i18n( "Display the dot files" ) },
     {"Look&Feel","Mark Dirs",            _MarkDirs,          i18n( "Automark directories" ),   false,  i18n( "When matching the select criteria, directories will also be marked" ) },
     {"Look&Feel","Case Sensative Sort",  _CaseSensativeSort, i18n( "Case sensitive sorting" ), true ,  i18n( "All files beginning with capital letters appear before files beginning with non-capital letters (UNIX default)." ) },
     {"Look&Feel","Fullpath Tab Names",   _FullPathTabNames,  i18n( "Use full path tab names" ), true ,  i18n( "Display the full path in the Folder Tabs; otherwise, only the last part of the path will be displayed." ) },
     //{"Look&Feel","Single Click Selects", false,              i18n( "Single click executes" ),   false,  i18n( "" ) },
     {"Look&Feel","New Style Quicksearch",  _NewStyleQuicksearch, i18n( "New style quicksearch" ), false,  i18n( "Opens a quick search dialog box." ) },
     {"Look&Feel","Case Sensitive Quicksearch",  _CaseSensitiveQuicksearch, i18n( "Case sensitive quicksearch" ), false,  i18n( "All files beginning with capital letters appear before files beginning with non-capital letters (UNIX default)." ) },
    };

  cbs = createCheckBoxGroup( 2, 0, settings, 8/*9*/, lookFeelGrp );
  lookFeelGrid->addWidget( cbs, 0, 0 );
  connect( cbs->find( "New Style Quicksearch" ), SIGNAL( stateChanged( int ) ), this, SLOT( slotDisable() ) );

  lookFeelGrid->addWidget( createLine( lookFeelGrp, "lookSep1" ), 1, 0 );
  
  addLabel( lookFeelGrid, 7, 0, i18n( "Single click / Double click Selection:" ),
            lookFeelGrp, "lookAndFeelLabel0" );

   KONFIGURATOR_NAME_VALUE_TIP singleOrDoubleClick[] =
  //          name                                               value  tooltip
    {{ i18n( "Double-click selects (classic)" ),                   "0", i18n( "A single click on a file will select and focus, a double click opens the file or steps into the directory." ) },
     { i18n( "Obey KDE's global selection policy" ),               "1", i18n( "Use the KDE Kcontrol setting: KDE Control Center->Peripherals->Mouse" ) }};    
  KonfiguratorRadioButtons *clickRadio = createRadioButtonGroup( "Look&Feel", "Single Click Selects", "0", 1, 0, singleOrDoubleClick, 2, lookFeelGrp, "myLook&FeelRadio0", true );
  lookFeelGrid->addWidget( clickRadio, 8, 0 );
  
  
  lookFeelGrid->addWidget( createLine( lookFeelGrp, "lookSep2" ), 9, 0 );
  
  addLabel( lookFeelGrid, 10, 0, i18n( "Mouse Selection Mode:" ),
            lookFeelGrp, "lookAndFeelLabel4" );
  KONFIGURATOR_NAME_VALUE_TIP mouseSelection[] =
  //            name                 value  tooltip
    {{ i18n( "Krusader Mode" ),        "0", i18n( "Both keys allow selecting files. To select more than one file, hold the Ctrl key and click the left mouse button.\nRight-click menu is invoked using a short click on the right mouse button." ) },
     { i18n( "Konqueror Mode" ),       "1", i18n( "Pressing the left mouse button selects files - you can click and select multiple files.\nRight-click menu is invoked using a short click on the right mouse button." ) },    
     { i18n( "Total-Commander Mode" ), "2", i18n( "The left mouse button does not select, but sets the current file without affecting the current selection.\nThe right mouse button selects multiple files and the right-click menu is invoked by pressing\nand holding the right mouse button." ) },
	  { i18n( "Custom Selection Mode" ),       "3", i18n( "Design your own selection mode!" ) }};
 mouseRadio = createRadioButtonGroup( "Look&Feel", "Mouse Selection",
      "0", 1, 0, mouseSelection, 4, lookFeelGrp, "myLook&FeelRadio", true );
  lookFeelGrid->addWidget( mouseRadio, 11, 0 );
  connect(mouseRadio, SIGNAL(clicked(int)), this, SLOT(slotSelectionModeChanged(int)));
	
  lookAndFeelLayout->addWidget( lookFeelGrp, 0, 0 );
  
  //  ---------------------------- PANEL TAB -------------------------------------
  tab_panel = new QWidget( tabWidget, "tab_panel" );
  tabWidget->insertTab( tab_panel, i18n( "Panel" ) );

  QGridLayout *panelLayout = new QGridLayout( tab_panel );
  panelLayout->setSpacing( 6 );
  panelLayout->setMargin( 11 );
  QGroupBox *panelGrp = createFrame( i18n( "Panel settings" ), tab_panel, "kgPanelGrp" );
  QGridLayout *panelGrid = createGridLayout( panelGrp->layout() );

  QHBox *hbox = new QHBox( panelGrp, "lookAndFeelHBox1" );
  new QLabel( i18n( "Panel font:" ), hbox, "lookAndFeelLabel" );
  createFontChooser( "Look&Feel", "Filelist Font", _FilelistFont, hbox, true );
  createSpacer ( hbox );
  panelGrid->addWidget( hbox, 0, 0 );

  QHBox *hbox2 = new QHBox( panelGrp, "lookAndFeelHBox2" );
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
  panelGrid->addWidget( hbox2, 1, 0 );

  panelGrid->addWidget( createLine( panelGrp, "lookSep2" ), 2, 0 );

  addLabel( panelGrid, 3, 0, i18n( "Show Columns:" ),
            panelGrp, "panelLabel" );

  KONFIGURATOR_CHECKBOX_PARAM fields[] =
  //   cfg_class  cfg_name                default text                                  restart tooltip
    {{"Look&Feel","Ext Column",           true,   i18n( "Extension" ),           true ,  i18n( "Show the last part of the filename (part after the last dot) in the Extension Column and\nnot as a complete filename in the Name Column like e.g. Konqueror does." ) },
     {"Look&Feel","Mime Column",          false,  i18n( "Mimetype" ),            true ,  i18n( "Show the Mimetype Column" ) },
     {"Look&Feel","Size Column",          true,   i18n( "Size" ),                true ,  i18n( "Show the Size Column" ) },
     {"Look&Feel","DateTime Column",      true,   i18n( "Date and Time" ),       true ,  i18n( "Show the Date and Time Column" ) },
     {"Look&Feel","Perm Column",          false,  i18n( "Permission" ),          true ,  i18n( "Show the Full Permissions Column, e.g. '-rw-r--r--'" ) },
     {"Look&Feel","KrPerm Column",        true,   i18n( "Krusader Permission" ), true ,  i18n( "Show only the rights of the current user, e.g. '-rw'" ) },
     {"Look&Feel","Owner Column",         false,  i18n( "Owner" ),               true ,  i18n( "Show the Owner Column" ) },
     {"Look&Feel","Group Column",         false,  i18n( "Group" ),               true ,  i18n( "Show the Group Column" ) },
    };

  KonfiguratorCheckBoxGroup *flds = createCheckBoxGroup( 0, 4, fields, 8, panelGrp );
  panelGrid->addWidget( flds, 4, 0 );

  panelGrid->addWidget( createLine( panelGrp, "lookSep3" ), 5, 0 );

  KONFIGURATOR_CHECKBOX_PARAM panelSettings[] =
  //   cfg_class  cfg_name                default text                                  restart tooltip
    {
	 {"Look&Feel","With Icons",           _WithIcons,   i18n( "Use icons in the filenames" ), true ,  i18n( "Show the icons for filenames and folders." ) },
	 {"Look&Feel","Human Readable Size",  _HumanReadableSize, i18n( "Use human-readable file size" ), true ,  i18n( "File sizes are displayed in B, KB, MB and GB, not just in bytes." ) },
    };

  KonfiguratorCheckBoxGroup *panelSett = createCheckBoxGroup( 0, 2, panelSettings, 2, panelGrp );
  panelGrid->addWidget( panelSett, 6, 0 );
  
  panelLayout->addWidget( panelGrp, 0, 0 );

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
  keyBindings = new KonfiguratorKeyChooser(krApp->actionCollection(),tab_3);
  connect( keyBindings, SIGNAL( reload( KonfiguratorKeyChooser * ) ), this, SLOT( slotReload( KonfiguratorKeyChooser *) ) );
  keyBindingsLayout->addMultiCellWidget(keyBindings->keyChooserWidget(),0,0,0,2);
  registerObject( keyBindings );
  
  // import and export shortcuts
  KPushButton *importBtn = new KPushButton(i18n("Import shortcuts"),tab_3);
  keyBindingsLayout->addWidget(importBtn,1,0);
  QToolTip::add(  importBtn, i18n( "Load a Key-binding profile, e.g., total_commander.keymap" ) );
  KPushButton *exportBtn = new KPushButton(i18n("Export shortcuts"),tab_3);
  keyBindingsLayout->addWidget(exportBtn,1,1);
  QToolTip::add(  exportBtn, i18n( "Save current Key-bindings in a keymap file." ) );
  keyBindingsLayout->addWidget(createSpacer(tab_3, "tab3spacer"), 1,2);
  connect(importBtn, SIGNAL(clicked()), this, SLOT(slotImportShortcuts()));
  connect(exportBtn, SIGNAL(clicked()), this, SLOT(slotExportShortcuts()));

  //  -------------------------- Panel Toolbar TAB ----------------------------------
  QWidget     *tab_4 = new QWidget( tabWidget, "tab_4" );
  tabWidget->insertTab( tab_4, i18n( "Panel Toolbar" ) );

  QBoxLayout * panelToolbarVLayout = new QVBoxLayout( tab_4 );
  panelToolbarVLayout->setSpacing( 6 );
  panelToolbarVLayout->setMargin( 11 );

  KONFIGURATOR_CHECKBOX_PARAM panelToolbarActiveCheckbox[] = 
  //   cfg_class    cfg_name                default        text                             restart tooltip
    {{"Look&Feel", "Panel Toolbar visible", _PanelToolBar, i18n( "Show Panel Toolbar" ), false,  i18n( "Krusader will display the Panel Toolbar." ) }
  };

  panelToolbarActive = createCheckBoxGroup( 1, 0, panelToolbarActiveCheckbox, 1, tab_4, "panelToolbarActive");
  connect( panelToolbarActive->find( "Panel Toolbar visible" ), SIGNAL( stateChanged( int ) ), this, SLOT( slotEnablePanelToolbar() ) );
    
  QGroupBox * panelToolbarGrp = createFrame( i18n( "Visible Panel Toolbar buttons" ), tab_4, "panelToolbarGrp");
  QGridLayout * panelToolbarGrid = createGridLayout( panelToolbarGrp->layout() );

  KONFIGURATOR_CHECKBOX_PARAM panelToolbarCheckboxes[] = 
    {
  //   cfg_class    cfg_name                default             text                       restart tooltip
     {"Look&Feel",  "Open Button Visible",  _Open,      i18n( "Open button" ),     true ,  i18n( "Opens the directory browser." ) },
     {"Look&Feel",  "Equal Button Visible", _cdOther,   i18n( "Equal button (=)" ),true ,  i18n( "Changes the panel directory to the other panel directory." ) },
     {"Look&Feel",  "Up Button Visible",    _cdUp,      i18n( "Up button (..)" ),  true ,  i18n( "Changes the panel directory to the parent directory." ) },
     {"Look&Feel",  "Home Button Visible",  _cdHome,    i18n( "Home button (~)" ), true ,  i18n( "Changes the panel directory to the home directory." ) },
     {"Look&Feel",  "Root Button Visible",  _cdRoot,    i18n( "Root button (/)" ), true ,  i18n( "Changes the panel directory to the root directory." ) },
     {"Look&Feel",  "SyncBrowse Button Visible",  _syncBrowseButton,    i18n( "Toggle-button for sync-browsing" ), true ,  i18n( "Each directory-change in the panel is also performed in the other panel." ) },
    };
  
  
  pnlcbs = createCheckBoxGroup(1, 0, panelToolbarCheckboxes, 6,
                                       panelToolbarGrp, "panelToolbarChecks");
  
  panelToolbarVLayout->addWidget( panelToolbarActive, 0, 0 );
  panelToolbarGrid->addWidget( pnlcbs, 0, 0 );
  panelToolbarVLayout->addWidget( panelToolbarGrp,    1, 0 );

  // Enable panel toolbar checkboxes
  slotEnablePanelToolbar();

  slotDisable();

  kgLookAndFeelLayout->addWidget( tabWidget, 0, 0 );
}

void KgLookFeel::slotReload( KonfiguratorKeyChooser * oldChooser )
{
  removeObject( oldChooser );
  delete oldChooser;

  keyBindings = new KonfiguratorKeyChooser(krApp->actionCollection(),tab_3);
  connect( keyBindings, SIGNAL( reload( KonfiguratorKeyChooser * ) ), this, SLOT( slotReload( KonfiguratorKeyChooser *) ) );
  keyBindingsLayout->addMultiCellWidget(keyBindings->keyChooserWidget(),0,0,0,2);
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

void KgLookFeel::slotDisable()
{
  bool isNewStyleQuickSearch = cbs->find( "New Style Quicksearch" )->isChecked();
  cbs->find( "Case Sensitive Quicksearch" )->setEnabled( isNewStyleQuickSearch );
}

void KgLookFeel::slotEnablePanelToolbar()
{
  bool enableTB = panelToolbarActive->find("Panel Toolbar visible")->isChecked();
  pnlcbs->find( "Root Button Visible"     )->setEnabled(enableTB);
  pnlcbs->find( "Home Button Visible"     )->setEnabled(enableTB);
  pnlcbs->find( "Up Button Visible"       )->setEnabled(enableTB);
  pnlcbs->find( "Equal Button Visible"    )->setEnabled(enableTB);
  pnlcbs->find( "Open Button Visible"     )->setEnabled(enableTB);  
}

void KgLookFeel::slotImportShortcuts() {
	// find $KDEDIR/share/apps/krusader
	QString basedir = KGlobal::dirs()->findResourceDir("appdata", "total_commander.keymap");
	// let the user select a file to load
	QString file = KFileDialog::getOpenFileName(basedir, "*.keymap", 0, i18n("Select a shortcuts file"));
	if (file == QString::null) return;
	// check if there's an info file with the keymap
	QFile info(file+".info");
	if (info.open(IO_ReadOnly)) {
		QTextStream stream(&info);
		QStringList infoText = QStringList::split("\n", stream.read());
		if (KMessageBox::questionYesNoList(krApp, i18n("The following information was attached to the keymap.\n"
				"Are you sure you want to import this keymap ?"), infoText)!=KMessageBox::Yes)
			return;
	}
	// ok, import away
	krApp->importKeyboardShortcuts(file);
	slotReload(keyBindings);	
	keyBindings->setChanged();
}

void KgLookFeel::slotExportShortcuts() {
	QString file = KFileDialog::getSaveFileName(QString::null, "*", 0, i18n("Select a shortcuts file"));
	if (file == QString::null) return;
	krApp->exportKeyboardShortcuts(file);
}

void KgLookFeel::slotSelectionModeChanged(int mode) {
	// mode 3 == user defined mode (ugly, but no #defines yet)
	if (mode == 3) {
		if (UserSelectionModeDlg::createCustomMode(this) == QDialog::Accepted)
			mouseRadio->extension()->setChanged();
	}
}

#include "kglookfeel.moc"

