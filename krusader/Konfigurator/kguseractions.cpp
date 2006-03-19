/***************************************************************************
                        kguseractions.cpp  -  description
                             -------------------
    copyright            : (C) 2004 by Jonas B�r
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
#include <klistview.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <klineedit.h>
#include <kclipboard.h>

#include <qsplitter.h> 

#include "../UserMenu/usermenu.h"
#include "../GUI/actionproperty.h"
#include "../GUI/useractionlistview.h"
#include "../UserAction/useraction.h"
#include "../UserAction/kraction.h"

//This is the filter in the KFileDialog of Import/Export:
static const char* FILE_FILTER = I18N_NOOP("*.xml|xml-files\n*|all files");
//This is the name of the config-entry:


KgUserActions::KgUserActions( bool first, QWidget* parent,  const char* name ) :
  KonfiguratorPage( first, parent, name )
{
  QGridLayout *kgStartupLayout = new QGridLayout( parent, 1, 1, 0, 6, "kgStartupLayout" );

  QSplitter *split = new QSplitter( parent, "kguseractions splitter");
  actionTree = new UserActionListView( split, "actionTree" );
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
  toClipButton = new KPushButton( i18n("Action to Clipboard"), parent, "toClipButton" );
  fromClipButton = new KPushButton( i18n("Action from Clipboard"), parent, "fromClipButton" );

  vboxButtons->addWidget( newButton );
  vboxButtons->addWidget( removeButton );
  vboxButtons->addWidget( importButton );
  vboxButtons->addWidget( exportButton );
  vboxButtons->addWidget( toClipButton );
  vboxButtons->addWidget( fromClipButton );
  vboxButtons->addItem( spacer );
  vboxButtons->addWidget( okButton );
  vboxButtons->addWidget( resetButton );

  kgStartupLayout->addLayout( vboxButtons, 0, 1 );

  _needApply = false;

  connect(  actionTree, SIGNAL( currentChanged(QListViewItem*) ), this, SLOT( slotChangeAction() ) );
  connect( okButton, SIGNAL( clicked() ), this, SLOT( slotUpdateAction() ) );
  connect( resetButton, SIGNAL( clicked() ), this, SLOT( slotReset() ) );
  connect( newButton, SIGNAL( clicked() ), this, SLOT( slotNewAction() ) );
  connect( removeButton, SIGNAL( clicked() ), this, SLOT( slotRemoveAction() ) );
  connect( importButton, SIGNAL( clicked() ), this, SLOT( slotImport() ) );
  connect( exportButton, SIGNAL( clicked() ), this, SLOT( slotExport() ) );
  connect( toClipButton, SIGNAL( clicked() ), this, SLOT( slotToClip() ) );
  connect( fromClipButton, SIGNAL( clicked() ), this, SLOT( slotFromClip() ) );

  // make the first UserActionListViewItem the current item and update the GUI
  actionTree->setFirstActionCurrent();
}

KgUserActions::~KgUserActions() {
}

void KgUserActions::slotChangeAction() {
   //kdDebug() << "entering KgUserActions::slotChangeAction" << endl;
   KrAction* action = actionTree->currentAction();
   if ( action ) {
      actionProperties->setEnabled( true );
      // the discinct name is used as ID it is not allowd to change it afterwards because it is may referenced anywhere else
      actionProperties->leDistinctName->setEnabled( false );
      actionProperties->updateGUI( action );
   }
   else {
      // If the current item in the tree is no action (i.e. a cathegory), disable the properties
      actionProperties->clear();
      actionProperties->setEnabled( false );
   }
}

void KgUserActions::slotUpdateAction() {
  //kdDebug() << "entering KgUserActions::slotUpdateAction" << endl;

  // check that we have a command line, title and a name
   if ( ! actionProperties->validProperties() )
     return;

  if ( actionProperties->leDistinctName->isEnabled() ) {
      // := new entry
      KrAction* action = new KrAction( krApp->actionCollection(), actionProperties->leDistinctName->text().latin1() );
      krUserAction->addKrAction( action );
      actionProperties->updateAction( action );
      UserActionListViewItem* item = actionTree->insertAction( action );
      actionTree->setCurrentItem( item );
      krApp->userMenu->update();
    }
    else { // := edit an existing
       actionProperties->updateAction();
       if ( UserActionListViewItem* item = dynamic_cast<UserActionListViewItem*>(actionTree->currentItem()) )
          actionTree->update( item ); // this is the same as item->update() but also honors category-changes
    }

  _needApply = true;
  emit sigChanged();
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
  actionProperties->leDistinctName->setEnabled( true );
  actionProperties->clear();
}

void KgUserActions::slotRemoveAction() {
   //kdDebug() << "entering KgUserActions::slotRemoveAction" << endl;
   if ( ! dynamic_cast<UserActionListViewItem*>( actionTree->currentItem() ) )
      return;

   int messageDelete = KMessageBox::warningContinueCancel ( this,	//parent
		i18n("Are you sure that you want to remove all selected action?"),	//text
		i18n("Remove selected actions?"), 	//caption
		i18n("Remove"),	//Label for the continue-button
		"Confirm Remove UserAction",	//dontAskAgainName (for the config-file)
		KMessageBox::Dangerous) ;

   if ( messageDelete != KMessageBox::Continue )
      return;

   actionTree->removeSelectedActions();

   _needApply = true;
   emit sigChanged();
}

void KgUserActions::slotImport() {
   //kdDebug() << "entering KgUserActions::slotImport" << endl;

   QString filename = KFileDialog::getOpenFileName(QString::null, i18n(FILE_FILTER), this);
   if ( filename.isEmpty() )
      return;

   UserAction::KrActionList newActions;
   krUserAction->readFromFile( filename, UserAction::renameDoublicated, &newActions );
   for ( KrAction* action = newActions.first(); action; action = newActions.next() )
      actionTree->insertAction( action );

   if ( newActions.count() > 0 ) {
      _needApply = true;
      emit sigChanged();
   }
}

void KgUserActions::slotExport() {
   //kdDebug() << "entering KgUserActions::slotExport" << endl;
   if ( ! dynamic_cast<UserActionListViewItem*>( actionTree->currentItem() ) )
      return;

   QString filename = KFileDialog::getSaveFileName(QString::null, i18n(FILE_FILTER), this);
   if ( filename.isEmpty() )
      return;

   QDomDocument doc = QDomDocument( ACTION_DOCTYPE );
   QFile file( filename );
   int answer = 0;
   if( file.open( IO_ReadOnly ) ) { // getting here, means the file already exists an can be read
      if( doc.setContent( &file ) ) // getting here means the file exists and already contains an UserAction-XML-tree
         answer = KMessageBox::warningYesNoCancel( this,	//parent
         		i18n("This file already contains some useractions.\nDo you want to overwrite it or should it be merged with the selected actions?"),	//text
         		i18n("Overwrite or merge?"),	//caption
         		i18n("Overwrite"),	//label for Yes-Button
         		i18n("Merge")	//label for No-Button
         	);
      file.close();
   }
   if ( answer == 0 && file.exists() )
      answer = KMessageBox::warningContinueCancel( this,	//parent
      		i18n("This file already exists. Do you want to overwrite it?"),	//text
      		i18n("Overwrite existing file?"),	//caption
      		i18n("Overwrite")	//label for Continue-Button
      	);

   if ( answer == KMessageBox::Cancel )
      return;

   if ( answer == KMessageBox::No ) // that means the merge-button
      doc = actionTree->dumpSelectedActions( &doc ); // merge
   else // Yes or Continue means overwrite
      doc = actionTree->dumpSelectedActions();

   bool success = UserAction::writeToFile( doc, filename );
   if ( ! success )
      KMessageBox::error( this,
      		i18n("Can't open %1 for writing!\nNothing exported.").arg(filename),
      		i18n("Export failed!")
      );
}

bool KgUserActions::isChanged() {
   bool changed = KonfiguratorPage::isChanged();

   return changed || _needApply;
}

bool KgUserActions::apply() {

   krUserAction->writeActionFile();

   _needApply = false;
   emit sigChanged();   // nessesary to disable the apply-button again
   return KonfiguratorPage::apply();
}

void KgUserActions::loadInitialValues() {
//FIXME: this doesn't make sence with UserActions => move Useractions to ActionMan

   _needApply = false;
   emit sigChanged();   // nessesary to disable the apply-button again

  KonfiguratorPage::loadInitialValues();
}

void KgUserActions::setDefaults() {
//FIXME: Add a way to disable the default-button as it makes no sence for the useractions or move to ActionMan

   return KonfiguratorPage::setDefaults();
}

void KgUserActions::slotToClip() {
   if ( ! dynamic_cast<UserActionListViewItem*>( actionTree->currentItem() ) )
      return;

   QDomDocument doc = actionTree->dumpSelectedActions();
   KApplication::clipboard()->setText( doc.toString() );
}

void KgUserActions::slotFromClip() {
   QDomDocument doc( ACTION_DOCTYPE );
   if ( doc.setContent( KApplication::clipboard()->text() ) ) {
      QDomElement root = doc.documentElement();
      UserAction::KrActionList newActions;
      krUserAction->readFromElement( root, UserAction::renameDoublicated, &newActions );
      for ( KrAction* action = newActions.first(); action; action = newActions.next() )
         actionTree->insertAction( action );
   }
}

#include "kguseractions.moc"
