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
#include "../Synchronizer/synchronizergui.h"
#include "../Search/krsearchdialog.h"
#include "../GUI/profilemanager.h"

#include <kdebug.h>
#include <kinputdialog.h>
#include <qstringlist.h>
#include <qclipboard.h>

#define NEED_PANEL		if ( panel == 0 ) { \
					   kdWarning() << "Expander: no panel specified for %_" << _expression << "%; ignoring..." << endl; \
					   return QString::null; \
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
QString exp_Path::expFunc( const ListPanel* panel, const QStringList& parameter, const bool& useUrl, const int& ) {
   NEED_PANEL
   
   QString result;
   
   if ( useUrl )
      result = panel->func->files()->vfs_getOrigin().url() + "/";
   else
      result = panel->func->files()->vfs_getOrigin().path() + "/";
         
   if ( parameter[0].lower() == "no" )  // don't escape spaces
      return result;
   else
      return result.replace(" ", "\\ ");
}

exp_Count::exp_Count() {
   _expression = "Count";
   _description = i18n("number of ...");
   _needPanel = true;
   
   addParameter( new exp_parameter( i18n("count all:"), "__choose:All;Files;Dirs;Selected", false ) );
}
QString exp_Count::expFunc( const ListPanel* panel, const QStringList& parameter, const bool&, const int& ) {
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
      kdWarning() << "Expander: no Items specified for %_Count%; ignoring..." << endl;
      return QString::null;
   }

   return QString("%1").arg( n );
}

exp_Filter::exp_Filter() {
   _expression = "Filter";
   _description = i18n("filter mask (*.h, *.cpp ...)");
   _needPanel = true;
}
QString exp_Filter::expFunc( const ListPanel* panel, const QStringList&, const bool&, const int& ) {
   NEED_PANEL
   
   return panel->view->filterMask();
}

exp_Current::exp_Current() {
   _expression = "Current";
   _description = i18n("current file (!= selected file)");
   _needPanel = true;
   
   addParameter( new exp_parameter( i18n("Ommit the current path (optional)"), "__no", false ) );
   addParameter( new exp_parameter( i18n("Automatic escape spaces"), "__yes", false ) );
}
QString exp_Current::expFunc( const ListPanel* panel, const QStringList& parameter, const bool& useUrl, const int& currentItem ) {
   NEED_PANEL
   
   QString item;
   if ( currentItem >= 0) {  // in a callEach-cycle
      if ( panel == ACTIVE_PANEL ) {  // callEach works only in the active panel
         QStringList items;
         panel->view->getSelectedItems( &items );
         item = items[currentItem];
      }
      else
         item = panel->view->getCurrentItem();
   }
   else {
      item = panel->view->getCurrentItem();
   }
   
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
      return result.replace(" ", "\\ ");
}

exp_List::exp_List() {
   _expression = "List";
   _description = i18n("Item list of ...");
   _needPanel = true;

   addParameter( new exp_parameter( i18n("Which items"), "__choose:All;Files;Dirs;Selected", false ) );
   addParameter( new exp_parameter( i18n("Separator between the items (optional)"), " ", false ) );
   addParameter( new exp_parameter( i18n("Ommit the current path (optional)"), "__no", false ) );
   addParameter( new exp_parameter( i18n("Mask (optional, all but 'Selected')"), "__select", false ) );
   addParameter( new exp_parameter( i18n("Automatic escape spaces"), "__yes", false ) );
}
QString exp_List::expFunc( const ListPanel* panel, const QStringList& parameter, const bool& useUrl, const int& ) {
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
      kdWarning() << "Expander: no Items specified for %_List%; ignoring..." << endl;
      return QString::null;
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
            result += (*it).replace(" ", "\\ ");
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
            result += ( (useUrl ? (*it).url() : (*it).path()) ).replace(" ", "\\ ");
      }
   }

   return result;
}

