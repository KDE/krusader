// Implementation of our JavaScript-Interpreter
//
// Author: Jonas Bähr (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//

#include "krjs.h"

#include "../krusader.h"
#include "../krusaderview.h"
#include "../panelmanager.h"

KrJS::KrJS() : KJSEmbed::KJSEmbedPart() {

   // make this object, the object Krusader, available for scripting as "Krusader":
   addObject( krApp, "Krusader" );

   // make this object available for scripting
   addObject( ACTIVE_MNG, "PanelManager" );
}
