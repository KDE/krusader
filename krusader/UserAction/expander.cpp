//
// C++ Implementation: expander
//
// Description: 
//
//
// Author: Jonas Bähr (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <expander.h>

#include "../krusader.h"
#include "../krusaderview.h"
#include "../panelmanager.h"
#include "../Panel/listpanel.h"
#include "../Panel/panelfunc.h"
#include "../Synchronizer/synchronizergui.h"

#include <kdebug.h>
#include <kinputdialog.h>
#include <qstringlist.h>
#include <qclipboard.h>

#define ACTIVE    krApp->mainView->activePanel

ListPanel* Expander::getPanel( const char& panelIndicator ) {
   switch ( panelIndicator ) {
   case 'a':
      return krApp->mainView->activePanel;
   case 'o':
      return krApp->mainView->activePanel->otherPanel;
   case 'l':
      return krApp->mainView->left;
   case 'r':
      return krApp->mainView->right;
   case '_':
      return 0;
   default:
      kdWarning() << "Expander: unknown Panel " << panelIndicator << endl;
      return 0;
   }
}


Expander::Placeholder Expander::placeholder[ Expander::numOfPlaceholder ] = {
// { "expression", "description", expander_func, {parameter}, no. of parameters, need a panel }
// if "expression" == "" => instertSeparator()
// parameter: { "description", "default", nessesary }, ... (max 5, see .h)
         {"Path", i18n("panel's path"), exp_Path, {
                        {i18n("Automatic escape spaces"), "__yes", false}
                    }, 1, true},
         {"Count", i18n("number of ..."), exp_Count, {
                        {i18n("count all:"), "__choose:All;Files;Dirs;Selected", false}
                    }, 1, true},
         {"Filter", i18n("filter mask (*.h, *.cpp ...)"), exp_Filter, {
                    }, 0, true},
         {"Current", i18n("current file (!= selected file)"), exp_Current, {
                        {i18n("Ommit the current path (optional)"), "__no", false},
                        {i18n("Automatic escape spaces"), "__yes", false}
                    }, 2, true},
         {"List", i18n("Itemlist of ..."), exp_List, {
                        {i18n("Which items"), "__choose:All;Files;Dirs;Selected", false},
                        {i18n("Separator between the items (optional)"), " ", false},
                        {i18n("Ommit the current path (optional)"), "__no", false},
                        {i18n("Mask (optional, all but 'Selected')"), "__select", false},
                        {i18n("Automatic escape spaces"), "__yes", false}
                    }, 5, true},
//--------- Internals --------------
         {"", "", 0, {}, 0, true},  // Separator
         {"Select", i18n("Manipulate the selection"), exp_Select, {
                        {i18n("Selectionmask"), "__select", true},
                        {i18n("Manipulate in which way"), "__choose:Set;Add;Remove", false}
                    }, 2, true},
         {"Bookmark", i18n("Jump to a bookmark"), exp_Bookmark, {
                        {i18n("please choose the bookmark"), "__bookmark", true},
                        {i18n("open the bookmark in a new tab"), "__no", false}
                    }, 2, true},
//          {"Search", i18n("Search for files using"),0, {
//                         {i18n("please choose the setting"), "__search", true},  //TODO: add this action as soon as the search supports saving search-settings
//                         {i18n("open the search in a new tab"), "__yes", false}  //TODO: add this also to panel-dependent as soon as vfs support the display of search-results
//                     }, 1, true},
//--------- Panel independent --------------
         {"Ask", i18n("Question the user for a parameter"), exp_Ask, {
                        {i18n("Question"), "Where do you want do go today?", true},
                        {i18n("Preset (optional)"), "", false},
                        {i18n("Caption (optional)"), "", false}
                    }, 3, false},
         {"Clipboard", i18n("Copy to clipboard"), exp_Clipboard, {
                        {i18n("What should be copied"), "__placeholder", true},
                        {i18n("Append to the current clipboard-content with this seperator (optional)"), "", false}
                    }, 2, false},
         {"Copy", i18n("Copy a file/folder"), exp_Copy, {
                        {i18n("What should be copied"), "__placeholder", true},
                        {i18n("Where it should be copied"), "__placeholder", true},
                    }, 2, false},
         {"Sync", i18n("Opens a synchronizer-profile"), exp_Sync, {
                        {i18n("Choose a profile"), "__syncprofile", true},
                    }, 1, false},
         {"Run", i18n("Execute a script"), 0, {
                        {i18n("Script"), "__file", true},
                    }, 1, false}
//          {"Search", i18n("Search for files using"),0, {
//                         {i18n("please choose the setting"), "__search", true},  //TODO: add this action as soon as the search supports saving search-settings
//                     }, 1, false},
    };


///////// expander functions //////////////////////////////////////////////////////////

QString Expander::exp_Path( const ListPanel* panel, const QStringList& parameter, const bool& useUrl, const int& ) {
   if ( panel == 0 ) {
      kdWarning() << "Expander: no panel specified for %_Path%; ignoring..." << endl;
      return QString::null;
   }
   
   QString result;
   
   if ( useUrl )
      result = panel->func->files()->vfs_getOrigin().url();
   else
      result = panel->func->files()->vfs_getOrigin().path();
         
   if ( parameter[0].lower() == "no" )  // don't escape spaces
      return result;
   else
      return result.replace(" ", "\\ ");
}