exp_Select::exp_Select() {
   _expression = "Select";
   _description = i18n("Manipulate the selection");
   _needPanel = true;

   addParameter( new exp_parameter( i18n("Selectionmask"), "__select", true ) );
   addParameter( new exp_parameter( i18n("Manipulate in which way"), "__choose:Set;Add;Remove", false ) );
}
QString exp_Select::expFunc( const ListPanel* panel, const QStringList& parameter, const bool&, const int& ) {
   NEED_PANEL
   
   QString mask;
    if ( parameter.count() <= 0 || parameter[0].isEmpty() )
       mask = "*";
    else
       mask = parameter[0];

    if ( parameter[1].lower() == "add")
       panel->view->select( mask );
    else if ( parameter[1].lower() == "remove")
       panel->view->unselect( mask );
    else { // parameter[1].lower() == "set" or isEmpty() or whatever
       panel->view->unselect( "*" );
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
QString exp_Goto::expFunc( const ListPanel* panel, const QStringList& parameter, const bool&, const int& ) {
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
   _description = i18n("Question the user for a parameter");
   _needPanel = false;

   addParameter( new exp_parameter( i18n("Question"), "Where do you want do go today?", true ) );
   addParameter( new exp_parameter( i18n("Preset (optional)"), "", false ) );
   addParameter( new exp_parameter( i18n("Caption (optional)"), "", false ) );
}
QString exp_Ask::expFunc( const ListPanel*, const QStringList& parameter, const bool&, const int& ) {
   QString caption, preset;
   if ( parameter.count() <= 2 || parameter[2].isEmpty() )
      caption = i18n("User Action");
   else
      caption = parameter[2];
   if ( parameter.count() <= 1 || parameter[1].isEmpty() )
      preset = QString::null;
   else
      preset = parameter[1];
   return KInputDialog::getText(
		caption,
		parameter[0],
		preset );
}

exp_Clipboard::exp_Clipboard() {
   _expression = "Clipboard";
   _description = i18n("Copy to clipboard");
   _needPanel = false;

   addParameter( new exp_parameter( i18n("What should be copied"), "__placeholder", true ) );
   addParameter( new exp_parameter( i18n("Append to the current clipboard-content with this seperator (optional)"), "", false ) );
}
QString exp_Clipboard::expFunc( const ListPanel*, const QStringList& parameter, const bool&, const int& ) {
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
QString exp_Copy::expFunc( const ListPanel*, const QStringList& parameter, const bool&, const int& ) {

   // basicly the parameter can already be used as URL, but since KURL has problems with ftp-proxy-urls (like ftp://username@proxyusername@url...) this is nessesary:
   KURL src = vfs::fromPathOrURL( parameter[0] );
   KURL dest = vfs::fromPathOrURL( parameter[1] );
   
   if ( !dest.isValid() || !src.isValid() ) {
      kdWarning() << "Expander: invalid URL's in %_Copy(\"src\", \"dest\")%" << endl;
      return QString::null; // do nothing with invalid url's
   }

   new KIO::CopyJob( src, dest, KIO::CopyJob::Copy, false, true );

   return QString::null;  // this doesn't return everything, that's normal!
}

exp_Sync::exp_Sync() {
   _expression = "Sync";
   _description = i18n("Open a synchronizer-profile");
   _needPanel = false;
   
   addParameter( new exp_parameter( i18n("Choose a profile"), "__syncprofile", true ) );
}
QString exp_Sync::expFunc( const ListPanel*, const QStringList& parameter, const bool&, const int& ) {
   if ( parameter[0].isEmpty() ) {
      kdWarning() << "Expander: no profile specified for %_Sync(profile)%; ignoring..." << endl;
      return QString::null;
   }

   new SynchronizerGUI( MAIN_VIEW, parameter[0] );

   return QString::null;  // this doesn't return everything, that's normal!
}

exp_NewSearch::exp_NewSearch() {
   _expression = "NewSearch";
   _description = i18n("Open a searchmodule-profile");
   _needPanel = false;
   
   addParameter( new exp_parameter( i18n("Choose a profile"), "__searchprofile", true ) );
}
QString exp_NewSearch::expFunc( const ListPanel*, const QStringList& parameter, const bool&, const int& ) {
   if ( parameter[0].isEmpty() ) {
      kdWarning() << "Expander: no profile specified for %_NewSearch(profile)%; ignoring..." << endl;
      return QString::null;
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
QString exp_Profile::expFunc( const ListPanel*, const QStringList& parameter, const bool&, const int& ) {
   if ( parameter[0].isEmpty() ) {
      kdWarning() << "Expander: no profile specified for %_Profile(profile)%; ignoring..." << endl;
      return QString::null;
   }
   
   MAIN_VIEW->profiles( parameter[0] );

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
   addPlaceholder( new exp_separator( true ) );
   addPlaceholder( new exp_Select() );
   addPlaceholder( new exp_Goto() );
//    addPlaceholder( new exp_Search() );
   //Panel-independent:
   addPlaceholder( new exp_Ask() );
   addPlaceholder( new exp_Clipboard() );
   addPlaceholder( new exp_Copy() );
   addPlaceholder( new exp_Sync() );
   addPlaceholder( new exp_NewSearch() );
   addPlaceholder( new exp_Profile() );
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
      kdWarning() << "Expander: unknown Panel " << panelIndicator << endl;
      return 0;
   }
}

QStringList Expander::expand( const QString& stringToExpand, bool useUrl, bool callEach ) {
   QStringList result;
   if ( callEach ) {
      QStringList items;
      ACTIVE_PANEL->view->getSelectedItems( &items );
      for ( unsigned int currentItem = 0; currentItem < items.count(); ++currentItem )
         result.append( expandCurrent( stringToExpand, useUrl, currentItem ) );
   }
   else
      result.append( expandCurrent( stringToExpand, useUrl, -1 ) );

   return result;
}

QString Expander::expandCurrent( const QString& stringToExpand, bool useUrl, int currentItem ) {
   QString result = QString::null, exp = QString::null;
   int begin, end, i;
//    int brackets = 0;
//    bool inQuotes = false;
   unsigned int idx = 0;
   while ( idx < stringToExpand.length() ) {
      if ( ( begin = stringToExpand.find( '%', idx ) ) == -1 ) break;
      if ( ( end = findEnd( stringToExpand, begin ) ) == -1 ) {
         kdWarning() << "Error: unterminated % in Expander::expandCurrent" << endl;
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
        char panelIndicator = exp.lower()[0].latin1();
        exp.replace( 0, 1, "" );
        for ( i = 0; i < placeholderCount(); ++i )
           if ( exp == placeholder( i )->expression() ) {
//               kdDebug() << "---------------------------------------" << endl;
              result += placeholder( i )->expFunc( getPanel( panelIndicator ), parameter, useUrl, currentItem );
//               kdDebug() << "---------------------------------------" << endl;
              break;
           }
        if ( i == placeholderCount() ) { // didn't find an expander
           kdWarning() << "Error: unrecognized %" << panelIndicator << exp << "% in Expander::expand" << endl;
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

QStringList Expander::separateParameter( QString* exp, bool useUrl ) {
   QStringList parameter;
   QString result;
   int begin, end;
   if ( ( begin = exp->find( '(' ) ) != -1 ) {
      if ( ( end = exp->findRev( ')' ) ) == -1 ) {
         kdWarning() << "Error: missing ')' in Expander::separateParameter" << endl;
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
//             kdWarning() << " ---- parameter: " << parameter.join(";") << endl;
         }
         idx++;
      }
      parameter.append( result.mid( begin, idx - begin) );  //don't forget the last one
      
      for (QStringList::Iterator it = parameter.begin(); it != parameter.end(); ++it) {
         *it = (*it).stripWhiteSpace();
         if ( (*it).left(1) == "\"" )
            *it = (*it).mid(1, (*it).length() - 2 );
         *it = expandCurrent( *it, useUrl, -1 );
      }
   }
      
//    kdWarning() << "------- exp: " << *exp << " ---- parameter: " << parameter.join(";") << endl;
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
