//
// C++ Implementation: addplaceholderpopup
//
// Description: 
//
//
// Author: Shie Erlich and Rafi Yanai <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "addplaceholderpopup.h"

#include "../UserAction/expander.h"

#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

// for ParameterDialog
#include "../krusader.h" // for konfig-access
#include "../BookMan/krbookmarkbutton.h"
#include "../GUI/profilemanager.h"

#include <qlayout.h>
#include <q3hbox.h>
#include <qlabel.h>
#include <qtoolbutton.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3Frame>
#include <Q3StrList>
#include <Q3VBoxLayout>
#include <klineedit.h>
#include <qcheckbox.h>
#include <kiconloader.h>
#include <kcombobox.h>
#include <kurlcompletion.h> 
#include <knuminput.h>

#include <kdebug.h>

#define ACTIVE_MASK		0x0100
#define OTHER_MASK		0x0200
#define LEFT_MASK			0x0400
#define RIGHT_MASK			0x0800
#define INDEPENDENT_MASK	0x1000
#define EXECUTABLE_ID		0xFFFF


AddPlaceholderPopup::AddPlaceholderPopup( QWidget *parent ) : KMenu( parent ) {

   _activeSub = new KMenu( this );
   _otherSub = new KMenu( this );
   _leftSub = new KMenu( this );
   _rightSub = new KMenu( this );
   _independentSub = new KMenu( this );

   insertItem( i18n( "Active panel" ), _activeSub );
   insertItem( i18n( "Other panel" ), _otherSub );
   insertItem( i18n( "Left panel" ), _leftSub );
   insertItem( i18n( "Right panel" ), _rightSub );
   insertItem( i18n( "Panel independent" ), _independentSub );
   _independentSub->insertItem( i18n( "Choose executable..." ), EXECUTABLE_ID );
   _independentSub->insertSeparator();

   // read the expressions array from the user menu and populate menus
   Expander expander;
   for ( int i = 0; i < expander.placeholderCount(); ++i ) {
      if (  expander.placeholder( i )->expression().isEmpty() ) {
         if ( expander.placeholder( i )->needPanel() ) {
            _activeSub->insertSeparator();
            _otherSub->insertSeparator();
            _leftSub->insertSeparator();
            _rightSub->insertSeparator();
         }
         else
            _independentSub->insertSeparator();
      }
      else {
         if ( expander.placeholder( i )->needPanel() ) {
            _activeSub->insertItem( i18n( expander.placeholder( i )->description().utf8() ), ( i | ACTIVE_MASK ) );
            _otherSub->insertItem( i18n( expander.placeholder( i )->description().utf8() ), ( i | OTHER_MASK ) );
            _leftSub->insertItem( i18n( expander.placeholder( i )->description().utf8() ), ( i | LEFT_MASK ) );
            _rightSub->insertItem( i18n( expander.placeholder( i )->description().utf8() ), ( i | RIGHT_MASK ) );
         }
         else
            _independentSub->insertItem( i18n( expander.placeholder( i )->description().utf8() ), ( i | INDEPENDENT_MASK ) );
      }
   }

}


QString AddPlaceholderPopup::getPlaceholder( const QPoint& pos ) {
   int res = exec( pos );
   if ( res == -1 )
      return QString();

   // add the selected flag to the command line
   if ( res == EXECUTABLE_ID ) { // did the user need an executable ?
      // select an executable
      QString filename = KFileDialog::getOpenFileName(QString(), QString(), this);
      if (filename != QString())
         return filename + " "; // with extra space
         //return filename; // without extra space
   } else { // user selected something from the menus
      Expander expander;
      const exp_placeholder* currentPlaceholder = expander.placeholder( res & ~( ACTIVE_MASK | OTHER_MASK | LEFT_MASK | RIGHT_MASK | INDEPENDENT_MASK ) );
//       if ( &currentPlaceholder->expFunc == 0 ) {
//          KMessageBox::sorry( this, "BOFH Excuse #93:\nFeature not yet implemented" );
//          return QString();
//       } 
      ParameterDialog* parameterDialog = new ParameterDialog( currentPlaceholder, this );
      QString panel, parameter = parameterDialog->getParameter();
      delete parameterDialog;
      // indicate the panel with 'a' 'o', 'l', 'r' or '_'.
      if ( res & ACTIVE_MASK )
         panel = "a";
      else if ( res & OTHER_MASK )
         panel = "o";
      else if ( res & LEFT_MASK )
         panel = "l";
      else if ( res & RIGHT_MASK )
         panel = "r";
      else if ( res & INDEPENDENT_MASK )
         panel = "_";
      //return "%" + panel + currentPlaceholder->expression() + parameter + "% "; // with extra space
      return "%" + panel + currentPlaceholder->expression() + parameter + "%"; // without extra space
   }
	return QString();
}


