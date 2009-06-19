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

#ifndef DEFAULTS_H
#define DEFAULTS_H

#include <kglobalsettings.h>

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
// Start to tray
#define _StartToTray      false
// Left Tab Bar
// Right Tab Bar
// Left Panel Popup
#define _LeftPanelPopup  0
// Right Panel Popup
#define _RightPanelPopup 0


////////////////////////[Look&Feel]
// Filelist Font ///////
#define _FilelistFont   new QFont(KGlobalSettings::generalFont())
// Warn On Exit ////////
#define _WarnOnExit     false
// Minimize To Tray ////
#define _MinimizeToTray false
// Mark Dirs ///////////
#define _MarkDirs       false
// Show Hidden /////////
#define _ShowHidden     true
// Sort By Extension ///
#define _SortByExt      false
// Case Sensative Sort /
#define _CaseSensativeSort false
// Html Min Font Size //
#define _HtmlMinFontSize 12
// Filelist Icon Size //
#define _FilelistIconSize QString("22")
// Mouse Selection /////
#define _MouseSelection 0 // 0 - normal (shift+click, ctrl+click), 1 - left click, 2 - right click
// Use fullpath tab names /////
#define _FullPathTabNames false
// User defined folder icons
#define _UserDefinedFolderIcons true

// Panel Toolbar Checkboxes
// Panel Toolbar Visible checkbox turned off
#define _PanelToolBar       true
// cd / is turned on
#define _cdRoot             true
// cd ~ is turned on
#define _cdHome             false
// cd .. is turned on
#define _cdUp               true
// cd other panel is turned on
#define _cdOther            false
// Open directory is turned on
#define _Open               false
// syncBrowseButton is turned on
#define _syncBrowseButton   false
// Use the default colors of KDE
#define _KDEDefaultColors     true
// Enable Alternate Background colors
#define _AlternateBackground  true
// Show current item even if not focused
#define _ShowCurrentItemAlways false
// Dim the colors of the inactive panel
#define _DimInactiveColors     false
// Human Readable Size
#define _HumanReadableSize  true
// With Icons
#define _WithIcons          true
// Single Click Selects
#define _SingleClickSelects false
// Numeric Permissions
#define _NumericPermissions false
// Number of Columns in the Brief View
#define _NumberOfBriefColumns  3
// Default Sort Method
#define _DefaultSortMethod KrViewProperties::Krusader
// Show splashscreen
#define _ShowSplashScreen true
// Single instance mode
#define _SingleInstanceMode false

/////////////////////// [General]
// Mimetype Magic /////
#define _MimetypeMagic true
// Move To Trash //////
#define _MoveToTrash   false
// Terminal ///////////
#define _Terminal      "konsole --workdir %d"
// Send CDs ///////////
#define _SendCDs  true
// Editor /////////////
#define _Editor        "internal editor"
// Temp Directory /////
#define _TempDirectory "/tmp/krusader.tmp"
// Classic Quicksearch
#define _NewStyleQuicksearch true
// Case Sensitive quick search, if _NewStyleQuicksearch is true
#define _CaseSensitiveQuicksearch false
// View In Separate Window
#define _ViewInSeparateWindow  false

/////////////////////// [Advanced]
// Permission Check ///
//#define _PermCheck     true
// AutoMount //////////
#define _AutoMount        false
// Preserving date //////////
#define _PreserveAttributes false
// Nonmount Points ////
#define _NonMountPoints "/, "
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
// Do LZMA ///////////
#define _DoLZMA       true
// Do Rar /////////////
#define _DoRar         true
// Do UnRar ///////////
#define _DoUnRar       true
// Do UnAce ///////////
#define _DoUnAce       true
// Do Arj /////////////
#define _DoArj         true
// Do UnArj ///////////
#define _DoUnarj       true
// Do RPM /////////////
#define _DoRPM         true
// Do DEB /////////////          ====> new
#define _DoDEB         true
// Do Lha /////////////
#define _DoLha         true
// Do 7z /////////////          ====> new
#define _Do7z          true
// Allow Move Into Archive //
#define _MoveIntoArchive false
// Test Archives //////
#define _TestArchives  false
// Test Before Unpack ////
#define _TestBeforeUnpack true
// Supported Packers // ====> a QStringList of SYSTEM supported archives ( also new )
// default compression level
#define _defaultCompressionLevel 5

