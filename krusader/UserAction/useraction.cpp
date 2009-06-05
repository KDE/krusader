/*****************************************************************************
 * Copyright (C) 2004 Jonas Bähr <jonas.baehr@web.de>                        *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#include "useraction.h"

#include <QtCore/QHash>
#include <QtCore/QTextStream>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

#include <kdebug.h>
#include <kurl.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

#include "kraction.h"
#include "../krusader.h"
#include "../krusaderview.h"
#include "../Panel/listpanel.h"
#include "../Panel/panelfunc.h"

UserAction::UserAction() {
   krOut << "Initialisising useractions..." << endl;
   readAllFiles();
   krOut << _actions.count() << " useractions read." << endl;
}

UserAction::~UserAction() {
  // KrActions are deleted by Krusader's KActionCollection
}

void UserAction::removeKrAction( KrAction* action ) {
   _actions.removeAll( action );
   if ( _defaultActions.contains( action->getName() ) )
      _deletedActions.insert( action->getName() );
}

void UserAction::setAvailability() {
   setAvailability( ACTIVE_FUNC->files()->vfs_getFile( ACTIVE_PANEL->view->getCurrentItem() ) );
}

void UserAction::setAvailability( const KUrl& currentURL ) {
   //kDebug() << "UserAction::setAvailability currendFile: " << currentURL.url() << endl;
   // disable the entries that should not appear in this folder
   QListIterator<KrAction *> it( _actions );
   while (it.hasNext())
   {
      KrAction * action = it.next();
      action->setEnabled( action->isAvailable( currentURL ) );
   }
}

void UserAction::populateMenu( KActionMenu* menu, const KUrl *currentURL ) {
   // I have not found any method in Qt/KDE
   // for non-recursive searching of children by name ...
   QMap<QString, KActionMenu *> categoryMap;
   QList<KrAction *> uncategorised;
   
   foreach( KrAction* action, _actions )
   {
      const QString category = action->category();
      if ( ! action->isEnabled() )
         continue;
      if ( currentURL != NULL && ! action->isAvailable( *currentURL ) )
         continue;
      if ( category.isEmpty() )
      {
         uncategorised.append( action );
      }
      else
      {
         if (! categoryMap.contains( category ) )
         {
            KActionMenu *categoryMenu = new KActionMenu( category, menu );
            categoryMenu->setObjectName( category );
            categoryMap.insert( category, categoryMenu );
         }
         KActionMenu *targetMenu = categoryMap.value( category );
         targetMenu->addAction( action );
      }
   }
   
   QMutableMapIterator<QString, KActionMenu *> mapIter(categoryMap);
   while ( mapIter.hasNext() )
   {
      mapIter.next();
      menu->addAction( mapIter.value() );
   }
   
   foreach ( KrAction* action, uncategorised )
   {
      menu->addAction( action );
   };
}

QStringList UserAction::allCategories() {
   QStringList actionCategories;

   QListIterator<KrAction *> it( _actions );
   while (it.hasNext())
   {
      KrAction * action = it.next();
      if ( actionCategories.indexOf( action->category() ) == -1 )
         actionCategories.append( action->category() );
   }

   return actionCategories;
}

QStringList UserAction::allNames() {
   QStringList actionNames;

   QListIterator<KrAction *> it( _actions );
   while (it.hasNext())
   {
      KrAction * action = it.next();
      actionNames.append( action->getName() );
   }

   return actionNames;
}

void UserAction::readAllFiles() {
   QString filename = KStandardDirs::locate( "data", ACTION_XML ); // locate returns the local file if it exists, else the global one is retrieved.
   if ( ! filename.isEmpty() )
      readFromFile( KStandardDirs::locate( "data", ACTION_XML ), renameDoublicated );

   filename = KStandardDirs::locate( "data", ACTION_XML_EXAMPLES );
   if ( ! filename.isEmpty() )
      readFromFile( KStandardDirs::locate( "data", ACTION_XML_EXAMPLES ), ignoreDoublicated ); // ignore samples which are already in the normal file
}

void UserAction::readFromFile( const QString& filename, ReadMode mode, KrActionList* list ) {
  QDomDocument* doc = new QDomDocument( ACTION_DOCTYPE );
  QFile file( filename );
  if( file.open( QIODevice::ReadOnly ) ) {
    //kDebug() << "UserAction::readFromFile - " << filename << "could be opened" << endl;
    if( ! doc->setContent( &file ) ) {
      //kDebug() << "UserAction::readFromFile - content set - failed" << endl;
      // if the file doesn't exist till now, the content CAN be set but is empty.
      // if the content can't be set, the file exists and is NOT an xml-file.
      file.close();
      delete doc; doc = 0;
      KMessageBox::error( MAIN_VIEW,
      		i18n( "The file %1 does not contain valid UserActions.\n", filename ), // text
      		i18n("UserActions - can't read from file!") // caption
      	);
    }
    file.close();

    if ( doc ) {
      QDomElement root = doc->documentElement();
      // check if the file has got the right root-element (ACTION_ROOT)
      // this finds out if the xml-file read to the DOM is really a krusader
      // useraction-file
      if( root.tagName() != ACTION_ROOT ) {
        KMessageBox::error( MAIN_VIEW,
        	i18n( "The actionfile's root-element isn't called "ACTION_ROOT", using %1", filename ),
        	i18n( "UserActions - can't read from file!" )
        );
        delete doc; doc = 0;
      }
      readFromElement( root, mode, list );
      delete doc;
    }

  } // if ( file.open( QIODevice::ReadOnly ) )
  else {
      KMessageBox::error( MAIN_VIEW,
      		i18n( "Unable to open actionfile %1", filename ),
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

         if ( mode == ignoreDoublicated ) {
            _defaultActions.insert( name );
             if ( krApp->actionCollection()->action( name ) || _deletedActions.contains( name ) )
                continue;
         }

         QString basename = name + "_%1";
         int i = 0;
         // appent a counter till the name is unique... (this checks every action, not only useractions)
         while ( krApp->actionCollection()->action( name ) )
            name = basename.arg( ++i );

         KrAction* act = new KrAction( krApp->actionCollection(), name );
         if ( act->xmlRead( e ) ) {
            _actions.append( act );
            if ( list )
               list->append( act );
         }
         else
            delete act;
      }
      else
      if ( e.tagName() == "deletedAction" ) {
         QString name = e.attribute( "name" );
         if ( name.isEmpty() ) {
            krOut << "A deleted action without name detected! \nThis is an error in the file.";
            continue;
         }
         _deletedActions.insert( name );
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
   QString filename = KStandardDirs::locateLocal( "data", ACTION_XML );

   QDomDocument doc = createEmptyDoc();
   QDomElement root = doc.documentElement();

   foreach ( const QString &name, _deletedActions ) {
      QDomElement element = doc.createElement("deletedAction");
      element.setAttribute( "name", name );
      root.appendChild( element );
   }
   
   QListIterator<KrAction *> it( _actions );
   while (it.hasNext())
   {
      KrAction * action = it.next();
      root.appendChild( action->xmlDump( doc ) );
   }

   return writeToFile( doc, filename );
}

bool UserAction::writeToFile( const QDomDocument& doc, const QString& filename ) {
   QFile file( filename );
   if( ! file.open( QIODevice::WriteOnly ) )
      return false;

/* // This is not needed, because each DomDocument created with UserAction::createEmptyDoc already contains the processinstruction
   if ( ! doc.firstChild().isProcessingInstruction() ) {
      // adding: <?xml version="1.0" encoding="UTF-8" ?> if not already present
      QDomProcessingInstruction instr = doc.createProcessingInstruction( "xml", ACTION_PROCESSINSTR );
      doc.insertBefore( instr, doc.firstChild() );
   }
*/

   QTextStream ts( &file );
   ts.setCodec("UTF-8");
   ts << doc.toString();

   file.close();
   return true;
}