////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// ParameterDialog ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ParameterDialog::ParameterDialog( const exp_placeholder* currentPlaceholder, QWidget *parent ) : KDialogBase( Plain, i18n("User Action Parameter Dialog"), Default | Ok, Ok, parent ) {
   _parameter.clear();
   _parameterCount = currentPlaceholder->parameterCount();
   
   Q3VBoxLayout* layout = new Q3VBoxLayout( plainPage() );
   layout->setAutoAdd( true );
   layout->setSpacing( 11 );
   
   new QLabel( i18n("This placeholder allows some parameter:"), plainPage(), "intro" );
   
   for (int i = 0; i < _parameterCount; ++i ) {
      if ( currentPlaceholder->parameter( i ).preset() == "__placeholder" )
         _parameter.append( new ParameterPlaceholder( currentPlaceholder->parameter( i ), plainPage() ) );
      else if ( currentPlaceholder->parameter( i ).preset() == "__yes" )
         _parameter.append( new ParameterYes( currentPlaceholder->parameter( i ), plainPage() ) );
      else if ( currentPlaceholder->parameter( i ).preset() == "__no" )
         _parameter.append( new ParameterNo( currentPlaceholder->parameter( i ), plainPage() ) );
      else if ( currentPlaceholder->parameter( i ).preset() == "__file" )
         _parameter.append( new ParameterFile( currentPlaceholder->parameter( i ), plainPage() ) );
      else if ( currentPlaceholder->parameter( i ).preset().find( "__choose" ) != -1 )
         _parameter.append( new ParameterChoose( currentPlaceholder->parameter( i ), plainPage() ) );
      else if ( currentPlaceholder->parameter( i ).preset() == "__select" )
         _parameter.append( new ParameterSelect( currentPlaceholder->parameter( i ), plainPage() ) );
      else if ( currentPlaceholder->parameter( i ).preset() == "__goto" )
         _parameter.append( new ParameterGoto( currentPlaceholder->parameter( i ), plainPage() ) );
      else if ( currentPlaceholder->parameter( i ).preset() == "__syncprofile" )
         _parameter.append( new ParameterSyncprofile( currentPlaceholder->parameter( i ), plainPage() ) );
      else if ( currentPlaceholder->parameter( i ).preset() == "__searchprofile" )
         _parameter.append( new ParameterSearch( currentPlaceholder->parameter( i ), plainPage() ) );
      else if ( currentPlaceholder->parameter( i ).preset() == "__panelprofile" )
         _parameter.append( new ParameterPanelprofile( currentPlaceholder->parameter( i ), plainPage() ) );
      else if ( currentPlaceholder->parameter( i ).preset().find( "__int" ) != -1 )
         _parameter.append( new ParameterInt( currentPlaceholder->parameter( i ), plainPage() ) );
      else
         _parameter.append( new ParameterText( currentPlaceholder->parameter( i ), plainPage() ) );
   }
   
   QFrame * line = new Q3Frame( plainPage() );
   line->setFrameShape( Q3Frame::HLine );
   line->setFrameShadow( Q3Frame::Sunken );

   connect( this, SIGNAL(defaultClicked()), this, SLOT(reset()) );
}

QString ParameterDialog::getParameter() {
   if ( _parameterCount == 0 ) // meaning no parameters
      return QString();

  if ( exec() == -1 )
     return QString();

  int lastParameter = _parameterCount;
  while ( --lastParameter > -1 ) {
     if ( _parameter[ lastParameter ]->text() != _parameter[ lastParameter ]->preset()  ||  _parameter[ lastParameter ]->nessesary() )
        break;
  }

  if ( lastParameter < 0) // all parameters have default-values
     return QString();

  QString parameter = "(";
  for ( int i = 0; i <= lastParameter; ++i ) {
     if ( i > 0 )
        parameter += ", ";
     parameter += "\"" + _parameter[ i ]->text().replace( "\"", "\\\"" ) + "\"";
  }
  parameter += ")";
  return parameter;
}

