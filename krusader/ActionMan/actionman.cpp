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
 : KDialog(parent)
{
	setWindowModality( Qt::WindowModal );
   setCaption(i18n("ActionMan - Manage Your Useractions"));
   setButtons( KDialog::Apply | KDialog::Close );

   userActionPage = new UserActionPage( this );
   setMainWidget( userActionPage );

   connect( this, SIGNAL( applyClicked() ), this, SLOT( slotApply() ) );
   connect( this, SIGNAL( closeClicked() ), this, SLOT( slotClose() ) );
   connect( userActionPage, SIGNAL( changed() ), SLOT( slotEnableApplyButton() ) );
   connect( userActionPage, SIGNAL( applied() ), SLOT( slotDisableApplyButton() ) );
   enableButtonApply( false );

   exec();
}

ActionMan::~ActionMan() {
}

void ActionMan::slotClose() {
   if ( userActionPage->readyToQuit() )
      reject();
}

void ActionMan::slotApply() {
   userActionPage->applyChanges();
}

void ActionMan::slotEnableApplyButton() {
   enableButtonApply( true );
}

void ActionMan::slotDisableApplyButton() {
   enableButtonApply( false );
}



#include "actionman.moc"
