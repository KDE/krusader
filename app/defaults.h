/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DEFAULTS_H
#define DEFAULTS_H

// QtGui
#include <QFontDatabase>

///////////////////////   [Startup]
// UI Save component Settings
#define _UiSave true
// Show Cmd Line
#define _ShowCmdline false
// Show status bar
#define _ShowStatusBar true
// Show actions tool bar
#define _ShowActionsToolBar true
// Show tool bar
#define _ShowToolBar true
// Show FN Keys
#define _ShowFNkeys true
// Show Terminal Emulator
#define _ShowTerminalEmulator false
// Remember Position
#define _RememberPos true
// Start to tray
#define _StartToTray false
// The position of the tab bar
#define _TabBarPosition "top"
// Left Tab Bar
// Right Tab Bar
// Size where lister is the default viewer
#define _ListerLimit 10

////////////////////////[Look&Feel]
// Filelist Font ///////
#define _FilelistFont QFontDatabase::systemFont(QFontDatabase::GeneralFont)
// Warn On Exit ////////
#define _WarnOnExit false
// Minimize To Tray ////
#define _ShowTrayIcon false
// Mark Dirs ///////////
#define _MarkDirs false
// Show Hidden /////////
#define _ShowHidden true
// Sort By Extension ///
#define _SortByExt false
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
// Always show current item decoration in panel
#define _AlwaysShowCurrentItem true
// Unslect files before copy/move
#define _UnselectBeforeOperation false
// Filter dialog remembers settings
#define _FilterDialogRemembersSettings false

// Panel Toolbar Checkboxes
// Panel Toolbar Visible checkbox turned off
#define _PanelToolBar true
// cd / is turned on
#define _cdRoot true
// cd ~ is turned on
#define _cdHome false
// cd .. is turned on
#define _cdUp true
// cd other panel is turned on
#define _cdOther false
// syncBrowseButton is turned on
#define _syncBrowseButton false
// Use the default colors of KDE
#define _KDEDefaultColors true
// Enable Alternate Background colors
#define _AlternateBackground true
// Show current item even if not focused
#define _ShowCurrentItemAlways false
// Dim the colors of the inactive panel
#define _DimInactiveColors false
// Human Readable Size
#define _HumanReadableSize true
// With Icons
#define _WithIcons true
// Single Click Selects
#define _SingleClickSelects false
// Numeric Permissions
#define _NumericPermissions false
// Number of Columns in the Brief View
#define _NumberOfBriefColumns 3
// Default Sort Method
#define _DefaultSortMethod KrViewProperties::Krusader
// Show splashscreen
#define _ShowSplashScreen false
// Single instance mode
#define _SingleInstanceMode false

/////////////////////// [General]
// Move To Trash //////
#define _MoveToTrash true
// Terminal ///////////
#define _Terminal "konsole --separate"
// Send CDs ///////////
#define _SendCDs true
// Follow Terminal CD ///////////
#define _FollowTerminalCD true
// Editor /////////////
#define _Editor "internal editor"
// Use Okteta as Hex viewer ///////
#define _UseOktetaViewer false
// Temp Directory /////
#define _TempDirectory "/tmp/krusader.tmp"
// Classic Quicksearch
#define _NewStyleQuicksearch true
// Case Sensitive quick search, if _NewStyleQuicksearch is true
#define _CaseSensitiveQuicksearch false
// Special handling of Right Arrow in Quicksearch
#define _NavigationWithRightArrowQuicksearch true
// View In Separate Window
#define _ViewInSeparateWindow false
// Hide Single Tab in Viewer
#define _ViewerHideSingleTab false

/////////////////////// [Advanced]
// Permission Check ///
// #define _PermCheck     true
// AutoMount //////////
#define _AutoMount false
// Preserving date //////////
#define _PreserveAttributes false
// Nonmount Points ////
#define _NonMountPoints "/, "
// Confirm Unempty Dir //     (for delete)
#define _ConfirmUnemptyDir true
// Confirm Delete /////       (for deleting files)
#define _ConfirmDelete true
// Confirm Copy ///////       (for copying files)
#define _ConfirmCopy true
// Confirm Move ///////       (for moving files)
#define _ConfirmMove true
// Icon Cache Size ////
#define _IconCacheSize 2048

