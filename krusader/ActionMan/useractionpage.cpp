//
// C++ Implementation: useractionpage
//
// Description: 
//
//
// Author: Shie Erlich and Rafi Yanai <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "useractionpage.h"

#include <kstandardguiitem.h>
#include <qsplitter.h>
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <klocale.h>

#include "actionproperty.h"
#include "useractionlistview.h"
#include "../UserMenu/usermenu.h" //FIXME this should not be needed here!
#include "../UserAction/useraction.h"
#include "../UserAction/kraction.h"
#include "../krusader.h"

#define ICON(N)		KIconLoader::global()->loadIcon(N, K3Icon::Toolbar)
//This is the filter in the KFileDialog of Import/Export:
static const char* FILE_FILTER = I18N_NOOP("*.xml|xml-files\n*|all files");


UserActionPage::UserActionPage( QWidget* parent )
 : QWidget( parent, "UserActionPage" )
{
   Q3VBoxLayout* layout = new Q3VBoxLayout( this, 0, 6, "UserActionPageLayout" ); // 0px margin, 6px item-spacing

   // ======== pseudo-toolbar start ========
   Q3HBoxLayout* toolbarLayout = new Q3HBoxLayout( layout, 0, 0 ); // neither margin nor spacing for the toolbar with autoRaise

   newButton = new QToolButton( this, "newButton" );
   newButton->setPixmap( ICON("filenew") );
   newButton->setAutoRaise(true);
   QToolTip::add( newButton, i18n("Create new useraction") );

   importButton = new QToolButton( this, "importButton" );
   importButton->setPixmap( ICON("fileimport") );
   importButton->setAutoRaise(true);
   QToolTip::add( importButton, i18n("Import useractions") );

   exportButton = new QToolButton( this, "exportButton" );
   exportButton->setPixmap( ICON("fileexport") );
   exportButton->setAutoRaise(true);
   QToolTip::add( exportButton, i18n("Export useractions") );

   copyButton = new QToolButton( this, "copyButton" );
   copyButton->setPixmap( ICON("editcopy") );
   copyButton->setAutoRaise(true);
   QToolTip::add( copyButton, i18n("Copy useractions to clipboard") );

   pasteButton = new QToolButton( this, "pasteButton" );
   pasteButton->setPixmap( ICON("editpaste") );
   pasteButton->setAutoRaise(true);
   QToolTip::add( pasteButton, i18n("Paste useractions from clipboard") );

   removeButton = new QToolButton( this, "removeButton" );
   removeButton->setPixmap( ICON("editdelete") );
   removeButton->setAutoRaise(true);
   QToolTip::add( removeButton, i18n("Delete selected useractions") );

   toolbarLayout->addWidget( newButton );
   toolbarLayout->addWidget( importButton );
   toolbarLayout->addWidget( exportButton );
   toolbarLayout->addWidget( copyButton );
   toolbarLayout->addWidget( pasteButton );
   toolbarLayout->addSpacing( 6 ); // 6 pixel nothing
   toolbarLayout->addWidget( removeButton );
   toolbarLayout->addStretch( 1000 ); // some very large stretch-factor
   // ======== pseudo-toolbar end ========
/* This seems obsolete now!
   // Display some help
   KMessageBox::information( this,	// parent
   		i18n( "When you apply changes to an action, the modifications "
   			"become available in the current session immediately.\n"
   			"When closing ActionMan, you will be asked to save the changes permanently."
   		),
  		QString(),	// caption
  		"show UserAction help"	//dontShowAgainName for the config
  	);
*/
   QSplitter *split = new QSplitter( this, "useractionpage splitter");
   layout->addWidget( split, 1000 ); // again a very large stretch-factor to fix the height of the toolbar

   actionTree = new UserActionListView( split, "actionTree" );
   actionProperties = new ActionProperty( split, "actionProperties" );
   actionProperties->setEnabled( false ); // if there are any actions in the list, the first is displayed and this widget is enabled

   connect(  actionTree, SIGNAL( currentChanged(Q3ListViewItem*) ), SLOT( slotChangeCurrent() ) );
   connect( newButton, SIGNAL( clicked() ), SLOT( slotNewAction() ) );
   connect( removeButton, SIGNAL( clicked() ), SLOT( slotRemoveAction() ) );
   connect( importButton, SIGNAL( clicked() ), SLOT( slotImport() ) );
   connect( exportButton, SIGNAL( clicked() ), SLOT( slotExport() ) );
   connect( copyButton, SIGNAL( clicked() ), SLOT( slotToClip() ) );
   connect( pasteButton, SIGNAL( clicked() ), SLOT( slotFromClip() ) );

   // forwards the changed signal of the properties
   connect ( actionProperties, SIGNAL( changed() ), SIGNAL( changed() ) );

   actionTree->setFirstActionCurrent();
   actionTree->setFocus();
}

UserActionPage::~UserActionPage()
{
}

bool UserActionPage::continueInSpiteOfChanges() {
   if ( ! actionProperties->isModified() )
      return true;

   int answer = KMessageBox::questionYesNoCancel( this,
   		i18n("The current action has been modified. Do you want to apply these changes?")
   	);
   if ( answer == KMessageBox::Cancel ) {
      disconnect(  actionTree, SIGNAL( currentChanged(Q3ListViewItem*) ), this, SLOT( slotChangeCurrent() ) );
      actionTree->setCurrentAction( actionProperties->action() );
      connect(  actionTree, SIGNAL( currentChanged(Q3ListViewItem*) ), SLOT( slotChangeCurrent() ) );
      return false;
   }
   if ( answer == KMessageBox::Yes ) {
      if ( ! actionProperties->validProperties() ) {
         disconnect(  actionTree, SIGNAL( currentChanged(Q3ListViewItem*) ), this, SLOT( slotChangeCurrent() ) );
         actionTree->setCurrentAction( actionProperties->action() );
         connect(  actionTree, SIGNAL( currentChanged(Q3ListViewItem*) ), SLOT( slotChangeCurrent() ) );
         return false;
      }
      slotUpdateAction();
   } // if Yes
   return true;
}