QString Expander::exp_Count( const ListPanel* panel, const QStringList& parameter, const bool&, const int& ) {
   if ( panel == 0 ) {
      kdWarning() << "Expander: no panel specified for %_Count%; ignoring..." << endl;
      return QString::null;
   }
   
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

QString Expander::exp_Filter( const ListPanel* panel, const QStringList&, const bool&, const int& ) {
   if ( panel == 0 ) {
      kdWarning() << "Expander: no panel specified for %_Filter%; ignoring..." << endl;
      return QString::null;
   }
   return panel->view->filterMask();
}

QString Expander::exp_Current( const ListPanel* panel, const QStringList& parameter, const bool& useUrl, const int& currentItem ) {
   if ( panel == 0 ) {
      kdWarning() << "Expander: no panel specified for %_Current%; ignoring..." << endl;
      return QString::null;
   }
   QString item;
   if ( currentItem >= 0) {  // in a callEach-cycle
      if ( panel == ACTIVE ) {  // callEach works only in the active panel
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

// items are separated by the second parameter
QString Expander::exp_List( const ListPanel* panel, const QStringList& parameter, const bool& useUrl, const int& ) {
   if ( panel == 0 ) {
      kdWarning() << "Expander: no panel specified for %_List%; ignoring..." << endl;
      return QString::null;
   }
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
kdDebug() << "result: '" << result << "'" << endl;
   return result;
}

// select in the panel using parameter[0] as mask
QString Expander::exp_Select( const ListPanel* panel, const QStringList& parameter, const bool&, const int& ) {
   if ( panel == 0 ) {
      kdWarning() << "Expander: no panel specified for %_Select%; ignoring..." << endl;
      return QString::null;
   }
   
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


// asks the user for an input
QString Expander::exp_Ask( const ListPanel*, const QStringList& parameter, const bool&, const int& ) {
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

// copies parameter[0] to clipboard
QString Expander::exp_Clipboard( const ListPanel*, const QStringList& parameter, const bool&, const int& ) {
//    kdDebug() << "Expander::exp_Clipboard, parameter[0]: '" << parameter[0] << "', Clipboard: " << KApplication::clipboard()->text() << endl;
    if ( parameter.count() <= 1 || parameter[1].isEmpty() || KApplication::clipboard()->text().isEmpty() )
       KApplication::clipboard()->setText( parameter[0] );
    else
       KApplication::clipboard()->setText( KApplication::clipboard()->text() + parameter[1] + parameter[0] );

   return QString::null;  // this doesn't return everything, that's normal!
}

// changes the path to parameter[0]
QString Expander::exp_Bookmark( const ListPanel* panel, const QStringList& parameter, const bool&, const int& ) {
   if ( panel == 0 ) {
      kdWarning() << "Expander: no panel specified for %_Bookmark%; ignoring..." << endl;
      return QString::null;
   }
   
   bool newTab = false;
   if ( parameter[1].lower() == "yes" )
      newTab = true;
   
   if ( newTab ) {
      if ( panel == krApp->mainView->left )
         krApp->mainView->leftMng->slotNewTab( parameter[0] );
      else
         krApp->mainView->rightMng->slotNewTab( parameter[0] );
   }
   else
      panel->func->openUrl( parameter[0], "" );
   
   return QString::null;  // this doesn't return everything, that's normal!
}

QString Expander::exp_Copy( const ListPanel*, const QStringList& parameter, const bool&, const int& ) {

   KURL src = parameter[0];
   KURL dest = parameter[1];
   
   if ( !dest.isValid() || !src.isValid() )
      return QString::null; // do nothing with invalid url's

   new KIO::CopyJob( src, dest, KIO::CopyJob::Copy, false, true );

   return QString::null;  // this doesn't return everything, that's normal!
}

QString Expander::exp_Sync( const ListPanel*, const QStringList& parameter, const bool&, const int& ) {
   if ( parameter[0].isEmpty() ) {
      kdWarning() << "Expander: no profile specified for %_Sync(profile)%; ignoring..." << endl;
      return QString::null;
   }

   SynchronizerGUI *sync = new SynchronizerGUI( krApp->mainView, parameter[0] );

   bool refresh = sync->wasSynchronization();
   delete sync;

   // refresh both panels:
   if( refresh ) {
      // first the other, then the active; else the focus would change
      getPanel( 'o' )->func->refresh();
      getPanel( 'a' )->func->refresh();
   }

   return QString::null;  // this doesn't return everything, that's normal!
}


/////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// end of expander functions //////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

QStringList Expander::expand( const QString& stringToExpand, bool useUrl, bool callEach ) {
   QStringList result;
   if ( callEach ) {
      QStringList items;
      ACTIVE->view->getSelectedItems( &items );
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
        for ( i = 0; i < numOfPlaceholder; ++i )
           if ( exp == placeholder[ i ].expression ) {
//               kdDebug() << "---------------------------------------" << endl;
              result += ( placeholder[ i ].expFunc ) ( getPanel( panelIndicator ), parameter, useUrl, currentItem );
//               kdDebug() << "---------------------------------------" << endl;
              break;
           }
        if ( i == numOfPlaceholder ) { // didn't find an expander
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
