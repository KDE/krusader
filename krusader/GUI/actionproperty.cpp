//
// C++ Implementation: actionproperty
//
// Description: 
//
//
// Author: Jonas Bähr (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "actionproperty.h"
#include "addplaceholderpopup.h"

#include "../UserAction/useraction.h"
#include "../UserAction/useractionxml.h"
#include "../UserAction/useractionproperties.h"
#include "../krusader.h"

#include <qtoolbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <kkeybutton.h>
#include <kcombobox.h>
#include <kicondialog.h>
#include <ktextedit.h>


ActionProperty::ActionProperty( QWidget *parent, const char *name, UserActionProperties *prop ) : ActionPropertyBase( parent, name ) {
   if (!prop)
      _properties = new UserActionProperties;
   else
      _properties = prop;
       
   updateGUI( _properties );
   
   // fill with all existing categories
   cbCategory->insertStringList( krUserAction->xml()->getActionCategories() );

   // create the 'add' popup menu
   _popup = new AddPlaceholderPopup( this );
   
   connect( ButtonAddPlaceholder, SIGNAL( clicked() ), this, SLOT( addPlaceholder() ) );
   connect( ButtonAddStartpath, SIGNAL( clicked() ), this, SLOT( addStartpath() ) );
   connect( ButtonNewProtocol, SIGNAL( clicked() ), this, SLOT( newProtocol() ) );
   connect( ButtonEditProtocol, SIGNAL( clicked() ), this, SLOT( editProtocol() ) );
   connect( ButtonRemoveProtocol, SIGNAL( clicked() ), this, SLOT( removeProtocol() ) );
   connect( ButtonAddPath, SIGNAL( clicked() ), this, SLOT( addPath() ) );
   connect( ButtonEditPath, SIGNAL( clicked() ), this, SLOT( editPath() ) );
   connect( ButtonRemovePath, SIGNAL( clicked() ), this, SLOT( removePath() ) );
   connect( ButtonAddMime, SIGNAL( clicked() ), this, SLOT( addMime() ) );
   connect( ButtonEditMime, SIGNAL( clicked() ), this, SLOT( editMime() ) );
   connect( ButtonRemoveMime, SIGNAL( clicked() ), this, SLOT( removeMime() ) );
   connect( ButtonNewFile, SIGNAL( clicked() ), this, SLOT( newFile() ) );
   connect( ButtonEditFile, SIGNAL( clicked() ), this, SLOT( editFile() ) );
   connect( ButtonRemoveFile, SIGNAL( clicked() ), this, SLOT( removeFile() ) );
   connect( ButtonProposeName, SIGNAL( clicked() ), this, SLOT( proposeName() ) );
   
   //these are used to keep the properties-structure up to date
   connect( leDistinctName, SIGNAL( textChanged(const QString&) ), this, SLOT( changedName() ) );
   connect( cbCategory, SIGNAL( textChanged(const QString&) ), this, SLOT( changedCategory() ) );
   connect( ButtonIcon, SIGNAL( iconChanged(QString) ), this, SLOT( changedIcon() ) );
   connect( leTitle, SIGNAL( textChanged(const QString&) ), this, SLOT( changedTitle() ) );
   connect( leTooltip, SIGNAL( textChanged(const QString&) ), this, SLOT( changedTooltip() ) );
   connect( textDescription, SIGNAL( textChanged() ), this, SLOT( changedDescription() ) );
   connect( chkUseTooltip, SIGNAL( toggled(bool) ), this, SLOT( changedChkUseTooltip() ) );
   connect( leCommandline, SIGNAL( textChanged(const QString&) ), this, SLOT( changedCommand() ) );
   connect( leStartpath, SIGNAL( textChanged(const QString&) ), this, SLOT( changedStartpath() ) );
   connect( bgExecType, SIGNAL( clicked(int) ), this, SLOT( changedExecType() ) );
   connect( bgAccept, SIGNAL( clicked(int) ), this, SLOT( changedAccept() ) );
   connect( bgMultiselect, SIGNAL( clicked(int) ), this, SLOT( changedCallEach() ) );
   connect( chkConfirmExecution, SIGNAL( toggled(bool) ), this, SLOT( changedConfirmExecution() ) );
   connect( KeyButtonShortcut, SIGNAL( capturedShortcut(const KShortcut&) ), this, SLOT( changedShortcut(const KShortcut&) ) );
   connect( leDifferentUser, SIGNAL( textChanged(const QString&) ), this, SLOT( changedUser() ) );
   connect( chkDifferentUser, SIGNAL( toggled(bool) ), this, SLOT( changedUser() ) );
}

