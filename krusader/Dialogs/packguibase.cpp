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
#include "../krusader.h"

/* 
 *  Constructs a PackGUIBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
PackGUIBase::PackGUIBase( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
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
    typeData->insertItem( i18n( "tar.bz2" ) );
    typeData->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0 ) );
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
    QSpacerItem* spacer_2 = new QSpacerItem( 140, 20, QSizePolicy::Expanding, QSizePolicy::Fixed );
    hbox_4->addItem( spacer_2 );

    okButton = new QPushButton( this, "okButton" );
    okButton->setText( i18n( "Ok"  ) );
    hbox_4->addWidget( okButton );

    cancelButton = new QPushButton( this, "cancelButton" );
    cancelButton->setText( i18n( "Cancel"  ) );
    hbox_4->addWidget( cancelButton );

    grid->addLayout( hbox_4, 4, 0 );

    hbox_5 = new QHBoxLayout;
    hbox_5->setSpacing( 6 );
    hbox_5->setMargin( 0 );

/*  moveCheckbox = new QCheckBox( this, "moveCheckbox" );
    moveCheckbox->setText( i18n( "Move into archive"  ) );
    hbox_5->addWidget( moveCheckbox );*/
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 26, QSizePolicy::Fixed, QSizePolicy::Expanding );
    hbox_5->addItem( spacer_3 );

    grid->addLayout( hbox_5, 3, 0 );

    // signals and slots connections
    connect( okButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
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

#include "packguibase.moc"
