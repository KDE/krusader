//
// C++ Implementation: useraction
//
// Description: This manages all useractions
//
//
// Author: Jonas B�r (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <kdebug.h>
#include <kurl.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

#include <qstring.h>
#include <qdom.h>

#include "useraction.h"
#include "kraction.h"

#include "../krusader.h"
#include "../krusaderview.h"
#include "../Panel/listpanel.h"
#include "../Panel/panelfunc.h"


UserAction::UserAction() {
   _actions.setAutoDelete( false ); // the actions are "owned" by Krusader's KActionCollection, so they should not be deleted
   krOut << "Initialisising useractions..." << endl;
   readAllFiles();
   krOut << _actions.count() << " useractions read." << endl;
}

UserAction::~UserAction() {
  // KrActions are deleted by Krusader's KActionCollection
}

void UserAction::setAvailability() {
   setAvailability( ACTIVE_FUNC->files()->vfs_getFile( ACTIVE_PANEL->view->getCurrentItem() ) );
}

void UserAction::setAvailability( const KURL& currentURL ) {
   //kdDebug() << "UserAction::setAvailability currendFile: " << currentURL.url() << endl;
   // disable the entries that should not appear in this folder
   for ( KrAction* action = _actions.first(); action; action = _actions.next() )
      action->setEnabled( action->isAvailable( currentURL ) );
}

void UserAction::populateMenu( KPopupMenu* menu ) {
   for ( KrAction* action = _actions.first(); action; action = _actions.next() )
      if ( ! action->isPlugged( menu ) )
         action->plug( menu );
}

QStringList UserAction::allCategories() {
   QStringList actionCategories;

   for ( KrAction* action = _actions.first(); action; action = _actions.next() )
      if ( actionCategories.find( action->category() ) == actionCategories.end() )
         actionCategories.append( action->category() );

  return actionCategories;
}

QStringList UserAction::allNames() {
   QStringList actionNames;

   for ( KrAction* action = _actions.first(); action; action = _actions.next() )
      actionNames.append( action->name() );

  return actionNames;
}

void UserAction::readAllFiles() {
   QString filename = locate( "data", ACTION_XML ); // locate returns the local file if it exists, else the global one is retrieved.
   if ( ! filename.isEmpty() )
      readFromFile( locate( "data", ACTION_XML ), renameDoublicated );

   filename = locate( "data", ACTION_XML_EXAMPLES );
   if ( ! filename.isEmpty() )
      readFromFile( locate( "data", ACTION_XML_EXAMPLES ), ignoreDoublicated ); // ignore samples which are already in the normal file
}

void UserAction::readFromFile( const QString& filename, ReadMode mode, KrActionList* list ) {
  QDomDocument* doc = new QDomDocument( ACTION_DOCTYPE );
  QFile file( filename );
  if( file.open( IO_ReadOnly ) ) {
    //kdDebug() << "UserAction::readFromFile - " << filename << "could be opened" << endl;
    if( ! doc->setContent( &file ) ) {
      //kdDebug() << "UserAction::readFromFile - content set - failed" << endl;
      // if the file doesn't exist till now, the content CAN be set but is empty.
      // if the content can't be set, the file exists and is NOT an xml-file.
      file.close();
      delete doc; doc = 0;
      KMessageBox::error( MAIN_VIEW,
      		i18n( "The file %1 does not contain valid UserActions.\n" ).arg( filename ), // text
      		i18n("UserActions - can't read from file!") // caption
      	);
    }
    file.close();

    if ( doc ) {
      QDomElement root = doc->documentElement();
      // check if the file got the right root-element (ACTION_ROOT) - this finds out if the xml-file read to the DOM is realy an krusader useraction-file
      if( root.tagName() != ACTION_ROOT ) {
        KMessageBox::error( MAIN_VIEW,
        	i18n( "The actionfile's root-element isn't called "ACTION_ROOT", using %1").arg( filename ),
        	i18n( "UserActions - can't read from file!" )
        );
        delete doc; doc = 0;
      }
      readFromElement( root, mode, list );
      delete doc;
    }

  } // if ( file.open( IO_ReadOnly ) )
  else {
      KMessageBox::error( MAIN_VIEW,
      		i18n( "Unable to open actionfile %1").arg( filename ),
      		i18n( "UserActions - can't read from file!" )
      );
   }

}

void UserAction::readFromElement( const QDomElement& element, ReadMode mode, KrActionList* list ) {
   for ( QDomNode node = element.firstChild(); ! node.isNull(); node = node.nextSibling() ) {
      QDomElement e = node.toElement();
      if ( e.isNull() )
         continue; // this should skip nodes which are not elements ( i.e. comments, <!-- -->, or text nodes)

      if ( e.tagName() == "action" ) {
         QString name = e.attribute( "name" );
         if ( name.isEmpty() ) {
            KMessageBox::error( MAIN_VIEW,
           	i18n( "Action without name detected. This action will not be imported!\nThis is an error in the file, you may want to correct it." ),
           	i18n( "UserActions - invalid action" )
           );
           continue;
         }

         if ( mode == ignoreDoublicated && krApp->actionCollection()->action( name.toLatin1() ) )
            continue;

         QString basename = name + "_%1";
         int i = 0;
         // appent a counter till the name is unique... (this checks every action, not only useractions)
         while ( krApp->actionCollection()->action( name.toLatin1() ) )
            name = basename.arg( ++i );

         KrAction* act = new KrAction( krApp->actionCollection(), name.toLatin1() );
         if ( act->xmlRead( e ) ) {
            _actions.append( act );
            if ( list )
               list->append( act );
         }
         else
            delete act;
      }
   } // for
}

QDomDocument UserAction::createEmptyDoc() {
   QDomDocument doc = QDomDocument( ACTION_DOCTYPE );
   // adding: <?xml version="1.0" encoding="UTF-8" ?>
   doc.appendChild( doc.createProcessingInstruction( "xml", ACTION_PROCESSINSTR ) );
   //adding root-element
   doc.appendChild( doc.createElement( ACTION_ROOT ) ); // create new actionfile by adding a root-element ACTION_ROOT
   return doc;
}

bool UserAction::writeActionFile() {
   QString filename = locateLocal( "data", ACTION_XML );

   QDomDocument doc = createEmptyDoc();
   QDomElement root = doc.documentElement();
   for ( KrAction* action = _actions.first(); action; action = _actions.next() )
      root.appendChild( action->xmlDump( doc ) );

   return writeToFile( doc, filename );
}

bool UserAction::writeToFile( const QDomDocument& doc, const QString& filename ) {
   QFile file( filename );
   if( ! file.open( IO_WriteOnly ) )
      return false;

/* // This is not needed, because each DomDocument created with UserAction::createEmptyDoc already contains the processinstruction
   if ( ! doc.firstChild().isProcessingInstruction() ) {
      // adding: <?xml version="1.0" encoding="UTF-8" ?> if not already present
      QDomProcessingInstruction instr = doc.createProcessingInstruction( "xml", ACTION_PROCESSINSTR );
      doc.insertBefore( instr, doc.firstChild() );
   }
*/

   QTextStream ts( &file );
   ts.setEncoding(ts.UnicodeUTF8);
   ts << doc.toString();

   file.close();
   return true;
}