ActionProperty::~ActionProperty() {
   delete _properties;
}

UserActionProperties* ActionProperty::properties() {
  return _properties;
}

void ActionProperty::updateGUI( UserActionProperties *properties ) {
    if (!properties)
        return;

    leDistinctName->setText( *properties->name() );
    cbCategory->setCurrentText( *properties->category() );
    leTitle->setText( *properties->title() );
    leTooltip->setText( *properties->tooltip() );
    if ( properties->descriptionUseTooltip() ) {
        textDescription->setText( *properties->tooltip() );
        chkUseTooltip->setChecked(true);
    } else {
        textDescription->setText( *properties->description() );
        chkUseTooltip->setChecked(false);
    }
    leCommandline->setText( *properties->command() );
    leStartpath->setText( *properties->startpath() );
    
    lbShowonlyProtocol->clear();
    lbShowonlyProtocol->insertStringList( *properties->showonlyProtocol() );
    lbShowonlyPath->clear();
    lbShowonlyPath->insertStringList( *properties->showonlyPath() );
    lbShowonlyMime->clear();
    lbShowonlyMime->insertStringList( *properties->showonlyMime() );
    lbShowonlyFile->clear();
    lbShowonlyFile->insertStringList( *properties->showonlyFile() );

    chkSeparateStdError->setChecked( properties->separateStderr() );
    if ( properties->execType() == UserActionProperties::Normal )
        radioNormal->setChecked(true);
    else if ( properties->execType() == UserActionProperties::Terminal )
        radioTerminal->setChecked(true);
    else if ( properties->execType() == UserActionProperties::CollectOutput )
        radioCollectOutput->setChecked(true);
    
    if ( properties->acceptURLs() )
        radioUrl->setChecked(true);
    else
        radioLocal->setChecked(true);
    
    chkSeparateCall->setChecked( properties->callEach() );
    chkConfirmExecution->setChecked( properties->confirmExecution() );
    
    if ( ! properties->icon()->isEmpty() )
        ButtonIcon->setIcon( *properties->icon() );
    else
        ButtonIcon->resetIcon();

    leDifferentUser->setText( *properties->user() );
    if ( properties->user()->isEmpty() )
        chkDifferentUser->setChecked(false);
    else
        chkDifferentUser->setChecked(true);

   // these functions are updating the internal _properties
   changedName();
   changedCategory();
   changedIcon();
   changedTitle();
   changedTooltip();
   changedDescription();
   changedChkUseTooltip();
   changedCommand();
   changedStartpath();
   changedUser();
   changedExecType();
   changedAccept();
   changedCallEach();
   changedConfirmExecution();
   changedShowonlyProtocol();
   changedShowonlyPath();
   changedShowonlyMime();
   changedShowonlyFile();
   changedShortcut( *properties->defaultShortcut() );
   //kdDebug() << "Default Shortcut: " << properties->defaultShortcut.toString() << endl;
}

bool ActionProperty::checkName( const QString& name ) {
  //check if name is unique (no existing action with same name = true)
  if ( krUserAction->xml()->nameExists( name ) )
    return false;
  else
    return true;
}

void ActionProperty::proposeName() {
  if ( ! leDistinctName->text().simplifyWhiteSpace().isEmpty() ) {
    //check if given name is unique
    if ( ! checkName( leDistinctName->text() ) ) {
      KMessageBox::error( this, i18n("There already is an action with this name") );
      leDistinctName->setFocus();
      return;
    } else {
      KMessageBox::information( this, i18n("The name is unique") );
      return;
    }
  }
  //TODO: propose unique name
  KMessageBox::sorry( this, "BOFH Excuse #93:\nFeature not yet implemented" );
}


void ActionProperty::addPlaceholder() {
   QString exp = _popup->getPlaceholder( mapToGlobal( QPoint( ButtonAddPlaceholder->pos().x() + ButtonAddPlaceholder->width() * 3 / 2,
                                        ButtonAddPlaceholder->pos().y() + ButtonAddPlaceholder->height() / 2 ) ) );
   leCommandline->insert( exp );
}


void ActionProperty::addStartpath() {
   QString folder = KFileDialog::getExistingDirectory(QString::null, this);
   if (folder != QString::null) leStartpath->setText( folder );
}


void ActionProperty::newProtocol() {
  bool ok;
  QString text = KInputDialog::getText(
		i18n( "New protocol" ),
		i18n( "Set a protocol:" ),
		lbShowonlyProtocol->currentText(),
		&ok, this );
    if ( ok && !text.isEmpty() ) {
      lbShowonlyProtocol->insertStringList( QStringList::split( ";", text ) );
      changedShowonlyProtocol();
    }
}