void ParameterDialog::reset() {
   for ( int i = 0; i < _parameterCount; ++i )
      _parameter[ i ]->reset();
}

void ParameterDialog::slotOk() {
   bool valid = true;
   for (int i = 0; i < _parameterCount; ++i ) {
      if ( _parameter[ i ]->nessesary() && ! _parameter[ i ]->valid() )
         valid = false;
   }
   
   if ( valid )
      accept();
}

///////////// ParameterText
ParameterText::ParameterText( const exp_parameter& parameter, QWidget* parent ) : ParameterBase( parameter, parent ) {
   Q3VBoxLayout* layout = new Q3VBoxLayout( this );
   layout->setAutoAdd( true );
   layout->setSpacing( 6 );
   
   new QLabel( i18n( parameter.description().utf8() ), this );
   _lineEdit = new KLineEdit( parameter.preset(), this );
   _preset = parameter.preset();
}

QString ParameterText::text() {
   return _lineEdit->text();
} 
QString ParameterText::preset() {
   return _preset;
} 
void ParameterText::reset() {
   _lineEdit->setText( _preset );
} 
bool ParameterText::valid() {
   if ( _lineEdit->text().isEmpty() )
      return false;
   else
      return true;
} 

///////////// ParameterPlaceholder
ParameterPlaceholder::ParameterPlaceholder( const exp_parameter& parameter, QWidget* parent ) : ParameterBase( parameter, parent ) {
   Q3VBoxLayout* layout = new Q3VBoxLayout( this );
   layout->setAutoAdd( true );
   layout->setSpacing( 6 );
   
   new QLabel( i18n( parameter.description().utf8() ), this );
   Q3HBox * hbox = new Q3HBox( this );
   hbox->setSpacing( 6 );
   _lineEdit = new KLineEdit( hbox );
   _button = new QToolButton( hbox);
   _button->setText( i18n("add") );
   connect( _button, SIGNAL(clicked()), this, SLOT(addPlaceholder()) );
}

QString ParameterPlaceholder::text() {
   return _lineEdit->text();
}
QString ParameterPlaceholder::preset() {
   return QString();
} 
void ParameterPlaceholder::reset() {
   _lineEdit->setText( QString() );
} 
bool ParameterPlaceholder::valid() {
   if ( _lineEdit->text().isEmpty() )
      return false;
   else
      return true;
} 
void ParameterPlaceholder::addPlaceholder() {
   AddPlaceholderPopup* popup = new AddPlaceholderPopup( this );
   QString exp = popup->getPlaceholder( mapToGlobal( QPoint( _button->pos().x() + _button->width() + 6, _button->pos().y() + _button->height() / 2 ) ) );
   _lineEdit->insert( exp );
   delete popup;
}

///////////// ParameterYes
ParameterYes::ParameterYes( const exp_parameter& parameter, QWidget* parent ) : ParameterBase( parameter, parent ) {
   Q3VBoxLayout* layout = new Q3VBoxLayout( this );
   layout->setAutoAdd( true );
   layout->setSpacing( 6 );
   
   _checkBox = new QCheckBox( i18n( parameter.description().utf8() ), this );
   _checkBox->setChecked( true );
}

QString ParameterYes::text() {
   if ( _checkBox->isChecked() )
      return QString();
   else
      return "No";
} 
QString ParameterYes::preset() {
   return QString();
} 
void ParameterYes::reset() {
   _checkBox->setChecked( true );
} 
bool ParameterYes::valid() {
   return true;
} 

///////////// ParameterNo
ParameterNo::ParameterNo( const exp_parameter& parameter, QWidget* parent ) : ParameterBase( parameter, parent ) {
   Q3VBoxLayout* layout = new Q3VBoxLayout( this );
   layout->setAutoAdd( true );
   layout->setSpacing( 6 );
   
   _checkBox = new QCheckBox( i18n( parameter.description().utf8() ), this );
   _checkBox->setChecked( false );
}

QString ParameterNo::text() {
   if ( _checkBox->isChecked() )
      return "Yes";
   else
      return QString();
} 
QString ParameterNo::preset() {
   return QString();
} 
void ParameterNo::reset() {
   _checkBox->setChecked( false );
} 
bool ParameterNo::valid() {
   return true;
} 

