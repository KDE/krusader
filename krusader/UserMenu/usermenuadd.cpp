//
// C++ Implementation: usermenuadd
//
// Description: 
//
//
// Author: Shie Erlich and Rafi Yanai <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <klocale.h>

#include "usermenuadd.h"

#include "../GUI/actionproperty.h"

#include <qvariant.h>
#include <kpushbutton.h>
#include <qlayout.h>
#include <kdebug.h>

UserMenuAdd::UserMenuAdd( QWidget* parent, const char* name, bool modal, WFlags fl ) : QDialog( parent, name, modal, fl )
{
    if ( !name )
        setName( "UserMenuAdd" );

    setCaption( i18n( "Add an useraction" ) );
    
    QGridLayout *UserMenuAddLayout = new QGridLayout( this, 1, 1, 11, 6, "UserMenuAddLayout"); 
    
    actionProperties = new ActionProperty(this);
    UserMenuAddLayout->addWidget( actionProperties, 0, 0 );
    
    QHBoxLayout *hboxButtons = new QHBoxLayout( this, 0, 6, "hboxButtons"); 
    QSpacerItem* spacer = new QSpacerItem( 350, 30, QSizePolicy::Expanding, QSizePolicy::Minimum );
    hboxButtons->addItem( spacer );

    okButton = new KPushButton( this, "okButton" );
    okButton->setText( i18n( "Ok" ) );
    okButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, okButton->sizePolicy().hasHeightForWidth() ) );
    hboxButtons->addWidget( okButton );

    cancelButton = new KPushButton( this, "cancelButton" );
    cancelButton->setText( i18n( "Cancel" ) );
    cancelButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, cancelButton->sizePolicy().hasHeightForWidth() ) );
    hboxButtons->addWidget( cancelButton );

    UserMenuAddLayout->addLayout( hboxButtons, 1, 0 );
    
    resize( QSize(450,500).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( cancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( okButton, SIGNAL( clicked() ), this, SLOT( check() ) );

}

UserMenuAdd::~UserMenuAdd() {}

void UserMenuAdd::check() {
   // check that we have a command line and a name
   if ( ! actionProperties->checkProperties() )
     return;
     
  //kdDebug() << "UserMenuAdd::check() emit newEntry; actionname: " << actionProperties->properties->name << endl;
  emit newEntry( actionProperties->properties() );
   
   UserMenuAdd::accept();
}

#include "usermenuadd.moc"