void ActionProperty::editProtocol() {
  if (lbShowonlyProtocol->currentItem() == -1)
    return;

  bool ok;
  QString text = KInputDialog::getText(
		i18n( "Edit protocol" ),
		i18n( "Set an other protocol:" ),
		lbShowonlyProtocol->currentText(),
		&ok, this );
    if ( ok && !text.isEmpty() ) {
      lbShowonlyProtocol->changeItem( text, lbShowonlyProtocol->currentItem() );
      changedShowonlyProtocol();
    }
}

void ActionProperty::removeProtocol() {
   if (lbShowonlyProtocol->currentItem() != -1) {
     lbShowonlyProtocol->removeItem( lbShowonlyProtocol->currentItem() );
     changedShowonlyProtocol();
   }
}

void ActionProperty::addPath() {
   QString folder = KFileDialog::getExistingDirectory(QString::null, this);
   if (folder != QString::null) {
     lbShowonlyPath->insertItem( folder );
     changedShowonlyPath();
   }
}

void ActionProperty::editPath() {
  if (lbShowonlyPath->currentItem() == -1)
    return;

  bool ok;
  QString text = KInputDialog::getText(
		i18n( "Edit path" ),
		i18n( "Set an other path:" ),
		lbShowonlyPath->currentText(),
		&ok, this );
    if ( ok && !text.isEmpty() ) {
      lbShowonlyPath->changeItem( text, lbShowonlyPath->currentItem() );
      changedShowonlyPath();
    }
}

void ActionProperty::removePath() {
   if (lbShowonlyPath->currentItem() != -1) {
     lbShowonlyPath->removeItem( lbShowonlyPath->currentItem() );
     changedShowonlyPath();
   }
}

void ActionProperty::addMime() { 
  //TODO read from an textfile with each mime-type in a seperate line
  KMessageBox::sorry( this, "sorry, not implemnted yet...\nTODO: Popup-menu with 'text', 'image', 'video', etc...\nand submenus with 'text/*', 'text/plain', 'text/html', etc... " );
}

void ActionProperty::editMime() { 
  if (lbShowonlyMime->currentItem() == -1)
    return;

  bool ok;
  QString text = KInputDialog::getText(
		i18n( "Edit mime-type" ),
		i18n( "Set an other mime-type:" ),
		lbShowonlyMime->currentText(),
		&ok, this );
    if ( ok && !text.isEmpty() ) {
      lbShowonlyMime->changeItem( text, lbShowonlyMime->currentItem() );
      changedShowonlyMime();
    }
}

void ActionProperty::removeMime() { 
   if (lbShowonlyMime->currentItem() != -1) {
     lbShowonlyMime->removeItem( lbShowonlyMime->currentItem() );
     changedShowonlyMime();
   }
}

void ActionProperty::newFile() {
  bool ok;
  QString text = KInputDialog::getText(
		i18n( "New filename" ),
		i18n( "Set a filename:" ),
		lbShowonlyFile->currentText(),
		&ok, this );
    if ( ok && !text.isEmpty() ) {
      lbShowonlyFile->insertStringList( QStringList::split( ";", text ) );
      changedShowonlyFile();
    }
}

void ActionProperty::editFile() {
  if (lbShowonlyFile->currentItem() == -1)
    return;

  bool ok;
  QString text = KInputDialog::getText(
		i18n( "Edit filename" ),
		i18n( "Set an other filename:" ),
		lbShowonlyFile->currentText(),
		&ok, this );
    if ( ok && !text.isEmpty() ) {
      lbShowonlyFile->changeItem( text, lbShowonlyFile->currentItem() );
      changedShowonlyFile();
    }
}

void ActionProperty::removeFile() {
   if (lbShowonlyFile->currentItem() != -1) {
     lbShowonlyFile->removeItem( lbShowonlyFile->currentItem() );
     changedShowonlyFile();
   }
}


bool ActionProperty::checkProperties() {
  if ( leDistinctName->text().simplifyWhiteSpace().isEmpty() ) {
    KMessageBox::error( this, i18n("Please set a unique name for the useraction") );
    leDistinctName->setFocus();
    return false;
  }
  if ( leTitle->text().simplifyWhiteSpace().isEmpty() ) {
    KMessageBox::error( this, i18n("Please set a title for the menu entry") );
    leTitle->setFocus();
    return false;
  }
  if ( leCommandline->text().simplifyWhiteSpace().isEmpty() ) {
    KMessageBox::error( this, i18n("Command line is empty") );
    leCommandline->setFocus();
    return false;
  }
  if ( leDistinctName->isEnabled() )
    if ( !checkName( leDistinctName->text() ) ) {
      KMessageBox::error( this, i18n("There already is an action with this name") );
      leDistinctName->setFocus();
      return false;
    }

  return true;
}


