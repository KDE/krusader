//
// C++ Implementation: useraction
//
// Description: This manages all useractions
//
//
// Author: Jonas Bähr (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <kdebug.h>
#include <kurl.h>

#include <qstring.h>
#include <qdom.h>

#include "useraction.h"
#include "useractionxml.h"
#include "useractionproperties.h"
#include "kraction.h"

#include "../krusader.h"
#include "../krusaderview.h"
#include "../Panel/listpanel.h"
#include "../Panel/panelfunc.h"


UserAction::UserAction() {
  kdDebug() << "Initialisising useractions..." << endl;
  _xml = new UserActionXML();
   if ( _xml != 0 ) {
     QStringList actionNames = _xml->getActionNames();
     for ( QStringList::Iterator it = actionNames.begin(); it != actionNames.end(); ++it )
        addKrAction( _xml->readAction(*it) );
     kdDebug() << _actions.count() << " useractions read." << endl;
   }
}

void UserAction::addKrAction( UserActionProperties* prop ) {
  _actions.append( new KrAction( prop, krApp->actionCollection() ) );
}

UserAction::~UserAction() {
  for ( KrActionList::iterator it = _actions.begin(); it != _actions.end(); ++it )
    delete *it;
}

KrAction* UserAction::action( const QString& name ) {
  for ( KrActionList::iterator it = _actions.begin(); it != _actions.end(); ++it ) {
    if ( name == *(*it)->properties()->name() )
      return *it;
  }
  return 0;
}

void UserAction::updateKrAction( const QString& name, UserActionProperties* prop ) {
   updateKrAction( action( name ), prop );
}

void UserAction::updateKrAction( KrAction* action, UserActionProperties* prop ) {
   action->setProperties( prop );
}

void UserAction::removeKrAction( const QString& name ) {
  for ( KrActionList::iterator it = _actions.begin(); it != _actions.end(); ++it ) {
    if ( *(*it)->properties()->name() == name ) {
      delete *it;
      _actions.remove( it );
      return;
    }
  }
}

void UserAction::setAvailability() {
   setAvailability( ACTIVE_FUNC->files()->vfs_getFile( ACTIVE_PANEL->view->getCurrentItem() ) );
}

void UserAction::setAvailability( const KURL& currentURL ) {
   //kdDebug() << "UserAction::setAvailability currendFile: " << currentURL.url() << endl;
   // disable the entries that should not appear in this folder
   for ( KrActionList::iterator it = _actions.begin(); it != _actions.end(); ++it )
      (*it)->setEnabled( (*it)->isAvailable( currentURL ) );
}

UserActionXML* UserAction::xml() {
   return _xml;
}

UserAction::KrActionList* UserAction::actionList() {
//    kdDebug() << "UserAction::actionList() provide " << _actions.count() << " useractions" << endl;
   return &_actions;
}

