/***************************************************************************
                         usermenuaddimpl.cpp  -  description
                            -------------------
   begin                : Fri Dec 26 2003
   copyright            : (C) 2003 by Shie Erlich & Rafi Yanai
   email                :
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "usermenuaddimpl.h"
#include <klocale.h>
#include <qtoolbutton.h>
#include <klineedit.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>

#define ACTIVE_MASK     0x1000
#define OTHER_MASK      0x2000
#define EXECUTABLE_ID   0x3000

UserMenuAddImpl::UserMenuAddImpl( UserMenu *menu, QWidget *parent, const char *name ) : UserMenuAdd( parent, name ) {
   // create the 'add' popup menu
   _popup = new KPopupMenu( this );
   _activeSub = new KPopupMenu( _popup );
   _otherSub = new KPopupMenu( _popup );

   _popup->insertItem( i18n( "Active panel" ), _activeSub );
   _popup->insertItem( i18n( "Other panel" ), _otherSub );
   _popup->insertItem( i18n( "Executable" ), EXECUTABLE_ID );

   // read the expressions array from the user menu and populate menus
   for ( int i = 0; i < UserMenu::numOfExps; i++ ) {
      _activeSub->insertItem( UserMenu::expressions[ i ].description, ( i | ACTIVE_MASK ) );
      _otherSub->insertItem( UserMenu::expressions[ i ].description, ( i | OTHER_MASK ) );
   }
}
UserMenuAddImpl::~UserMenuAddImpl() {}

void UserMenuAddImpl::accept() {
   // check that we have a command line and a name
   if (cmdName->text().simplifyWhiteSpace().isEmpty()) {
      KMessageBox::error(this, i18n("Please set a name for the menu entry"));
      return;
   } else if (cmdLine->text().simplifyWhiteSpace().isEmpty()) {
      KMessageBox::error(this, i18n("Command line is empty"));
      return;
   } else {
      // set the execution type
      UserMenuProc::ExecType type = UserMenuProc::None;
      if (runInTerminal->isChecked()) type = UserMenuProc::Terminal;
      else if (collectOutput->isChecked()) type = UserMenuProc::OutputOnly;

      emit newEntry( cmdName->text().simplifyWhiteSpace(), cmdLine->text().simplifyWhiteSpace(),
                     type, separateStderr->isChecked(), acceptLocalURLs->isChecked() || acceptRemote->isChecked(),
                     acceptRemote->isChecked(), showEverywhere->isChecked(),
                     QStringList::split(';', showOnlyIn->text().simplifyWhiteSpace()));
   }
   UserMenuAdd::accept();
}

void UserMenuAddImpl::popupAddMenu() {
   int res = _popup->exec( mapToGlobal( QPoint( addButton->pos().x() + addButton->width() * 3 / 2,
                                        addButton->pos().y() + addButton->height() / 2 ) ) );
   if ( res == -1 ) return ;
   // add the selected flag to the command line
   if ( res == EXECUTABLE_ID ) { // did the user need an executable ?
      // select an executable
      QString filename = KFileDialog::getOpenFileName(QString::null, QString::null, this);
      if (filename != QString::null) cmdLine->insert(filename+' ');
   } else { // user selected something from the menus
      QString exp = UserMenu::expressions[ res & ~( ACTIVE_MASK | OTHER_MASK ) ].expression;
      // replace the '_' with 'a' or 'o'.
      if ( res & ACTIVE_MASK ) exp.replace( '_', 'a' );
      else exp.replace( '_', 'o' ); // it can only be ACTIVE_MASK or OTHER_MASK
      cmdLine->insert( exp+' ' ); // with extra space
   }
}

void UserMenuAddImpl::addShowInFolder() {
   QString folder = KFileDialog::getExistingDirectory(QString::null, this);
   if (folder != QString::null) showIn->insert(folder+';');
}