///////////// ParameterFile
ParameterFile::ParameterFile( const exp_parameter& parameter, QWidget* parent ) : ParameterBase( parameter, parent ) {
   Q3VBoxLayout* layout = new Q3VBoxLayout( this );
   layout->setAutoAdd( true );
   layout->setSpacing( 6 );
   
   new QLabel( i18n( parameter.description().utf8() ), this );
   Q3HBox * hbox = new Q3HBox( this );
   hbox->setSpacing( 6 );
   _lineEdit = new KLineEdit( hbox );
   _button = new QToolButton( hbox);
   KIconLoader *iconLoader = new KIconLoader();
  _button->setPixmap( iconLoader->loadIcon( "fileopen", KIcon::Toolbar, 16 ) );
   connect( _button, SIGNAL(clicked()), this, SLOT(addFile()) );
}

QString ParameterFile::text() {
   return _lineEdit->text();
}
QString ParameterFile::preset() {
   return QString();
} 
void ParameterFile::reset() {
   _lineEdit->setText( QString() );
} 
bool ParameterFile::valid() {
   if ( _lineEdit->text().isEmpty() )
      return false;
   else
      return true;
} 
void ParameterFile::addFile() {
   QString filename = KFileDialog::getOpenFileName(QString(), QString(), this);
   _lineEdit->insert( filename );
}

///////////// ParameterChoose
ParameterChoose::ParameterChoose( const exp_parameter& parameter, QWidget* parent ) : ParameterBase( parameter, parent ) {
   Q3VBoxLayout* layout = new Q3VBoxLayout( this );
   layout->setAutoAdd( true );
   layout->setSpacing( 6 );
   
   new QLabel( i18n( parameter.description().utf8() ), this );
   _combobox = new KComboBox( this );
   _combobox->insertStringList( QStringList::split( ";", parameter.preset().section(":", 1) ) );
}

QString ParameterChoose::text() {
   return _combobox->currentText();
} 
QString ParameterChoose::preset() {
   return _combobox->text( 0 );
} 
void ParameterChoose::reset() {
   _combobox->setCurrentItem( 0 );
} 
bool ParameterChoose::valid() {
      return true;
} 

///////////// ParameterSelect
ParameterSelect::ParameterSelect( const exp_parameter& parameter, QWidget* parent ) : ParameterBase( parameter, parent ) {
   Q3VBoxLayout* layout = new Q3VBoxLayout( this );
   layout->setAutoAdd( true );
   layout->setSpacing( 6 );
   
   new QLabel( i18n( parameter.description().utf8() ), this );
   _combobox = new KComboBox( this );
   _combobox->setEditable( true );
   
   krConfig->setGroup( "Private" );
   Q3StrList lst;
   int i = krConfig->readListEntry( "Predefined Selections", lst );
   if ( i > 0 )
      _combobox->insertStrList( lst );

   _combobox->setCurrentText( "*" );
}

QString ParameterSelect::text() {
   return _combobox->currentText();
} 
QString ParameterSelect::preset() {
   return "*";
} 
void ParameterSelect::reset() {
   _combobox->setCurrentText( "*" );
} 
bool ParameterSelect::valid() {
      return true;
} 

///////////// ParameterGoto
ParameterGoto::ParameterGoto( const exp_parameter& parameter, QWidget* parent ) : ParameterBase( parameter, parent ) {
   Q3VBoxLayout* layout = new Q3VBoxLayout( this );
   layout->setAutoAdd( true );
   layout->setSpacing( 6 );
   
   new QLabel( i18n( parameter.description().utf8() ), this );
   Q3HBox * hbox = new Q3HBox( this );
   hbox->setSpacing( 6 );
   _lineEdit = new KLineEdit( hbox );
   _lineEdit->setCompletionObject( new KUrlCompletion( KUrlCompletion::DirCompletion ) );
   _dirButton = new QToolButton( hbox );
   KIconLoader *iconLoader = new KIconLoader();
  _dirButton->setPixmap( iconLoader->loadIcon( "fileopen", KIcon::Toolbar, 16 ) );
   connect( _dirButton, SIGNAL(clicked()), this, SLOT(setDir()) );
   _placeholderButton = new QToolButton( hbox);
   _placeholderButton->setText( i18n("add") );
   connect( _placeholderButton, SIGNAL(clicked()), this, SLOT(addPlaceholder()) );
}

