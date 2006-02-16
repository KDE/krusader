//
// C++ Implementation: expander
//
// Description: 
//
//
// Author: Jonas B�r (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "expander.h"

#include "../krusader.h"
#include "../krusaderview.h"
#include "../panelmanager.h"
#include "../Panel/listpanel.h"
#include "../Panel/panelfunc.h"
#include "../Panel/krview.h"
#include "../Synchronizer/synchronizergui.h"
#include "../Search/krsearchdialog.h"
#include "../GUI/profilemanager.h"
#include "../VFS/preservingcopyjob.h"
#include "../KViewer/krviewer.h"

#ifdef __KJSEMBED__
#include "../KrJS/krjs.h"
#endif

#include <kdebug.h>
#include <kinputdialog.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <qstringlist.h>
#include <qclipboard.h>

#define UA_CANCEL		"@CANCEL@"
#define NEED_PANEL		if ( panel == 0 ) { \
					   krOut << "Expander: no panel specified for %_" << _expression << "%; ignoring..." << endl; \
					   return UA_CANCEL; \
					}

////////////////////////////////////////////////////////////
//////////////////////// utils ////////////////////////
////////////////////////////////////////////////////////////

/**
 * escapes everything that confuses bash in filenames
 * @param s String to manipulate
 * @return escaped string
 */