/////////////////////// [UserActions]
// Terminal for UserActions ///////////
#define _UserActions_Terminal      "konsole --noclose -e"
// Normal font for output collection ///////
#define _UserActions_NormalFont   new QFont(KGlobalSettings::generalFont())
// Font for output collection with fixed width ///////
#define _UserActions_FixedFont   new QFont(KGlobalSettings::fixedFont())
// Use for output collection  fixed width font as default ///////
#define _UserActions_UseFixedFont   false

/////////////////////// [Private]
// Start Position /////
#define _StartPosition QPoint(QApplication::desktop()->width()/2 - mainView->sizeHint().width()/2,QApplication::desktop()->height()/2 - 250)
// Start Size /////////
#define _StartSize     QSize(mainView->sizeHint().width(),500)
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
// Confirm Feed to Listbox ///// (costum-name on feed ti listbox)
#define _ConfirmFeedToListbox   true


/////////// here are additional variables used internally by Krusader ////////////
// BookmarkArchives   - The infobox about not allowing bookmarks inside archives
// BackArchiveWarning - The infobox about not allowing to back up into archives
// SupermountWarning  - Warning about mounting/unmounting supermount filesystems
// lastHomeRight      - Save the last place the right list panel was showing
// lastHomeLeft       - Save the last place the left list panel was showing
// lastUsedPacker     - used by packGUI to remember the last used packer

/////////////////////// [Popular Urls]
// PopularUrls     - a string list containing the top urls
// PopularUrlsRank - an int list contains the urls' ranking

/////////////////////// [Synchronize directories]
// Don't overwrite automatically /////////////
#define  _ConfirmOverWrites   false
// Recursive search in the subdirectories /////////////
#define  _RecurseSubdirs    true
// The searcher follows symlinks /////////////
#define  _FollowSymlinks    false
// Files with similar size are compared by content /////////////
#define  _CompareByContent  false
// The date information is ignored at synchronization /////////////
#define  _IgnoreDate        false
// Asymmetric Client-File Server compare mode /////////////
#define  _Asymmetric        false
// Case insensitive compare in synchronizer /////////////
#define  _IgnoreCase        false
// Scrolls the results of the synchronization /////////////
#define  _ScrollResults     false
// The right arrow button is turned on /////////////
#define  _BtnLeftToRight    true
// The equals button is turned on /////////////
#define  _BtnEquals         true
// The not equals button is turned on /////////////
#define  _BtnDifferents     true
// The left arrow button is turned on /////////////
#define  _BtnRightToLeft    true
// The trash button is turned on /////////////
#define  _BtnDeletable      true
// The duplicates button is turned on /////////////
#define  _BtnDuplicates     true
// The singles button is turned on /////////////
#define  _BtnSingles        true

/////////////////////// [Custom Selection Mode]
// QT Selection
#define _QtSelection   false
// Left Selects
#define _LeftSelects   true
// Left Preserves
#define _LeftPreserves  false
// ShiftCtrl Left Selects
#define _ShiftCtrlLeft  false
// Right Selects
#define _RightSelects  true
// Right Preserves
#define _RightPreserves  false
// ShiftCtrl Right Selects
#define _ShiftCtrlRight  false
// Space Moves Down
#define _SpaceMovesDown  true
// Space Calc Space
#define _SpaceCalcSpace  true
// Insert Moves Down
#define _InsertMovesDown true
// Immediate Context Menu
#define _ImmediateContextMenu true

// Root directory
#ifdef Q_WS_WIN
#define DIR_SEPARATOR       "/"
#define DIR_SEPARATOR2      "\\"
#define DIR_SEPARATOR_CHAR  '/'
#define DIR_SEPARATOR_CHAR2 '\\'
#define REPLACE_DIR_SEP2(x) x = x.replace( DIR_SEPARATOR2, DIR_SEPARATOR );
#define ROOT_DIR "C:\\"
#define EXEC_SUFFIX         ".exe"
#else
#define DIR_SEPARATOR       "/"
#define DIR_SEPARATOR2      "/"
#define DIR_SEPARATOR_CHAR  '/'
#define DIR_SEPARATOR_CHAR2 '/'
#define REPLACE_DIR_SEP2(x)
#define ROOT_DIR            "/"
#define EXEC_SUFFIX         ""
#endif

#endif
