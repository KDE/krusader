/***************************************************************************
                                 packguibase.cpp
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
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
#include "packguibase.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>
#include "../krusader.h"

/* 
 *  Constructs a PackGUIBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
PackGUIBase::PackGUIBase( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl ), expanded( false )
{
    if ( !name )
	setName( "PackGUIBase" );
    resize( 430, 140 );
    setCaption( i18n( "Pack" ) );
    grid = new QGridLayout( this );
    grid->setSpacing( 6 );
    grid->setMargin( 11 );

    hbox = new QHBoxLayout;
    hbox->setSpacing( 6 );
    hbox->setMargin( 0 );

    TextLabel3 = new QLabel( this, "TextLabel3" );
    TextLabel3->setText( i18n( "To archive"  ) );
    hbox->addWidget( TextLabel3 );

    nameData = new QLineEdit( this, "nameData" );
    hbox->addWidget( nameData );

    typeData = new QComboBox( FALSE, this, "typeData" );
    typeData->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0 ) );
    connect( typeData, SIGNAL( activated( const QString & ) ), this,  SLOT( checkPasswordConsistency() ) );
    connect( typeData, SIGNAL( highlighted( const QString & ) ), this,  SLOT( checkPasswordConsistency() ) );
    hbox->addWidget( typeData );

    grid->addLayout( hbox, 1, 0 );

    hbox_2 = new QHBoxLayout;
    hbox_2->setSpacing( 6 );
    hbox_2->setMargin( 0 );

    TextLabel5 = new QLabel( this, "TextLabel5" );
    TextLabel5->setText( i18n( "In directory"  ) );
    hbox_2->addWidget( TextLabel5 );

    dirData = new QLineEdit( this, "dirData" );
    hbox_2->addWidget( dirData );

    browseButton = new QToolButton( this, "browseButton" );
    browseButton->setIconSet( SmallIcon( "fileopen" ) );
    hbox_2->addWidget( browseButton );
    QSpacerItem* spacer = new QSpacerItem( 48, 20, QSizePolicy::Fixed, QSizePolicy::Fixed );
    hbox_2->addItem( spacer );

    grid->addLayout( hbox_2, 2, 0 );

    hbox_3 = new QHBoxLayout;
    hbox_3->setSpacing( 6 );
    hbox_3->setMargin( 0 );

    PixmapLabel1 = new QLabel( this, "PixmapLabel1" );
    PixmapLabel1->setPixmap( krLoader->loadIcon("package", KIcon::Desktop, 32) );
    PixmapLabel1->setScaledContents( TRUE );
    PixmapLabel1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0 ) );
    hbox_3->addWidget( PixmapLabel1 );

    TextLabel1 = new QLabel( this, "TextLabel1" );
    TextLabel1->setText( i18n( "Pack"  ) );
    hbox_3->addWidget( TextLabel1 );

    grid->addLayout( hbox_3, 0, 0 );


    hbox_4 = new QHBoxLayout;
    hbox_4->setSpacing( 6 );
    hbox_4->setMargin( 0 );

    QSpacerItem* spacer_3 = new QSpacerItem( 20, 26, QSizePolicy::Fixed, QSizePolicy::Expanding );
    hbox_4->addItem( spacer_3 );
    grid->addLayout( hbox_4, 3, 0 );

    advancedWidget = new QWidget( this, "advancedWidget" );

    grid_5 = new QGridLayout( advancedWidget );
    grid_5->setSpacing( 6 );
    grid_5->setMargin( 0 );

    TextLabel4 = new QLabel( advancedWidget, "TextLabel4" );
    TextLabel4->setText( i18n( "Password"  ) );
    grid_5->addWidget( TextLabel4, 0, 0 );

    password = new QLineEdit( advancedWidget, "password" );
    password->setEchoMode( QLineEdit::Password );
    connect( password, SIGNAL( textChanged ( const QString & ) ), this, SLOT( checkPasswordConsistency() ) );

    grid_5->addWidget( password, 0, 1 );

    TextLabel6 = new QLabel( advancedWidget, "TextLabel6" );
    TextLabel6->setText( i18n( "Password again"  ) );
    grid_5->addWidget( TextLabel6, 1, 0 );

    passwordAgain = new QLineEdit( advancedWidget, "password" );
    passwordAgain->setEchoMode( QLineEdit::Password );
    connect( passwordAgain, SIGNAL( textChanged ( const QString & ) ), this, SLOT( checkPasswordConsistency() ) );

    grid_5->addWidget( passwordAgain, 1, 1 );

    QHBoxLayout * pswHbox = new QHBoxLayout;
    encryptHeaders = new QCheckBox( i18n( "Encrypt headers" ), advancedWidget, "encryptHeaders" );
    pswHbox->addWidget ( encryptHeaders );
    pswHbox->layout()->addItem( new QSpacerItem( 20, 26, QSizePolicy::Expanding, QSizePolicy::Fixed ) );
    passwordConsistencyLabel = new QLabel( advancedWidget, "passwordConsistencyLabel" );
    pswHbox->addWidget ( passwordConsistencyLabel );

    grid_5->addMultiCellLayout( pswHbox, 2, 2, 0, 1 );

    QSpacerItem* spacer_4 = new QSpacerItem( 20, 26, QSizePolicy::Fixed, QSizePolicy::Expanding );
    grid_5->addItem( spacer_4, 3, 0 );

    advancedWidget->hide();
    checkPasswordConsistency();

    grid->addWidget( advancedWidget, 4, 0 );

    hbox_6 = new QHBoxLayout;
    hbox_6->setSpacing( 6 );
    hbox_6->setMargin( 0 );

    advancedButton = new QPushButton( this, "advancedButton" );
    advancedButton->setText( i18n( "&Advanced" ) + " >>" );
    hbox_6->addWidget( advancedButton );

    QSpacerItem* spacer_2 = new QSpacerItem( 140, 20, QSizePolicy::Expanding, QSizePolicy::Fixed );
    hbox_6->addItem( spacer_2 );

    okButton = new QPushButton( this, "okButton" );
    okButton->setText( i18n( "Ok"  ) );
    hbox_6->addWidget( okButton );

    cancelButton = new QPushButton( this, "cancelButton" );
    cancelButton->setText( i18n( "Cancel"  ) );
    hbox_6->addWidget( cancelButton );

    grid->addLayout( hbox_6, 5, 0 );

    // signals and slots connections
    connect( okButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( advancedButton, SIGNAL( clicked() ), this, SLOT( expand() ) );
    connect( cancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( browseButton, SIGNAL( clicked() ), this, SLOT( browse() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
PackGUIBase::~PackGUIBase()
{
    // no need to delete child widgets, Qt does it all for us
}

void PackGUIBase::browse()
{
    qWarning( "PackGUIBase::browse(): Not implemented yet!" );
}

void PackGUIBase::expand() {
    expanded = !expanded;
    
    advancedButton->setText( i18n( "&Advanced" ) + ( expanded ?  " <<" : " >>" ) );

    if( expanded )
      advancedWidget->show();
    else {
      advancedWidget->hide();
      layout()->activate();
      QSize minSize = minimumSize();
      resize( width(), minSize.height() );
    }
    show();
}

void PackGUIBase::checkPasswordConsistency() {
    if( password->text() == passwordAgain->text() ) {
      passwordConsistencyLabel->setPaletteForegroundColor( KGlobalSettings::textColor() );
      passwordConsistencyLabel->setText( i18n( "The passwords are equal" ) );
    }
    else {
      passwordConsistencyLabel->setPaletteForegroundColor( Qt::red );
      passwordConsistencyLabel->setText( i18n( "The passwords are different" ) );
    }

    QString packer = typeData->currentText();

    bool passworded = false;
    if( packer == "7z" || packer == "rar" || packer == "zip" || packer == "arj" )
      passworded = true;

    passwordConsistencyLabel->setEnabled( passworded );
    password->setEnabled( passworded );
    passwordAgain->setEnabled( passworded );
    TextLabel4->setEnabled( passworded );
    TextLabel6->setEnabled( passworded );

    encryptHeaders->setEnabled( packer == "rar" );
}

bool PackGUIBase::extraProperties( QMap<QString,QString> & inMap ) {
    inMap.clear();

    if( password->isEnabled() && passwordAgain->isEnabled() ) {
      if( password->text() != passwordAgain->text() ) {
        KMessageBox::error( this, i18n( "Cannot pack! The passwords are different!" ) );
        return false;
      }

      if( !password->text().isEmpty() ) {
        inMap[ "Password" ] = password->text();

        if( encryptHeaders->isEnabled() && encryptHeaders->isChecked() )
          inMap[ "EncryptHeaders" ] = "1";
      }
    }
    return true;
}

#include "packguibase.moc"