/////////////////////// [Archives]
// Do Tar /////////////
#define _DoTar true
// Do GZip ////////////
#define _DoGZip true
// Do Zip /////////////
#define _DoZip true
// Do UnZip ///////////
#define _DoUnZip true
// Do BZip2 ///////////
#define _DoBZip2 true
// Do LZMA ///////////
#define _DoLZMA true
// Do XZ ///////////
#define _DoXZ true
// Do Rar /////////////
#define _DoRar true
// Do UnRar ///////////
#define _DoUnRar true
// Do UnAce ///////////
#define _DoUnAce true
// Do Arj /////////////
#define _DoArj true
// Do UnArj ///////////
#define _DoUnarj true
// Do RPM /////////////
#define _DoRPM true
// Do DEB /////////////          ====> new
#define _DoDEB true
// Do Lha /////////////
#define _DoLha true
// Do 7z /////////////          ====> new
#define _Do7z true
// Allow Move Into Archive //
#define _MoveIntoArchive false
// Test Archives //////
#define _TestArchives false
// Test Before Unpack ////
#define _TestBeforeUnpack true
// Supported Packers // ====> a QStringList of SYSTEM supported archives ( also new )
// default compression level
#define _defaultCompressionLevel 5
// treat Archives as Directories
#define _ArchivesAsDirectories true

/////////////////////// [UserActions]
// Terminal for UserActions ///////////
#define _UserActions_Terminal "konsole --noclose --workdir %d --title %t -e"
// Normal font for output collection ///////
#define _UserActions_NormalFont QFontDatabase::systemFont(QFontDatabase::GeneralFont)
// Font for output collection with fixed width ///////
#define _UserActions_FixedFont QFontDatabase::systemFont(QFontDatabase::FixedFont)
// Use for output collection  fixed width font as default ///////
#define _UserActions_UseFixedFont false

/////////////////////// [Private]
// Start Position /////
#define _StartPosition QPoint(QApplication::primaryScreen()->geometry().width() / 2 - MAIN_VIEW->sizeHint().width() / 2, QApplication::primaryScreen()->geometry().height() / 2 - 250)
// Start Size /////////
#define _StartSize QSize(MAIN_VIEW->sizeHint().width(), 500)
// Panel Size /////////
#define _PanelSize 0
// Terminal Size //////
#define _TerminalSize 0
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
#define _ConfirmFeedToListbox true

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
#define _ConfirmOverWrites false
// Recursive search in the subdirectories /////////////
#define _RecurseSubdirs true
// The searcher follows symlinks /////////////
#define _FollowSymlinks false
// Files with similar size are compared by content /////////////
#define _CompareByContent false
// The date information is ignored at synchronization /////////////
#define _IgnoreDate false
// Asymmetric Client-File Server compare mode /////////////
#define _Asymmetric false
// Case insensitive compare in synchronizer /////////////
#define _IgnoreCase false
// Scrolls the results of the synchronization /////////////
#define _ScrollResults false
// The right arrow button is turned on /////////////
#define _BtnLeftToRight true
// The equals button is turned on /////////////
#define _BtnEquals true
// The not equals button is turned on /////////////
#define _BtnDifferents true
// The left arrow button is turned on /////////////
#define _BtnRightToLeft true
// The trash button is turned on /////////////
#define _BtnDeletable true
// The duplicates button is turned on /////////////
#define _BtnDuplicates true
// The singles button is turned on /////////////
#define _BtnSingles true

/////////////////////// [Custom Selection Mode]
// QT Selection
#define _QtSelection false
// Left Selects
#define _LeftSelects true
// Left Preserves
#define _LeftPreserves false
// ShiftCtrl Left Selects
#define _ShiftCtrlLeft false
// Right Selects
#define _RightSelects true
// Right Preserves
#define _RightPreserves false
// ShiftCtrl Right Selects
#define _ShiftCtrlRight false
// Space Moves Down
#define _SpaceMovesDown true
// Space Calc Space
#define _SpaceCalcSpace true
// Insert Moves Down
#define _InsertMovesDown true
// Immediate Context Menu
#define _ImmediateContextMenu true
// Reset selection items
#define _ResetSelectionItems false

// Root directory
#ifdef Q_OS_WIN
#define DIR_SEPARATOR "/"
#define DIR_SEPARATOR2 "\\"
#define DIR_SEPARATOR_CHAR '/'
#define DIR_SEPARATOR_CHAR2 '\\'
#define REPLACE_DIR_SEP2(x) x = x.replace(DIR_SEPARATOR2, DIR_SEPARATOR);
#define ROOT_DIR "C:\\"
#define EXEC_SUFFIX ".exe"
#else
#define DIR_SEPARATOR "/"
#define DIR_SEPARATOR2 "/"
#define DIR_SEPARATOR_CHAR '/'
#define DIR_SEPARATOR_CHAR2 '/'
#define REPLACE_DIR_SEP2(x)
#define ROOT_DIR "/"
#define EXEC_SUFFIX ""
#endif

#endif
