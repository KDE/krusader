//
// C++ Implementation: actionman
//
// Description: This manages all useractions
//
//
// Author: Jonas Bähr (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "actionman.h"

#include <klocale.h>
#include <kmessagebox.h>

#include "useractionpage.h"
#include "../krusader.h"
#include "../UserAction/useraction.h"


ActionMan::ActionMan( QWidget * parent )
 : KDialogBase( parent, "ActionMan", true /*modal*/, "ActionMan - Manage your useractions", KDialogBase::Close )
{
   setPlainCaption(i18n("ActionMan - Manage Your Useractions"));

   userActionPage = new UserActionPage( this );
   setMainWidget( userActionPage );

   exec();
}

ActionMan::~ActionMan() {
}

void ActionMan::slotClose() {
   if ( userActionPage->readyToQuit() )
      reject();
}



#include "actionman.moc"
