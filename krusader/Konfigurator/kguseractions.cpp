/***************************************************************************
                        kguseractions.cpp  -  description
                             -------------------
    copyright            : (C) 2004 by Jonas Bähr
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kguseractions.h"
#include "../defaults.h"
#include "../krusader.h"

#include <klocale.h>
#include <kpushbutton.h>
#include <kdebug.h>
#include <klistbox.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

#include <qsplitter.h> 

#include "../UserMenu/usermenu.h"
#include "../GUI/actionproperty.h"
#include "../UserAction/useraction.h"
#include "../UserAction/useractionxml.h"
#include "../UserAction/useractionproperties.h"

//This is the filter in the KFileDialog of Import/Export:
#define	FILE_FILTER	i18n("*.xml|xml-files") + '\n' + i18n("*|all files")
//This is the name of the config-entry:
#define	CFG_DONT_ASK_AGAIN_REMOVE_ACTION	"dontAskAgain_removeAction"

KgUserActions::KgUserActions( bool first, QWidget* parent,  const char* name ) :
  KonfiguratorPage( first, parent, name )
{
  QGridLayout *kgStartupLayout = new QGridLayout( parent, 1, 1, 0, 6, "kgStartupLayout" );

  QSplitter *split = new QSplitter( parent, "kguseractions splitter");
  actionList = new KListBox( split );
  actionProperties = new ActionProperty( split );

  kgStartupLayout->addWidget( split, 0, 0 );
  
  QVBoxLayout *vboxButtons = new QVBoxLayout( parent, 0, 6, "vboxButtons"); 
  QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
  
  okButton = new KPushButton( i18n("Ok"), parent, "okButton" );
  resetButton = new KPushButton( i18n("Reset"), parent, "resetButton" );
  newButton = new KPushButton( i18n("New Action"), parent, "newButton" );
  removeButton = new KPushButton( i18n("Remove Action"), parent, "removeButton" );
  importButton = new KPushButton( i18n("Import Action"), parent, "importButton" );
  exportButton = new KPushButton( i18n("Export Action"), parent, "exportButton" );

  vboxButtons->addWidget( newButton );
  vboxButtons->addWidget( removeButton );
  vboxButtons->addWidget( importButton );
  vboxButtons->addWidget( exportButton );
  vboxButtons->addItem( spacer );
  vboxButtons->addWidget( okButton );
  vboxButtons->addWidget( resetButton );
  
  kgStartupLayout->addLayout( vboxButtons, 0, 1 );
  
  _importXML = 0;
  
  // fill the ListBox with all actions
  //TODO display the titles, not the names
  //TODO change this to an tree-view, grouped by category
  actionList->insertStringList( krUserAction->xml()->getActionNames() ); 
 
  
  connect(  actionList, SIGNAL( currentChanged(QListBoxItem*) ), this, SLOT( slotChangeAction() ) );
  connect( okButton, SIGNAL( clicked() ), this, SLOT( slotUpdateAction() ) );
  connect( resetButton, SIGNAL( clicked() ), this, SLOT( slotReset() ) );
  connect( newButton, SIGNAL( clicked() ), this, SLOT( slotNewAction() ) );
  connect( removeButton, SIGNAL( clicked() ), this, SLOT( slotRemoveAction() ) );
  connect( importButton, SIGNAL( clicked() ), this, SLOT( slotImport() ) );
  connect( exportButton, SIGNAL( clicked() ), this, SLOT( slotExport() ) );

  if ( actionList->count() > 0 ) {
    actionList->setCurrentItem( actionList->topItem() );
    slotChangeAction();
  }
}

KgUserActions::~KgUserActions() {
  delete _importXML;
}

void KgUserActions::slotChangeAction() {
  //kdDebug() << "entering KgUserActions::slotChangeAction" << endl;
  
  if ( _importXML != 0 ) {
    actionProperties->leDistinctName->setEnabled(true);
    actionProperties->updateGUI( _importXML->readAction( actionList->currentText() ) );
  }
  else {
    // the discinct name is used as ID it is not allowd to change it afterwards because it is may referenced anywhere else
    actionProperties->leDistinctName->setEnabled(false);
    actionProperties->updateGUI( krUserAction->xml()->readAction( actionList->currentText() ) );
  }

}

void KgUserActions::slotUpdateAction() {
  //kdDebug() << "entering KgUserActions::slotUpdateAction" << endl;
  
  // check that we have a command line, title and a name
   if ( ! actionProperties->checkProperties() )
     return;
     
  // This is to resolve name-conflicts after the import
  if ( _importXML != 0 ) {
    _importXML->removeAction( actionList->currentText() );
    krUserAction->xml()->addActionToDom( actionProperties->properties() );
    krUserAction->addKrAction( actionProperties->properties() );
    actionList->removeItem( actionList->currentItem() );
    
    //check if there still are imported actions with name-conflicts
    if ( _importXML->getActionNames().isEmpty() ) {
      delete _importXML;
      _importXML = 0;
      
      // re-fill the action-list with the stored actions
      actionList->clear();
      actionList->insertStringList( krUserAction->xml()->getActionNames() );
      
      newButton->setEnabled( true );
      importButton->setEnabled( true );
      exportButton->setEnabled( true );
    }
  } //finished with the name-conflict stuff
  else {
    if ( actionProperties->leDistinctName->isEnabled() ) {
      // := new entry
      krUserAction->xml()->addActionToDom( actionProperties->properties() );
      krUserAction->addKrAction( actionProperties->properties() );
      actionList->insertItem( *actionProperties->properties()->name() );
    }
    else // := edit an existing
      krUserAction->xml()->updateAction( actionProperties->properties() );
      //krUserAction->updateKrAction( actionProperties->properties()->name, actionProperties->properties() );
  }
  
  krUserAction->updateKrAction( *actionProperties->properties()->name(), actionProperties->properties() );
  krUserAction->xml()->writeActionDom();
  
  slotChangeAction();
  
  //kdDebug() << "leaving KgUserActions::slotUpdateAction" << endl;
}

void KgUserActions::slotReset() {
  
  if ( actionProperties->leDistinctName->isEnabled() )
    slotNewAction();
  else
    slotChangeAction();
}

void KgUserActions::slotNewAction() {
  //kdDebug() << "entering KgUserActions::slotNewAction" << endl;
  
  actionProperties->leDistinctName->setEnabled(true);
  
  UserActionProperties *prop = new UserActionProperties;
  
  actionProperties->updateGUI( prop );
  
  delete prop;
}

void KgUserActions::slotRemoveAction() {
  //kdDebug() << "entering KgUserActions::slotRemoveAction" << endl;
  if ( actionList->currentText().isEmpty() )
    return;
  
    int MessageDelete = KMessageBox::warningContinueCancel ( this,	//parent
		i18n("Are you sure that you want to remove this action?\nA recovery is not possible!"),	//text
		i18n("Remove this action?"), 	//caption
		i18n("Remove"),	//Label for the continue-button
		CFG_DONT_ASK_AGAIN_REMOVE_ACTION,	//dontAskAgainName (for the config-file)
		KMessageBox::Dangerous) ;

    if ( MessageDelete != KMessageBox::Continue )
      return;
    
  // This is to resolve name-conflicts after the import
  if ( _importXML != 0 ) {
    _importXML->removeAction( actionList->currentText() );
    actionList->removeItem( actionList->currentItem() );
    
    //check if there still are imported actions with name-conflicts
    if ( _importXML->getActionNames().isEmpty() ) {
      delete _importXML;
      _importXML = 0;
      
      // re-fill the action-list with the stored actions
      actionList->clear();
      actionList->insertStringList( krUserAction->xml()->getActionNames() );
      
      newButton->setEnabled( true );
      importButton->setEnabled( true );
      exportButton->setEnabled( true );
    }
  } //finished with the name-conflict stuff
  else {
    //TODO: make actionList multiselect and remove all selected actions  
    krUserAction->xml()->removeAction( actionList->currentText() );
    krUserAction->removeKrAction( *actionProperties->properties()->name() );
    actionList->removeItem( actionList->currentItem() );

    krUserAction->xml()->writeActionDom();
    krApp->userMenu->update();
  }
  
  slotChangeAction();  
}

void KgUserActions::slotImport() {
  //kdDebug() << "entering KgUserActions::slotImport" << endl;
  
  QString filename = KFileDialog::getOpenFileName(QString::null, FILE_FILTER, this);
  if ( filename.isEmpty() )
    return;
  
  _importXML = new UserActionXML( filename );
  //kdDebug() << "ActionImport: " << filename << " read to DOM" << endl;
  if ( _importXML == 0 ) {
    KMessageBox::error( this, i18n("The file you've choosen don't seem to be a valid action-file.") );
    return;
  }
  //kdDebug() << "ActionImport: DOM valid, continueing" << endl;
  
  QStringList actionNames = _importXML->getActionNames();
  
  if ( actionNames.isEmpty() ) {
    KMessageBox::error( this, i18n("No actions found in this file.") );
    delete _importXML;
    _importXML = 0;
    return;
  }
  
  for ( QStringList::iterator it = actionNames.begin(); it != actionNames.end(); ++it ) {
    if ( ! krUserAction->xml()->nameExists( *it ) ) {
      UserActionProperties* prop = _importXML->readAction( *it );
      krUserAction->addKrAction( prop );
      krUserAction->xml()->addActionToDom( prop );
      _importXML->removeAction( *it );
      actionList->insertItem( *it );
    }
  } //for
  
  krUserAction->xml()->writeActionDom();
  krApp->userMenu->update();
  
  //check if there still are actions which aren't imported because of name-conflicts
  actionNames = _importXML->getActionNames();
  if ( actionNames.isEmpty() ) {
    //kdDebug() << "KgUserActions::slotImport: All actions imported" << endl;
    delete _importXML;
    _importXML = 0;
  }
  else {
    //kdDebug() << "KgUserActions::slotImport: Not all actions importet due to name-conflicts" << endl;
    KMessageBox::sorry( this, i18n("Cound't import all actions because of name-conflicts. Check the action-list on the left to fix them.\nPlease note that these actions won't be imported if you close the Konfigurator now!") );
    actionList->clear();
    actionList->insertStringList( actionNames );
    
    newButton->setEnabled( false );
    importButton->setEnabled( false );
    exportButton->setEnabled( false );
  }
  
  if ( actionList->count() > 0 ) {
    actionList->setCurrentItem( actionList->topItem() );
    slotChangeAction();
  }

}

void KgUserActions::slotExport() {
  //kdDebug() << "entering KgUserActions::slotExport" << endl;
  if ( actionList->currentText().isEmpty() )
    return;
  
  QString filename = KFileDialog::getSaveFileName(QString::null, FILE_FILTER, this);
  if ( filename.isEmpty() )
    return;

  //if the file already holds a xml-file with type KrusaderUserAction the new action will be added to this one.
  UserActionXML* exportXML = new UserActionXML( filename );
  exportXML->addActionToDom( actionProperties->properties() );
  exportXML->writeActionDom();
  
  delete exportXML;
}

#include "kguseractions.moc"
