/***************************************************************************
                                defaults.h
                             -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
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

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

///////////////////////   [Startup]
// UI Save Settings
#define _UiSave           false
// Show Cmd Line
#define _ShowCmdline      true
// Show status bar
#define _ShowStatusBar    true
// Show tool bar
#define _ShowToolBar      true
// Show FN Keys
#define _ShowFNkeys       true
// Show Terminal Emulator
#define _ShowTerminalEmulator false
// Remember Position
#define _RememberPos      true
// Panels Save Settings
#define _PanelsSave       false
// Left Panel Type
#define _LeftPanelType    "List"
// Left Panel Origin
#define _LeftPanelOrigin  "homepage"
// Left Panel Homepage
#define _LeftHomepage     "/"
// Right Panel Type
#define _RightPanelType   "List"
// Right Panel Origin
#define _RightPanelOrigin "homepage"
// Right Panel Homepage
#define _RightHomepage     "/"

////////////////////////[Look&Feel]
// Filelist Font ///////
#define _FilelistFont   new QFont("helvetica",12)
// Warn On Exit ////////
#define _WarnOnExit     false
// Minimize To Tray ////
#define _MinimizeToTray false
// Mark Dirs ///////////
#define _MarkDirs       false
// Show Hidden /////////
#define _ShowHidden     true
// Sort By Extention ///
#define _SortByExt      false
// Case Sensative Sort /
#define _CaseSensativeSort false
// Html Min Font Size //
#define _HtmlMinFontSize 12
// Filelist Icon Size //
#define _FilelistIconSize QString("22")
// Mouse Selection /////
#define _MouseSelection 0 // 0 - normal (shift+click, ctrl+click), 1 - left click, 2 - right click

/////////////////////// [General]
// Mimetype Magic /////
#define _MimetypeMagic true
// Move To Trash //////
#define _MoveToTrash   false
// Terminal ///////////
#define _Terminal      "konsole"
// Editor /////////////
#define _Editor        "internal editor"
// Temp Directory /////
#define _TempDirectory "/tmp/krusader.tmp"

/////////////////////// [Advanced]
// Permission Check ///
//#define _PermCheck     true
// AutoMount //////////
#define _AutoMount     false
// Confirm Unempty Dir //     (for delete)
#define _ConfirmUnemptyDir true
// Confirm Delete /////       (for deleting files)
#define _ConfirmDelete true
// Confirm Copy ///////       (for copying files)
#define _ConfirmCopy   true
// Confirm Move ///////       (for moving files)
#define _ConfirmMove   true
// Icon Cache Size ////
#define _IconCacheSize 2048

/////////////////////// [Archives]
// Do Tar /////////////
#define _DoTar         true
// Do GZip ////////////
#define _DoGZip        true
// Do Zip /////////////
#define _DoZip         true
// Do UnZip ///////////
#define _DoUnZip       true
// Do BZip2 ///////////
#define _DoBZip2       true
// Do Rar /////////////
#define _DoRar         true
// Do UnRar ///////////
#define _DoUnRar       true
// Do UnAce ///////////
#define _DoUnAce       true
// Do Arj /////////////          ====> new
#define _DoArj         true
// Do UnArj ///////////          ====> new
#define _DoUnarj       true
// Do RPM /////////////          ====> new
#define _DoRPM         true
// Allow Move Into Archive //
#define _MoveIntoArchive false
// Test Archives //////
#define _TestArchives false
// Supported Packers // ====> a QStringList of SYSTEM supported archives ( also new )

/////////////////////// [Bookmarks]
// Links //////////////
// the basic link (/) is defined internally
// Sorted /////////////
#define _Sorted        true

/////////////////////// [Private]
// Start Position /////
#define _StartPosition new QPoint(QApplication::desktop()->width()/2 - mainView->sizeHint().width()/2,QApplication::desktop()->height()/2 - 250)
// Start Size /////////
#define _StartSize     new QSize(mainView->sizeHint().width(),500)
// Panel Size /////////
#define _PanelSize     0
// Terminal Size //////
#define _TerminalSize  0
// Left Name Size  - size of the left panel's name column
// Left Size Size  - size of the left panel's size column
// Left Date Size  - size of the left panel's date column
// Right Name Size - size of the right panel's name column
// Right Size Size - size of the left panel's size column
// Right Date Size - size of the left panel's date column
// Splitter Sizes - sizes of the splitter

/////////////////////// [RemoteMan]
// Connections ////////
// the basic connections are defined internally

/////////////////////// [Search]
// Saved Searches /////
// holds an index of saved searches

/////////// here are additional variables used internally by Krusader ////////////
// BookmarkArchives   - The infobox about not allowing bookmarks inside archives
// BackArchiveWarning - The infobox about not allowing to back up into archives
// SupermountWarning  - Warning about mounting/unmounting supermount filesystems
// lastHomeRight      - Save the last place the right list panel was showing
// lastHomeLeft       - Save the last place the left list panel was showing
// lastUsedPacker     - used by packGUI to remember the last used packer