void ActionProperty::changedName() {
  _properties->setName( leDistinctName->text().stripWhiteSpace() );
  if ( leDistinctName->text() != "" ) {
    ButtonProposeName->setText( i18n("Check") );
  }
  else {
    ButtonProposeName->setText( i18n("Propose") );
  }
}
void ActionProperty::changedCategory() {
  _properties->setCategory( cbCategory->currentText().stripWhiteSpace() );
}
void ActionProperty::changedIcon() {
  _properties->setIcon( ButtonIcon->icon() );
}
void ActionProperty::changedTitle() {
  _properties->setTitle( leTitle->text().stripWhiteSpace() );
}
void ActionProperty::changedTooltip() {
  _properties->setTooltip( leTooltip->text().stripWhiteSpace() );
  if ( _properties->descriptionUseTooltip() )
    textDescription->setText( leTooltip->text().stripWhiteSpace() );
}
void ActionProperty::changedDescription() {
  _properties->setDescription( textDescription->text().stripWhiteSpace() );
}
void ActionProperty::changedChkUseTooltip() {
  _properties->setDescriptionUseTooltip( chkUseTooltip->isChecked() );
  if ( _properties->descriptionUseTooltip() )
    textDescription->setText( leTooltip->text().stripWhiteSpace() );
}
void ActionProperty::changedCommand() {
  _properties->setCommand( leCommandline->text().stripWhiteSpace() );
}
void ActionProperty::changedStartpath() {
  _properties->setStartpath( leStartpath->text().stripWhiteSpace() );
}
void ActionProperty::changedUser() {
  if ( chkDifferentUser->isChecked() )
    _properties->setUser( leDifferentUser->text().stripWhiteSpace() );
  else
    _properties->setUser( QString::null );
}
void ActionProperty::changedExecType() {
  _properties->setSeparateStderr( chkSeparateStdError->isChecked() );
  if ( radioNormal->isChecked() )
    _properties->setExecType( UserActionProperties::Normal );
  else  if ( radioTerminal->isChecked() )
    _properties->setExecType( UserActionProperties::Terminal );
  else if ( radioCollectOutput->isChecked() )
    _properties->setExecType( UserActionProperties::CollectOutput );
}
void ActionProperty::changedAccept() {
  if ( radioUrl->isChecked() )
    _properties->setAcceptURLs( true );
  else
    _properties->setAcceptURLs( false );
}
void ActionProperty::changedCallEach() {
  _properties->setCallEach( chkSeparateCall->isChecked() );
}
void ActionProperty::changedConfirmExecution() {
  _properties->setConfirmExecution( chkConfirmExecution->isChecked() );
}
void ActionProperty::changedShowonlyProtocol() {
  _properties->showonlyProtocol()->clear();
  uint count = lbShowonlyProtocol->count();
  for (uint i = 0; i < count; ++i)
    if ( _properties->showonlyProtocol()->find( lbShowonlyProtocol->text(i) ) == _properties->showonlyProtocol()->end() )
      _properties->showonlyProtocol()->append( lbShowonlyProtocol->text(i) );
}
void ActionProperty::changedShowonlyPath() {
  _properties->showonlyPath()->clear();
  uint count = lbShowonlyPath->count();
  for (uint i = 0; i < count; ++i)
    if ( _properties->showonlyPath()->find( lbShowonlyPath->text(i) ) == _properties->showonlyPath()->end() )
      _properties->showonlyPath()->append( lbShowonlyPath->text(i) );
}
void ActionProperty::changedShowonlyMime() {
  _properties->showonlyMime()->clear();
  uint count = lbShowonlyMime->count();
  for (uint i = 0; i < count; ++i)
    if ( _properties->showonlyMime()->find( lbShowonlyMime->text(i) ) == _properties->showonlyMime()->end() )
      _properties->showonlyMime()->append( lbShowonlyMime->text(i) );
}
void ActionProperty::changedShowonlyFile() {
  _properties->showonlyFile()->clear();
  uint count = lbShowonlyFile->count();
  for (uint i = 0; i < count; ++i)
    if ( _properties->showonlyFile()->find( lbShowonlyFile->text(i) ) == _properties->showonlyFile()->end() )
      _properties->showonlyFile()->append( lbShowonlyFile->text(i) );
}

void ActionProperty::changedShortcut( const KShortcut& shortcut ) {
  KeyButtonShortcut->setShortcut( shortcut, false );
  _properties->setDefaultShortcut( shortcut );
}