QString bashquote( QString s ) {
    
    const QString evilstuff = "\\\"'`()[]{}!?;$&<>| ";		// stuff that should get escaped
     
    for ( unsigned int i = 0; i < evilstuff.length(); ++i )
        s.replace( evilstuff[ i ], ('\\' + evilstuff[ i ]) );

    return s;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// expander classes ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

exp_Path::exp_Path() {
   _expression = "Path";
   _description = i18n("panel's path");
   _needPanel = true;
   
   addParameter( new exp_parameter( i18n("Automatic escape spaces"), "__yes", false ) );
}
QString exp_Path::expFunc( const ListPanel* panel, const QStringList& parameter, const bool& useUrl ) {
   NEED_PANEL
   
   QString result;
   
   if ( useUrl )
      result = panel->func->files()->vfs_getOrigin().url() + "/";
   else
      result = panel->func->files()->vfs_getOrigin().path() + "/";
         
   if ( parameter[0].lower() == "no" )  // don't escape spaces
      return result;
   else
      return bashquote(result);
}

exp_Count::exp_Count() {
   _expression = "Count";
   _description = i18n("number of ...");
   _needPanel = true;
   
   addParameter( new exp_parameter( i18n("count all:"), "__choose:All;Files;Dirs;Selected", false ) );
}
QString exp_Count::expFunc( const ListPanel* panel, const QStringList& parameter, const bool& ) {
   NEED_PANEL
   
   int n = -1;
   if ( parameter[ 0 ].isEmpty() || parameter[ 0 ].lower() == "all" )
      n = panel->view->numDirs() + panel->view->numFiles();
   else if ( parameter[ 0 ].lower() == "files" )
      n = panel->view->numFiles();
   else if ( parameter[ 0 ].lower() == "dirs" )
      n = panel->view->numDirs();
   else if ( parameter[ 0 ].lower() == "selected" )
      n = panel->view->numSelected();
   else {
      krOut << "Expander: no Items specified for %_Count%; abort..." << endl;
      return UA_CANCEL;
   }

   return QString("%1").arg( n );
}

exp_Filter::exp_Filter() {
   _expression = "Filter";
   _description = i18n("filter mask: *.h, *.cpp, etc.");
   _needPanel = true;
}
QString exp_Filter::expFunc( const ListPanel* panel, const QStringList&, const bool& ) {
   NEED_PANEL

   return panel->view->filterMask().nameFilter();
}

exp_Current::exp_Current() {
   _expression = "Current";
   _description = i18n("current file (!= selected file)");
   _needPanel = true;

   addParameter( new exp_parameter( i18n("Omit the current path (optional)"), "__no", false ) );
   addParameter( new exp_parameter( i18n("Automatic escape spaces"), "__yes", false ) );
}
QString exp_Current::expFunc( const ListPanel* panel, const QStringList& parameter, const bool& useUrl ) {
   NEED_PANEL
   
   QString item = panel->view->getCurrentItem();

   QString result;
   
   if ( parameter[0].lower() == "yes" )  // ommit the current path
      result = item;
   else {
      KURL url = panel->func->files()->vfs_getFile( item );
      if ( useUrl )
         result = url.url();
      else
         result = url.path();
   }

   if ( parameter[1].lower() == "no" )  // don't escape spaces
      return result;
   else
      return bashquote(result);
}

exp_List::exp_List() {
   _expression = "List";
   _description = i18n("Item list of ...");
   _needPanel = true;

   addParameter( new exp_parameter( i18n("Which items"), "__choose:All;Files;Dirs;Selected", false ) );
   addParameter( new exp_parameter( i18n("Separator between the items (optional)"), " ", false ) );
   addParameter( new exp_parameter( i18n("Omit the current path (optional)"), "__no", false ) );
   addParameter( new exp_parameter( i18n("Mask (optional, all but 'Selected')"), "__select", false ) );
   addParameter( new exp_parameter( i18n("Automatic escape spaces"), "__yes", false ) );
}
QString exp_List::expFunc( const ListPanel* panel, const QStringList& parameter, const bool& useUrl ) {
   NEED_PANEL

   // get selected items from view
   QStringList items;
   QString mask;
   
   if ( parameter.count() <= 3 || parameter[3].isEmpty() )
      mask = "*";
   else
      mask = parameter[3];
   
   if ( parameter[ 0 ].isEmpty() || parameter[ 0 ].lower() == "all" )
      panel->view->getItemsByMask( mask, &items );
   else if ( parameter[ 0 ].lower() == "files" )
      panel->view->getItemsByMask( mask, &items, false, true );
   else if ( parameter[ 0 ].lower() == "dirs" )
      panel->view->getItemsByMask( mask, &items, true, false );
   else if ( parameter[ 0 ].lower() == "selected" )
      panel->view->getSelectedItems( &items );
   else {
      krOut << "Expander: no Items specified for %_List%; abort..." << endl;
      return UA_CANCEL;
   }

   QString separator;
   if ( parameter.count() <= 1 || parameter[1].isEmpty() )
      separator = " ";
   else
      separator = parameter[1];
   QString result;      
   if ( parameter[2].lower() == "yes" ) {  // ommit the current path
      for (QStringList::Iterator it = items.begin(); it != items.end(); ++it) {
         if ( ! result.isEmpty() )
            result += separator;
         if ( parameter[4].lower() == "no" )  // don't escape spaces
            result += *it;
         else
            result += bashquote(*it);
      }
   }
   else {
      // translate to urls using vfs
      KURL::List* list = panel->func->files()->vfs_getFiles(&items);
      // parse everything to a single qstring
      for (KURL::List::Iterator it = list->begin(); it != list->end(); ++it) {
         if ( ! result.isEmpty() )
            result += separator;
         if ( parameter[4].lower() == "no" )  // don't escape spaces
            result += ( (useUrl ? (*it).url() : (*it).path()) );
         else
            result += bashquote( (useUrl ? (*it).url() : (*it).path()) );
      }
   }

   return result;
}

exp_ListFile::exp_ListFile() {
   _expression = "ListFile";
   _description = i18n("Filename of an itemlist ...");
   _needPanel = true;

   addParameter( new exp_parameter( i18n("Which items"), "__choose:All;Files;Dirs;Selected", false ) );
   addParameter( new exp_parameter( i18n("Separator between the items (optional)"), "\n", false ) );
   addParameter( new exp_parameter( i18n("Omit the current path (optional)"), "__no", false ) );
   addParameter( new exp_parameter( i18n("Mask (optional, all but 'Selected')"), "__select", false ) );
   addParameter( new exp_parameter( i18n("Automatic escape spaces"), "__no", false ) );
}
QString exp_ListFile::expFunc( const ListPanel* panel, const QStringList& parameter, const bool& useUrl ) {
   NEED_PANEL

   // get selected items from view
   QStringList items;
   QString mask;
   
   if ( parameter.count() <= 3 || parameter[3].isEmpty() )
      mask = "*";
   else
      mask = parameter[3];
   
   if ( parameter[ 0 ].isEmpty() || parameter[ 0 ].lower() == "all" )
      panel->view->getItemsByMask( mask, &items );
   else if ( parameter[ 0 ].lower() == "files" )
      panel->view->getItemsByMask( mask, &items, false, true );
   else if ( parameter[ 0 ].lower() == "dirs" )
      panel->view->getItemsByMask( mask, &items, true, false );
   else if ( parameter[ 0 ].lower() == "selected" )
      panel->view->getSelectedItems( &items );
   else {
      krOut << "Expander: no Items specified for %_ListFile%; abort..." << endl;
      return UA_CANCEL;
   }

   QString separator;
   if ( parameter.count() <= 1 || parameter[1].isEmpty() )
      separator = "\n";
   else
      separator = parameter[1];
   QString result;      
   if ( parameter[2].lower() == "yes" ) {  // ommit the current path
      for (QStringList::Iterator it = items.begin(); it != items.end(); ++it) {
         if ( ! result.isEmpty() )
            result += separator;
         if ( parameter[4].lower() == "yes" )  // escape spaces
            result += bashquote(*it);
         else
            result += *it;
      }
   }
   else {
      // translate to urls using vfs
      KURL::List* list = panel->func->files()->vfs_getFiles(&items);
      // parse everything to a single qstring
      for (KURL::List::Iterator it = list->begin(); it != list->end(); ++it) {
         if ( ! result.isEmpty() )
            result += separator;
         if ( parameter[4].lower() == "yes" )  // escape spaces
            result += bashquote( (useUrl ? (*it).url() : (*it).path()) );
         else
            result += (useUrl ? (*it).url() : (*it).path());
      }
   }
   
   KTempFile tmpFile( locateLocal("tmp", "krusader"), ".itemlist" );
   
    if ( tmpFile.status() != 0 ) {
      krOut << "Expander: tempfile couldn't be opend (" << strerror( tmpFile.status() ) << "); abort..." << endl;
      return UA_CANCEL;
    }
    
    QTextStream stream( tmpFile.file() );
    stream << result << "\n";
    tmpFile.close();

   return tmpFile.name();
}

exp_Select::exp_Select() {
   _expression = "Select";
   _description = i18n("Manipulate the selection");
   _needPanel = true;

   addParameter( new exp_parameter( i18n("Selection mask"), "__select", true ) );
   addParameter( new exp_parameter( i18n("Manipulate in which way"), "__choose:Set;Add;Remove", false ) );
}
QString exp_Select::expFunc( const ListPanel* panel, const QStringList& parameter, const bool& ) {
   NEED_PANEL
   
   KRQuery mask;
    if ( parameter.count() <= 0 || parameter[0].isEmpty() )
       mask = KRQuery( "*" );
    else
       mask = KRQuery( parameter[0] );

    if ( parameter[1].lower() == "add")
       panel->view->select( mask );
    else if ( parameter[1].lower() == "remove")
       panel->view->unselect( mask );
    else { // parameter[1].lower() == "set" or isEmpty() or whatever
       panel->view->unselect( KRQuery( "*" ) );
       panel->view->select( mask );
    }

   return QString::null;  // this doesn't return everything, that's normal!
}

exp_Goto::exp_Goto() {
   _expression = "Goto";
   _description = i18n("Jump to a location");
   _needPanel = true;
   
   addParameter( new exp_parameter( i18n("please choose a path"), "__goto", true ) );
   addParameter( new exp_parameter( i18n("open the location in a new tab"), "__no", false ) );
}
QString exp_Goto::expFunc( const ListPanel* panel, const QStringList& parameter, const bool& ) {
   NEED_PANEL
   
   bool newTab = false;
   if ( parameter[1].lower() == "yes" )
      newTab = true;
   
   if ( newTab ) {
      if ( panel == LEFT_PANEL)
         krApp->mainView->leftMng->slotNewTab( parameter[0] );
      else
         krApp->mainView->rightMng->slotNewTab( parameter[0] );
   }
   else
      panel->func->openUrl( parameter[0], "" );
   
   return QString::null;  // this doesn't return everything, that's normal!
}

/*
exp_Search::exp_Search() {
   _expression = "Search";
   _description = i18n("Search for files");
   _needPanel = true;

   addParameter( new exp_parameter( i18n("please choose the setting"), "__searchprofile", true ) );
   addParameter( new exp_parameter( i18n("open the search in a new tab"), "__yes", false ) );  //TODO: add this also to panel-dependent as soon as vfs support the display of search-results
}
*/

exp_Ask::exp_Ask() {
   _expression = "Ask";
   _description = i18n("Ask the user for a parameter");
   _needPanel = false;

   addParameter( new exp_parameter( i18n("Question"), "Where do you want do go today?", true ) );
   addParameter( new exp_parameter( i18n("Preset (optional)"), "", false ) );
   addParameter( new exp_parameter( i18n("Caption (optional)"), "", false ) );
}
QString exp_Ask::expFunc( const ListPanel*, const QStringList& parameter, const bool& ) {
   QString caption, preset, result;
   
   if ( parameter.count() <= 2 || parameter[2].isEmpty() )
      caption = i18n("User Action");
   else
      caption = parameter[2];
   if ( parameter.count() <= 1 || parameter[1].isEmpty() )
      preset = QString::null;
   else
      preset = parameter[1];
   
   bool ok;
   result = KInputDialog::getText(
		caption,
		parameter[0],
		preset,
		&ok );
   
   if (ok)
      return result;
   else
      return UA_CANCEL;
}

exp_Clipboard::exp_Clipboard() {
   _expression = "Clipboard";
   _description = i18n("Copy to clipboard");
   _needPanel = false;

   addParameter( new exp_parameter( i18n("What should be copied"), "__placeholder", true ) );
   addParameter( new exp_parameter( i18n("Append to the current clipboard-content with this separator (optional)"), "", false ) );
}
QString exp_Clipboard::expFunc( const ListPanel*, const QStringList& parameter, const bool& ) {
//    kdDebug() << "Expander::exp_Clipboard, parameter[0]: '" << parameter[0] << "', Clipboard: " << KApplication::clipboard()->text() << endl;
    if ( parameter.count() <= 1 || parameter[1].isEmpty() || KApplication::clipboard()->text().isEmpty() )
       KApplication::clipboard()->setText( parameter[0] );
    else
       KApplication::clipboard()->setText( KApplication::clipboard()->text() + parameter[1] + parameter[0] );

   return QString::null;  // this doesn't return everything, that's normal!
}

exp_Copy::exp_Copy() {
   _expression = "Copy";
   _description = i18n("Copy a file/folder");
   _needPanel = false;

   addParameter( new exp_parameter( i18n("What should be copied"), "__placeholder", true ) );
   addParameter( new exp_parameter( i18n("Where it should be copied"), "__placeholder", true ) );
}                    
QString exp_Copy::expFunc( const ListPanel*, const QStringList& parameter, const bool& ) {

   // basicly the parameter can already be used as URL, but since KURL has problems with ftp-proxy-urls (like ftp://username@proxyusername@url...) this is nessesary:
   KURL src = vfs::fromPathOrURL( parameter[0] );
   KURL dest = vfs::fromPathOrURL( parameter[1] );
   
   if ( !dest.isValid() || !src.isValid() ) {
      krOut << "Expander: invalid URL's in %_Copy(\"src\", \"dest\")%" << endl;
      return UA_CANCEL; // do nothing with invalid url's
   }

   PreservingCopyJob::createCopyJob( PM_DEFAULT, src, dest, KIO::CopyJob::Copy, false, true );

   return QString::null;  // this doesn't return everything, that's normal!
}

exp_Move::exp_Move() {
   _expression = "Move";
   _description = i18n("Move/Rename a file/folder");
   _needPanel = false;

   addParameter( new exp_parameter( i18n("What moved/renamed"), "__placeholder", true ) );
   addParameter( new exp_parameter( i18n("New target/name"), "__placeholder", true ) );
}                    
QString exp_Move::expFunc( const ListPanel*, const QStringList& parameter, const bool& ) {

   // basicly the parameter can already be used as URL, but since KURL has problems with ftp-proxy-urls (like ftp://username@proxyusername@url...) this is nessesary:
   KURL src = vfs::fromPathOrURL( parameter[0] );
   KURL dest = vfs::fromPathOrURL( parameter[1] );
   
   if ( !dest.isValid() || !src.isValid() ) {
      krOut << "Expander: invalid URL's in %_Move(\"src\", \"dest\")%" << endl;
      return UA_CANCEL; // do nothing with invalid url's
   }

   PreservingCopyJob::createCopyJob( PM_DEFAULT, src, dest, KIO::CopyJob::Move, false, true );

   return QString::null;  // this doesn't return everything, that's normal!
}

exp_Sync::exp_Sync() {
   _expression = "Sync";
   _description = i18n("Open a synchronizer-profile");
   _needPanel = false;
   
   addParameter( new exp_parameter( i18n("Choose a profile"), "__syncprofile", true ) );
}
QString exp_Sync::expFunc( const ListPanel*, const QStringList& parameter, const bool& ) {
   if ( parameter[0].isEmpty() ) {
      krOut << "Expander: no profile specified for %_Sync(profile)%; abort..." << endl;
      return UA_CANCEL;
   }

   new SynchronizerGUI( 0, parameter[0] );

   return QString::null;  // this doesn't return everything, that's normal!
}

exp_NewSearch::exp_NewSearch() {
   _expression = "NewSearch";
   _description = i18n("Open a searchmodule-profile");
   _needPanel = false;
   
   addParameter( new exp_parameter( i18n("Choose a profile"), "__searchprofile", true ) );
}
QString exp_NewSearch::expFunc( const ListPanel*, const QStringList& parameter, const bool& ) {
   if ( parameter[0].isEmpty() ) {
      krOut << "Expander: no profile specified for %_NewSearch(profile)%; abort..." << endl;
      return UA_CANCEL;
   }

   new KrSearchDialog( parameter[0] );

   return QString::null;  // this doesn't return everything, that's normal!
}

exp_Profile::exp_Profile() {
   _expression = "Profile";
   _description = i18n("Load a panel-profile");
   _needPanel = false;
   
   addParameter( new exp_parameter( i18n("Choose a profile"), "__panelprofile", true ) );
}
QString exp_Profile::expFunc( const ListPanel*, const QStringList& parameter, const bool& ) {
   if ( parameter[0].isEmpty() ) {
      krOut << "Expander: no profile specified for %_Profile(profile)%; abort..." << endl;
      return UA_CANCEL;
   }
   
   MAIN_VIEW->profiles( parameter[0] );

   return QString::null;  // this doesn't return everything, that's normal!
}

exp_Each::exp_Each() {
   _expression = "Each";
   _description = i18n("Separate programm call for each...");
   _needPanel = true;

   addParameter( new exp_parameter( i18n("Which items"), "__choose:All;Files;Dirs;Selected", false ) );
   addParameter( new exp_parameter( i18n("Omit the current path (optional)"), "__no", false ) );
   addParameter( new exp_parameter( i18n("Mask (optional, all but 'Selected')"), "__select", false ) );
   addParameter( new exp_parameter( i18n("Automatic escape spaces"), "__yes", false ) );
}
QString exp_Each::expFunc( const ListPanel* panel, const QStringList& parameter, const bool& ) {
   NEED_PANEL
   
   QString mark = "@EACH";	//this mark is later used to splitt the commandline
   
   if ( panel == ACTIVE_PANEL )
      mark += "a";
   else if ( panel == OTHER_PANEL )
      mark += "o";
   else if ( panel == LEFT_PANEL )
      mark += "l";
   else if ( panel == RIGHT_PANEL )
      mark += "r";
   
   if ( parameter[ 0 ].isEmpty() || parameter[ 0 ].lower() == "all" )
      mark += "a";
   else if ( parameter[ 0 ].lower() == "files" )
      mark += "f";
   else if ( parameter[ 0 ].lower() == "dirs" )
      mark += "d";
   else if ( parameter[ 0 ].lower() == "selected" )
      mark += "s";
   else {
      krOut << "Expander: no Items specified for %_Each%; abort..." << endl;
      return UA_CANCEL;
   }

   if ( parameter[1].lower() == "yes" )  // ommit the current path
      mark += "y";
   else
      mark += "n";

   QString mask;
   if ( parameter.count() <= 2 || parameter[2].isEmpty() )
      mask = "*";
   else
      mask = parameter[2];
   
   if ( parameter[3].lower() == "no" )  // don't escape spaces
      mark += "n";
   else
      mark += "y";
   
   /*
    * NOTE: This expFunc doesn't the work itself!!
    * it sets only a mark where the commandline is splitt after _all_ placeholders are replaced!
    */
   return mark + mask + "@";
}

exp_ColSort::exp_ColSort() {
   _expression = "ColSort";
   _description = i18n("Set the sorting for this panel");
   _needPanel = true;
   
   addParameter( new exp_parameter( i18n("Choose a column"), "__choose:Name;Ext;Type;Size;Modified;Perms;rwx;Owner;Group", true ) );
   addParameter( new exp_parameter( i18n("Choose the sort sequence"), "__choose:Toggle;Asc;Desc", false ) );
}
QString exp_ColSort::expFunc( const ListPanel* panel, const QStringList& parameter, const bool& ) {
   NEED_PANEL

   if ( parameter[0].isEmpty() ) {
      krOut << "Expander: no column specified for %_ColSort(column)%; abort..." << endl;
      return UA_CANCEL;
   }
   
   int mode = (int) panel->view->sortMode();
   
   /* from Panel/krview.h:
      enum SortSpec { Name=0x1, 
                              Ext=0x2,
                              Size=0x4,
                              Type=0x8,
                              Modified=0x10,
                              Permissions=0x20,
                              KrPermissions=0x40,
                              Owner=0x80,
                              Group=0x100,
                              Descending=0x200,
                              DirsFirst=0x400,
                              IgnoreCase=0x800 };
   */
   
   krOut << "start: exp_ColSort::expFunc" << endl;
   #define MODE_OUT krOut << QString( "mode: %1" ).arg( mode, 0, 2 ) << endl; // displays mode in base-2
   MODE_OUT
      
   if ( parameter.count() <= 1 || ( parameter[1].lower() != "asc" && parameter[1].lower() != "desc" ) ) {  //default == toggle
      if ( mode & KrViewProperties::Descending )
         mode &= ~KrViewProperties::Descending; // == asc
      else
         mode |= KrViewProperties::Descending; // == desc
   } else
   if ( parameter[1].lower() == "asc" ) {
      mode &= ~KrViewProperties::Descending;
   }
   else { // == desc
      mode |= KrViewProperties::Descending;
   }
   
   MODE_OUT
   
   // clear all column-infromation:
   mode &= ~( KrViewProperties::Name | KrViewProperties::Ext | KrViewProperties::Size | KrViewProperties::Type | KrViewProperties::Modified | KrViewProperties::Permissions | KrViewProperties::KrPermissions | KrViewProperties::Owner | KrViewProperties::Group );
   
   MODE_OUT
   
   if ( parameter[0].lower() == "name" ) {
      mode |= KrViewProperties::Name;
   } else
   if ( parameter[0].lower() == "ext" ) {
      mode |= KrViewProperties::Ext;
   } else
   if ( parameter[0].lower() == "type" ) {
      mode |= KrViewProperties::Type;
   } else
   if ( parameter[0].lower() == "size" ) {
      mode |= KrViewProperties::Size;
   } else
   if ( parameter[0].lower() == "modified" ) {
      mode |= KrViewProperties::Modified;
   } else
   if ( parameter[0].lower() == "perms" ) {
      mode |= KrViewProperties::Permissions;
   } else
   if ( parameter[0].lower() == "rwx" ) {
      mode |= KrViewProperties::KrPermissions;
   } else
   if ( parameter[0].lower() == "owner" ) {
      mode |= KrViewProperties::Owner;
   } else
   if ( parameter[0].lower() == "group" ) {
      mode |= KrViewProperties::Group;
   } else {
      krOut << "Expander: unknown column specified for %_ColSort(column)%; ignoring..." << endl;
      return QString::null;
   }
   
   MODE_OUT
   panel->view->setSortMode( (KrViewProperties::SortSpec)mode );
   krOut << "end: exp_ColSort::expFunc" << endl;
   return QString::null;  // this doesn't return everything, that's normal!
}

exp_PanelSize::exp_PanelSize() {
   _expression = "PanelSize";
   _description = i18n("Set the relation between the two panels");
   _needPanel = true;
   
   addParameter( new exp_parameter( i18n("Set the new size in percent"), "__int:0;100;5;50", true ) );
}
QString exp_PanelSize::expFunc( const ListPanel* panel, const QStringList& parameter, const bool& ) {
   NEED_PANEL
   int newSize;
   
   if ( parameter[0].isEmpty() )
      newSize = 50;	//default is 50%
   else
      newSize = parameter[0].toInt();
   
   if ( newSize < 0 || newSize > 100 ) {
      krOut << "Expander: Value out of range for %_PanelSize(percent)%. The first parameter has to be >0 and <100" << endl;
      return UA_CANCEL;
   }

    QValueList<int> panelSizes = MAIN_VIEW->horiz_splitter->sizes();
    int totalSize = panelSizes[0] + panelSizes[1];
    
    if ( panel == LEFT_PANEL ) {
       panelSizes[0] = totalSize * newSize / 100;
       panelSizes[1] = totalSize * (100 - newSize) / 100;
    }
    else { // == RIGHT_PANEL
       panelSizes[0] = totalSize * (100 - newSize) / 100;
       panelSizes[1] = totalSize * newSize / 100;
    }
    
    MAIN_VIEW->horiz_splitter->setSizes( panelSizes );

   return QString::null;  // this doesn't return everything, that's normal!
}

#ifdef __KJSEMBED__
exp_Script::exp_Script() {
   _expression = "Script";
   _description = i18n("Executes a JavaScript-extension");
   _needPanel = false;

   addParameter( new exp_parameter( i18n("Location of the script"), "", true ) );
   addParameter( new exp_parameter( i18n("Set some variables for the execution (optional).\ni.e. \"return=cmd;foo=bar\", consult the handbook for more information"), "", false ) );
}
QString exp_Script::expFunc( const ListPanel*, const QStringList& parameter, const bool& ) {
   if ( parameter[0].isEmpty() ) {
      krOut << "Expander: no script specified for %_Script(script)%; abort..." << endl;
      return UA_CANCEL;
   }

   QString filename = parameter[0];
   if ( filename.find('/') && KURL::isRelativeURL(filename) ) {
      // this return the local version of the file if this exists. else the global one is returnd
      filename = locate( "data", "krusader/js/"+filename );
   }

  if ( ! krJS )
      krJS = new KrJS();

   KJS::ExecState *exec = krJS->globalExec();

   QString jsReturn = QString::null;
   if ( parameter[1].lower() == "yes" )	// to stay compatible with the old-style parameter
      jsReturn = "cmd";
   else {
      QStringList jsVariables = QStringList::split( ';', parameter[1] );
      QString jsVariable, jsValue;
      for ( QStringList::Iterator it = jsVariables.begin(); it != jsVariables.end(); ++it ) {
         jsVariable = (*it).section('=', 0, 0).stripWhiteSpace();
         jsValue = (*it).section('=', 1);
         if ( jsVariable == "return" )
            jsReturn = jsValue.stripWhiteSpace();
         else
            krJS->putValue( jsVariable, KJSEmbed::convertToValue(exec, jsValue ) );
      }
   }

   krJS->runFile( filename );

   if ( ! jsReturn.isEmpty() )
      return krJS->getValue( jsReturn ).toString( krJS->globalExec() ).qstring();
   else
      return QString::null;
}
#endif

exp_View::exp_View() {
   _expression = "View";
   _description = i18n("View a file with Krusader's internal viewer");
   _needPanel = false;

   addParameter( new exp_parameter( i18n("Which file to view (normally '%aCurrent%')"), "__placeholder", true ) );
   addParameter( new exp_parameter( i18n("Choose a view-mode"), "__choose:generic;text;hex", false ) );
   //addParameter( new exp_parameter( i18n("Choose a window-mode"), "__choose:tab;window;panel", false ) );
   //TODO: window-mode 'panel' should open the file in the third-hand viewer
   addParameter( new exp_parameter( i18n("Choose a window-mode"), "__choose:tab;window", false ) );
}
QString exp_View::expFunc( const ListPanel*, const QStringList& parameter, const bool& ) {
   if ( parameter[0].isEmpty() ) {
      krOut << "Expander: no file to view in %_View(filename)%; abort..." << endl;
      return UA_CANCEL;
   }

   QString viewMode, windowMode;
   if ( parameter.count() <= 1 || parameter[1].isEmpty() )
      viewMode = "generic";
   else
      viewMode = parameter[1];

   if ( parameter.count() <= 2 || parameter[2].isEmpty() )
      windowMode = "tab";
   else
      windowMode = parameter[2];

   KrViewer::Mode mode = KrViewer::Generic;
   if( viewMode == "text" ) mode = KrViewer::Text;
   else if( viewMode == "hex" ) mode = KrViewer::Hex;

	KrViewer::view(parameter[0],mode,(windowMode == "window"));
   //TODO: Call the viewer with viewMode and windowMode. Filename is in parameter[0].
   // It would be nice if parameter[0] could also be a space-separated filename-list (provided if the first parameter is %aList(selected)%)

   return QString::null;  // this doesn't return everything, that's normal!
}

/////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// end of expander classes ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

Expander::Expander() {
   //Penel-dependent:
   addPlaceholder( new exp_Path() );
   addPlaceholder( new exp_Count() );
   addPlaceholder( new exp_Filter() );
   addPlaceholder( new exp_Current() );
   addPlaceholder( new exp_List() );
   addPlaceholder( new exp_ListFile() );
   addPlaceholder( new exp_Each() );
   addPlaceholder( new exp_separator( true ) );
   addPlaceholder( new exp_Select() );
   addPlaceholder( new exp_Goto() );
   addPlaceholder( new exp_ColSort() );
   addPlaceholder( new exp_PanelSize() );
//    addPlaceholder( new exp_Search() );
   //Panel-independent:
   addPlaceholder( new exp_Ask() );
   addPlaceholder( new exp_Clipboard() );
   addPlaceholder( new exp_Copy() );
   addPlaceholder( new exp_Move() );
   addPlaceholder( new exp_Sync() );
   addPlaceholder( new exp_NewSearch() );
   addPlaceholder( new exp_Profile() );
   #ifdef __KJSEMBED__
   addPlaceholder( new exp_Script() );
   #endif
   addPlaceholder( new exp_View() );
//    addPlaceholder( new exp_Run() );
}

Expander::~Expander() {
   for ( int i = 0; i < placeholderCount(); ++i )
      delete placeholder( i );
}

ListPanel* Expander::getPanel( const char& panelIndicator ) {
   switch ( panelIndicator ) {
   case 'a':
      return ACTIVE_PANEL;
   case 'o':
      return OTHER_PANEL;
   case 'l':
      return LEFT_PANEL;
   case 'r':
      return RIGHT_PANEL;
   case '_':
      return 0;
   default:
      krOut << "Expander: unknown Panel " << panelIndicator << endl;
      return 0;
   }
}

QStringList Expander::expand( const QString& stringToExpand, bool useUrl ) {
   QStringList resultList;
   _ua_cancel = false;
   
   QString result = expandCurrent( stringToExpand, useUrl );
   if ( _ua_cancel )
      return QString::null;
   
   if ( result.contains("@EACH") )
      resultList = splitEach( result, useUrl );
   else
      resultList.append( result );

   return resultList;
}

QString Expander::expandCurrent( const QString& stringToExpand, bool useUrl ) {
   QString result = QString::null, exp = QString::null;
   QString tmpResult;
   int begin, end, i;
//    int brackets = 0;
//    bool inQuotes = false;
   unsigned int idx = 0;
   while ( idx < stringToExpand.length() ) {
      if ( ( begin = stringToExpand.find( '%', idx ) ) == -1 ) break;
      if ( ( end = findEnd( stringToExpand, begin ) ) == -1 ) {
         krOut << "Error: unterminated % in Expander::expandCurrent" << endl;
         return QString::null;
      }

      result += stringToExpand.mid( idx, begin - idx ); // copy until the start of %exp%

      // get the expression, and expand it using the correct expander function
      exp = stringToExpand.mid( begin + 1, end - begin - 1 );
//       kdDebug() << "------------- exp: '" << exp << "'" << endl;
      if ( exp == "" )
        result += "%";
      else {
        QStringList parameter = separateParameter( &exp, useUrl );
        if ( _ua_cancel )
           return QString::null;
        char panelIndicator = exp.lower()[0].latin1();
        exp.replace( 0, 1, "" );
        for ( i = 0; i < placeholderCount(); ++i )
           if ( exp == placeholder( i )->expression() ) {
//               kdDebug() << "---------------------------------------" << endl;
              tmpResult = placeholder( i )->expFunc( getPanel( panelIndicator ), parameter, useUrl );
              if ( tmpResult == UA_CANCEL ) {
                 _ua_cancel = true;
                 return QString::null;
              }
              else
                 result += tmpResult;
//               kdDebug() << "---------------------------------------" << endl;
              break;
           }
        if ( i == placeholderCount() ) { // didn't find an expander
           krOut << "Error: unrecognized %" << panelIndicator << exp << "% in Expander::expand" << endl;
           return QString::null;
        }
      } //else
      idx = end + 1;
   }
   // copy the rest of the string
   result += stringToExpand.mid( idx );
//    kdDebug() << "============== result '" << result << "'" << endl;
   return result;
}

QStringList Expander::splitEach( const QString& stringToSplit, bool useUrl ) {
//    kdDebug() << "stringToSplit: " << stringToSplit << endl;
   QStringList chunks;
   unsigned int idx = 0;
   int begin, end;

   // find all @EACH and put thier "parameter" in the array 'chunks'
   while (idx < stringToSplit.length()) {
      if ( ( begin = stringToSplit.find( "@EACH", idx ) ) == -1 ) break;
      begin += 5; // = lenth ("@EACH")
      end = stringToSplit.find( "@", begin );
      chunks.append( stringToSplit.mid(begin, end - begin) );
      
      idx = end + 1;
   }
//    kdDebug() << "chunks: " << chunks << endl;

   typedef QValueList <QStringList> ItemList;
   ItemList itemList;
   QStringList items;
   
   // get the items for each chunk
   for ( QStringList::iterator it = chunks.begin(); it != chunks.end(); ++it ) {
      items.clear();
//       kdDebug() << "current chunk: " << *it << endl;
      QString mask = (*it).mid(4);
//       kdDebug() << "current mask: " << mask << endl;
      QChar panelIndicator = (*it)[0];
//       kdDebug() << "current panel: " << panelIndicator << endl;
      if ( (*it)[1] == 'a' ) // get all
         getPanel( panelIndicator )->view->getItemsByMask( mask, &items );
      else if ( (*it)[1] == 'f' ) // get files
         getPanel( panelIndicator )->view->getItemsByMask( mask, &items, false, true );
      else if ( (*it)[1] == 'd' ) // get dirs
         getPanel( panelIndicator )->view->getItemsByMask( mask, &items, true, false );
      else if ( (*it)[1] == 's' ) // get selected
         getPanel( panelIndicator )->view->getSelectedItems( &items );

      itemList.append( items );
   }

   unsigned int maxCount = 0;
   
   // check if we got for each chunk a replacement: find the smalles item-count
   for ( ItemList::iterator it = itemList.begin(); it != itemList.end(); ++it ) {
      if ( maxCount == 0 || (*it).count() < maxCount )
         maxCount = (*it).count();
   }
   
   QStringList result;
   QString tmp, replacement;
   
   // replace the chunks with thier items
   for ( unsigned int i = 0; i < maxCount; ++i ) {
      tmp = stringToSplit;
      for ( unsigned int c = 0; c < chunks.count(); ++c) {
         if ( chunks[c][2] == 'y' ) { // ommit path
            if (chunks[c][3] == 'n') // don't escape spaces
               replacement = itemList[c][i];
            else
               replacement = bashquote( itemList[c][i] );
         }
         else {
            KURL url = getPanel( (QChar) chunks[c][0] )->func->files()->vfs_getFile( itemList[c][i] );
            if (chunks[c][3] == 'n') // don't escape spaces
               replacement = (useUrl ? url.url() : url.path());
            else
               replacement = bashquote( (useUrl ? url.url() : url.path()) );
         }
         tmp.replace( "@EACH" + chunks[c] + "@", replacement );
      } //for
      result.append( tmp );
   } //for

   return result;
}

QStringList Expander::separateParameter( QString* exp, bool useUrl ) {
   QStringList parameter;
   QString result;
   int begin, end;
   if ( ( begin = exp->find( '(' ) ) != -1 ) {
      if ( ( end = exp->findRev( ')' ) ) == -1 ) {
         krOut << "Error: missing ')' in Expander::separateParameter" << endl;
         return QString::null;
      }
      result = exp->mid( begin + 1, end - begin - 1 );
      *exp = exp->left( begin );
   
      bool inQuotes = false;
      unsigned int idx = 0;
      begin = 0;
      while ( idx < result.length() ) {
         if ( result[ idx ].latin1() == '\\' ) {
            if ( result[ idx+1 ].latin1() == '"')
               result.replace( idx, 1, "" );
//             idx++;
         }
         if ( result[ idx ].latin1() == '"' )
            inQuotes = !inQuotes;
         if ( result[ idx ].latin1() == ',' && !inQuotes ) {
            parameter.append( result.mid( begin, idx - begin) );
            begin = idx + 1;
//             krOut << " ---- parameter: " << parameter.join(";") << endl;
         }
         idx++;
      }
      parameter.append( result.mid( begin, idx - begin) );  //don't forget the last one
      
      for (QStringList::Iterator it = parameter.begin(); it != parameter.end(); ++it) {
         *it = (*it).stripWhiteSpace();
         if ( (*it).left(1) == "\"" )
            *it = (*it).mid(1, (*it).length() - 2 );
         *it = expandCurrent( *it, useUrl );
         if ( _ua_cancel )
            return QString::null;
      }
   }
      
//    krOut << "------- exp: " << *exp << " ---- parameter: " << parameter.join(";") << endl;
   return parameter;
}

int Expander::findEnd( const QString& str, int start ) {
   int end = str.find( '%', start + 1 );
   if ( end == -1 )
      return end;
   int bracket = str.find( '(', start + 1 );
   if ( end < bracket || bracket == -1 )
      return end;
      
   unsigned int idx = bracket;
   bool inQuotes = false;
   
   while ( idx < str.length() ) {
      if ( str[ idx ].latin1() == '\\' )
         idx += 2;
      if ( str[ idx ].latin1() == '"' )
         inQuotes = !inQuotes;
      if ( str[ idx ].latin1() == ')' && !inQuotes )
         return ++idx;
      idx++;
   }
	// failsafe
	return end;
}
