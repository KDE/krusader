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

namespace Defaults
{
///////////////////////   [Startup]
// UI Save component Settings
constexpr bool uiSave = true;
// Show Cmd Line
constexpr bool showCmdline = false;
// Show FN Keys
constexpr bool showFNkeys = true;
// Show Terminal Emulator
constexpr bool showTerminalEmulator = false;
// Remember Position
constexpr bool rememberPos = true;
// Start to tray
constexpr bool startToTray = false;
// The position of the tab bar
constexpr const char *tabBarPosition = "top";
// Left Tab Bar
// Right Tab Bar
// Size where lister is the default viewer
constexpr int listerLimit = 10;

////////////////////////[Look&Feel]
// Filelist Font ///////
inline QFont filelistFont()
{
    return QFontDatabase::systemFont(QFontDatabase::GeneralFont);
}

// Warn On Exit ////////
constexpr bool warnOnExit = false;
// Minimize To Tray ////
constexpr bool showTrayIcon = false;
// Mark Dirs ///////////
constexpr bool markDirs = false;
// Show Hidden /////////
constexpr bool showHidden = true;
// Case Sensative Sort /
constexpr bool caseSensativeSort = false;
// Filelist Icon Size //
constexpr int filelistIconSize = 22;
// Use fullpath tab names /////
constexpr bool fullPathTabNames = false;
// User defined folder icons
constexpr bool userDefinedFolderIcons = true;
// Always show current item decoration in panel
constexpr bool alwaysShowCurrentItem = true;
// Unslect files before copy/move
constexpr bool unselectBeforeOperation = false;
// Filter dialog remembers settings
constexpr bool filterDialogRemembersSettings = false;

// Panel Toolbar Checkboxes
// Panel Toolbar Visible checkbox turned off
constexpr bool panelToolBar = true;
// cd / is turned on
constexpr bool cdRoot = true;
// cd ~ is turned on
constexpr bool cdHome = false;
// cd .. is turned on
constexpr bool cdUp = true;
// cd other panel is turned on
constexpr bool cdOther = false;
// syncBrowseButton is turned on
constexpr bool syncBrowseButton = false;
// Use the default colors of KDE
constexpr bool plasmaDefaultColors = true;
// Enable Alternate Background colors
constexpr bool alternateBackground = true;
// Show current item even if not focused
constexpr bool showCurrentItemAlways = false;
// Dim the colors of the inactive panel
constexpr bool dimInactiveColors = false;
// Human Readable Size
constexpr bool humanReadableSize = true;
// With Icons
constexpr bool withIcons = true;
// Single Click Selects
constexpr bool singleClickSelects = false;
// Numeric Permissions
constexpr bool numericPermissions = false;
// Number of Columns in the Brief View
constexpr int numberOfBriefColumns = 3;
// Show splashscreen
constexpr bool showSplashScreen = false;
// Single instance mode
constexpr bool singleInstanceMode = false;

/////////////////////// [General]
// Move To Trash //////
constexpr bool moveToTrash = true;
// Send CDs ///////////
constexpr bool sendCDs = true;
// Follow Terminal CD ///////////
constexpr bool followTerminalCD = true;
// Editor /////////////
constexpr const char *editor = "internal editor";
// Use Okteta as Hex viewer ///////
constexpr bool useOktetaViewer = false;
// Temp Directory /////
constexpr const char *tempDirectory = "/tmp/krusader.tmp";
// Classic Quicksearch
constexpr bool newStyleQuicksearch = true;
// Case Sensitive quick search, if Defaults::newStyleQuicksearch is true
constexpr bool caseSensitiveQuicksearch = false;
// Special handling of Right Arrow in Quicksearch
constexpr bool navigationWithRightArrowQuicksearch = true;
// View In Separate Window
constexpr bool viewInSeparateWindow = false;
// Hide Single Tab in Viewer
constexpr bool viewerHideSingleTab = false;

/////////////////////// [Advanced]
// Permission Check ///
// constexpr bool permCheck = true;
// AutoMount //////////
constexpr bool autoMount = false;
// Nonmount Points ////
constexpr const char *nonMountPoints = "/, ";
// Confirm Unempty Dir //     (for delete)
constexpr bool confirmUnemptyDir = true;
// Confirm Delete /////       (for deleting files)
constexpr bool confirmDelete = true;
// Confirm Copy ///////       (for copying files)
constexpr bool confirmCopy = true;
// Confirm Move ///////       (for moving files)
constexpr bool confirmMove = true;
// Icon Cache Size ////
constexpr int iconCacheSize = 2048;

/////////////////////// [Archives]
// Test Archives //////
constexpr bool testArchives = false;
// Test Before Unpack ////
constexpr bool testBeforeUnpack = true;
// Supported Packers // ====> a QStringList of SYSTEM supported archives ( also new )
// default compression level
constexpr int defaultCompressionLevel = 5;
// treat Archives as Directories
constexpr bool archivesAsDirectories = true;

/////////////////////// [UserActions]
// Terminal for UserActions ///////////
constexpr const char *userActionsTerminal = "konsole --noclose --workdir %d --title %t -e";
// Normal font for output collection ///////
inline QFont userActionsNormalFont()
{
    return QFontDatabase::systemFont(QFontDatabase::GeneralFont);
}
// Font for output collection with fixed width ///////
inline QFont userActionsFixedFont()
{
    return QFontDatabase::systemFont(QFontDatabase::FixedFont);
}
// Use for output collection  fixed width font as default ///////
constexpr bool userActionsUseFixedFont = false;

/////////////////////// [Search]
// Saved Searches /////
// holds an index of saved searches
// Confirm Feed to Listbox ///// (costum-name on feed ti listbox)
constexpr bool confirmFeedToListbox = true;

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
constexpr bool confirmOverWrites = false;
// Recursive search in the subdirectories /////////////
constexpr bool recurseSubdirs = true;
// The searcher follows symlinks /////////////
constexpr bool followSymlinks = false;
// Files with similar size are compared by content /////////////
constexpr bool compareByContent = false;
// The date information is ignored at synchronization /////////////
constexpr bool ignoreDate = false;
// Asymmetric Client-File Server compare mode /////////////
constexpr bool asymmetric = false;
// Case insensitive compare in synchronizer /////////////
constexpr bool ignoreCase = false;
// Scrolls the results of the synchronization /////////////
constexpr bool scrollResults = false;
// The right arrow button is turned on /////////////
constexpr bool btnLeftToRight = true;
// The equals button is turned on /////////////
constexpr bool btnEquals = true;
// The not equals button is turned on /////////////
constexpr bool btnDifferents = true;
// The left arrow button is turned on /////////////
constexpr bool btnRightToLeft = true;
// The trash button is turned on /////////////
constexpr bool btnDeletable = true;
// The duplicates button is turned on /////////////
constexpr bool btnDuplicates = true;
// The singles button is turned on /////////////
constexpr bool btnSingles = true;

/////////////////////// [Custom Selection Mode]
// QT Selection
constexpr bool qtSelection = false;
// Left Selects
constexpr bool leftSelects = true;
// Left Preserves
constexpr bool leftPreserves = false;
// ShiftCtrl Left Selects
constexpr bool shiftCtrlLeft = false;
// Right Selects
constexpr bool rightSelects = true;
// Right Preserves
constexpr bool rightPreserves = false;
// ShiftCtrl Right Selects
constexpr bool shiftCtrlRight = false;
// Space Moves Down
constexpr bool spaceMovesDown = true;
// Space Calc Space
constexpr bool spaceCalcSpace = true;
// Insert Moves Down
constexpr bool insertMovesDown = true;
// Immediate Context Menu
constexpr bool immediateContextMenu = true;
// Reset selection items
constexpr bool resetSelectionItems = false;
} // namespace Defaults

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