QString ParameterGoto::text() {
   return _lineEdit->text();
}
QString ParameterGoto::preset() {
   return QString();
} 
void ParameterGoto::reset() {
   _lineEdit->setText( QString() );
} 
bool ParameterGoto::valid() {
   if ( _lineEdit->text().isEmpty() )
      return false;
   else
      return true;
} 
void ParameterGoto::setDir() {
   QString folder = KFileDialog::getExistingDirectory(QString(), this);
   _lineEdit->setText( folder );
}
void ParameterGoto::addPlaceholder() {
   AddPlaceholderPopup* popup = new AddPlaceholderPopup( this );
   QString exp = popup->getPlaceholder( mapToGlobal( QPoint( _placeholderButton->pos().x() + _placeholderButton->width() + 6, _placeholderButton->pos().y() + _placeholderButton->height() / 2 ) ) );
   _lineEdit->insert( exp );
   delete popup;
}

///////////// ParameterSyncprofile
ParameterSyncprofile::ParameterSyncprofile( const exp_parameter& parameter, QWidget* parent ) : ParameterBase( parameter, parent ) {
   Q3VBoxLayout* layout = new Q3VBoxLayout( this );
   layout->setAutoAdd( true );
   layout->setSpacing( 6 );
   
   new QLabel( i18n( parameter.description().utf8() ), this );
   _combobox = new KComboBox( this );
   
   _combobox->insertStringList( ProfileManager::availableProfiles("SynchronizerProfile") );
}

QString ParameterSyncprofile::text() {
   return _combobox->currentText();
} 
QString ParameterSyncprofile::preset() {
   return _combobox->text( 0 );
} 
void ParameterSyncprofile::reset() {
   _combobox->setCurrentItem( 0 );
} 
bool ParameterSyncprofile::valid() {
      return true;
} 

///////////// ParameterSearch
ParameterSearch::ParameterSearch( const exp_parameter& parameter, QWidget* parent ) : ParameterBase( parameter, parent ) {
   Q3VBoxLayout* layout = new Q3VBoxLayout( this );
   layout->setAutoAdd( true );
   layout->setSpacing( 6 );
   
   new QLabel( i18n( parameter.description().utf8() ), this );
   _combobox = new KComboBox( this );
   
   _combobox->insertStringList( ProfileManager::availableProfiles("SearcherProfile") );
}

QString ParameterSearch::text() {
   return _combobox->currentText();
} 
QString ParameterSearch::preset() {
   return _combobox->text( 0 );
} 
void ParameterSearch::reset() {
   _combobox->setCurrentItem( 0 );
} 
bool ParameterSearch::valid() {
      return true;
} 

///////////// ParameterPanelprofile
ParameterPanelprofile::ParameterPanelprofile( const exp_parameter& parameter, QWidget* parent ) : ParameterBase( parameter, parent ) {
   Q3VBoxLayout* layout = new Q3VBoxLayout( this );
   layout->setAutoAdd( true );
   layout->setSpacing( 6 );
   
   new QLabel( i18n( parameter.description().utf8() ), this );
   _combobox = new KComboBox( this );
   
   _combobox->insertStringList( ProfileManager::availableProfiles("Panel") );
}

QString ParameterPanelprofile::text() {
   return _combobox->currentText();
} 
QString ParameterPanelprofile::preset() {
   return _combobox->text( 0 );
} 
void ParameterPanelprofile::reset() {
   _combobox->setCurrentItem( 0 );
} 
bool ParameterPanelprofile::valid() {
      return true;
} 

///////////// ParameterInt
ParameterInt::ParameterInt( const exp_parameter& parameter, QWidget* parent ) : ParameterBase( parameter, parent ) {
   Q3HBoxLayout* layout = new Q3HBoxLayout( this );
   layout->setAutoAdd( true );
   layout->setSpacing( 6 );
   
   new QLabel( i18n( parameter.description().utf8() ), this );
   _spinbox = new KIntSpinBox( this );
   QStringList para = QStringList::split( ";", parameter.preset().section(":", 1) );
   
   _spinbox->setMinValue( para[0].toInt() );
   _spinbox->setMaxValue( para[1].toInt() );
   _spinbox->setLineStep( para[2].toInt() );
   _spinbox->setValue( para[3].toInt() );
   
   _default = _spinbox->value();
}

QString ParameterInt::text() {
   return _spinbox->text();
} 
QString ParameterInt::preset() {
   return QString( "%1" ).arg( _default );
} 
void ParameterInt::reset() {
   return _spinbox->setValue( _default );
} 
bool ParameterInt::valid() {
      return true;
} 


#include "addplaceholderpopup.moc"