void UserActionPage::slotChangeCurrent() {
   if ( ! continueInSpiteOfChanges() )
      return;

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
   emit applied(); // to disable the apply-button
}


void UserActionPage::slotUpdateAction() {
  // check that we have a command line, title and a name
   if ( ! actionProperties->validProperties() )
     return;

  if ( actionProperties->leDistinctName->isEnabled() ) {
      // := new entry
      KrAction* action = new KrAction( krApp->actionCollection(), actionProperties->leDistinctName->text().toLatin1() );
      krUserAction->addKrAction( action );
      actionProperties->updateAction( action );
      UserActionListViewItem* item = actionTree->insertAction( action );
      actionTree->setCurrentItem( item );
      krApp->userMenu->update();
    }
    else { // := edit an existing
       actionProperties->updateAction();
       actionTree->update( actionProperties->action() ); // update the listviewitem as well...
    }
   apply();
}


void UserActionPage::slotNewAction() {
   if ( continueInSpiteOfChanges() ) {
      actionTree->clearSelection();  // else the user may think that he is overwriting the selected action
      actionProperties->clear();
      actionProperties->setEnabled( true ); // it may be disabled because the tree has the focus on a category
      actionProperties->leDistinctName->setEnabled( true );
      actionProperties->leDistinctName->setFocus();
   }
}

void UserActionPage::slotRemoveAction() {
   if ( ! dynamic_cast<UserActionListViewItem*>( actionTree->currentItem() ) )
      return;

   int messageDelete = KMessageBox::warningContinueCancel ( this,	//parent
		i18n("Are you sure that you want to remove all selected actions?"),	//text
		i18n("Remove selected actions?"), 	//caption
		KStandardGuiItem::remove(),	//Label for the continue-button
		KStandardGuiItem::cancel(), 
		"Confirm Remove UserAction",	//dontAskAgainName (for the config-file)
		KMessageBox::Dangerous | KMessageBox::Notify) ;

   if ( messageDelete != KMessageBox::Continue )
      return;

   actionTree->removeSelectedActions();

   apply();
}

void UserActionPage::slotImport() {
   QString filename = KFileDialog::getOpenFileName(QString(), i18n(FILE_FILTER), this);
   if ( filename.isEmpty() )
      return;

   UserAction::KrActionList newActions;
   krUserAction->readFromFile( filename, UserAction::renameDoublicated, &newActions );
   for ( KrAction* action = newActions.first(); action; action = newActions.next() )
      actionTree->insertAction( action );

   if ( newActions.count() > 0 ) {
      apply();
   }
}

void UserActionPage::slotExport() {
   if ( ! dynamic_cast<UserActionListViewItem*>( actionTree->currentItem() ) )
      return;

   QString filename = KFileDialog::getSaveFileName(QString(), i18n(FILE_FILTER), this);
   if ( filename.isEmpty() )
      return;

   QDomDocument doc = QDomDocument( ACTION_DOCTYPE );
   QFile file( filename );
   int answer = 0;
   if( file.open( QIODevice::ReadOnly ) ) { // getting here, means the file already exists an can be read
      if( doc.setContent( &file ) ) // getting here means the file exists and already contains an UserAction-XML-tree
         answer = KMessageBox::warningYesNoCancel( this,	//parent
         		i18n("This file already contains some useractions.\nDo you want to overwrite it or should it be merged with the selected actions?"),	//text
         		i18n("Overwrite or merge?"),	//caption
         		KStandardGuiItem::overwrite(),	//label for Yes-Button
         		KGuiItem(i18n("Merge"))	//label for No-Button
         	);
      file.close();
   }
   if ( answer == 0 && file.exists() )
      answer = KMessageBox::warningContinueCancel( this,	//parent
      		i18n("This file already exists. Do you want to overwrite it?"),	//text
      		i18n("Overwrite existing file?"),	//caption
      		KStandardGuiItem::overwrite()	//label for Continue-Button
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

/*
void UserActionPage::slotToClip() {
   if ( ! dynamic_cast<UserActionListViewItem*>( actionTree->currentItem() ) )
      return;

   QDomDocument doc = actionTree->dumpSelectedActions();
   KApplication::clipboard()->setText( doc.toString() );
}
*/

/*
void UserActionPage::slotFromClip() {
   QDomDocument doc( ACTION_DOCTYPE );
   if ( doc.setContent( KApplication::clipboard()->text() ) ) {
      QDomElement root = doc.documentElement();
      UserAction::KrActionList newActions;
      krUserAction->readFromElement( root, UserAction::renameDoublicated, &newActions );
      for ( KrAction* action = newActions.first(); action; action = newActions.next() )
         actionTree->insertAction( action );
      if ( newActions.count() > 0 ) {
         apply();
      }
   } // if ( doc.setContent )
}
*/

bool UserActionPage::readyToQuit() {
   // Check if the current UserAction has changed
   if ( ! continueInSpiteOfChanges() )
      return false;

   krUserAction->writeActionFile();
   return true;
}

void UserActionPage::apply() {
   krUserAction->writeActionFile();
   emit applied();
}

void UserActionPage::applyChanges() {
   slotUpdateAction();
}


#include "useractionpage.moc"
