/***************************************************************************
                                 krmaskchoice.cpp
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
#include "krmaskchoice.h"

#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qmessagebox.h>
#include <klocale.h>
#include <qlineedit.h>

/*
 *  Constructs a KRMaskChoice which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
KRMaskChoice::KRMaskChoice( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "KRMaskChoice" );
    resize( 401, 314 );
    setCaption( i18n( "Choose Files"  ) );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5 ) );

    selection = new QComboBox( FALSE, this, "selection" );
    selection->setGeometry( QRect( 12, 48, 377, selection->font().pointSize()*2 + 4) );
    selection->setEditable( TRUE );
    selection->setInsertionPolicy( QComboBox::AtTop );
    selection->setAutoCompletion( TRUE );

    QWidget* Layout7 = new QWidget( this, "Layout7" );
    Layout7->setGeometry( QRect( 10, 10, 380, 30 ) ); 
    hbox = new QHBoxLayout( Layout7 );
    hbox->setSpacing( 6 );
    hbox->setMargin( 0 );

    PixmapLabel1 = new QLabel( Layout7, "PixmapLabel1" );
    PixmapLabel1->setScaledContents( TRUE );
    PixmapLabel1->setMaximumSize( QSize( 31, 31 ) );
	// now, add space for the pixmap    
    hbox->addWidget( PixmapLabel1 );

    label = new QLabel( Layout7, "label" );
    label->setText( i18n( "Select the following files:"  ) );
    hbox->addWidget( label );

    GroupBox1 = new QGroupBox( this, "GroupBox1" );
    GroupBox1->setGeometry( QRect( 11, 77, 379, 190 ) );
    GroupBox1->setTitle( i18n( "Predefined Selections"  ) );

    QWidget* Layout6 = new QWidget( GroupBox1, "Layout6" );
    Layout6->setGeometry( QRect( 10, 20, 360, 160 ) ); 
    hbox_2 = new QHBoxLayout( Layout6 ); 
    hbox_2->setSpacing( 6 );
    hbox_2->setMargin( 0 );

    preSelections = new QListBox( Layout6, "preSelections" );
    preSelections->setVScrollBarMode( QListBox::AlwaysOn );
    QWhatsThis::add(  preSelections, i18n( "A predefined selection is a file-mask which you use often, good\nexamples might be *.c, *.h, *.c *.o etc. You can Add these masks to\nthe list by writing them down and pressing the Add button.\nDelete removes a predefined selection and Clear removes all of them. Notice that the line in which you edit the mask has it's own history, you can roll it down if needed." ) );
    hbox_2->addWidget( preSelections );

    vbox = new QVBoxLayout; 
    vbox->setSpacing( 6 );
    vbox->setMargin( 0 );

    PushButton7 = new QPushButton( Layout6, "PushButton7" );
    PushButton7->setText( i18n( "Add"  ) );
    QToolTip::add(  PushButton7, i18n( "Adds the selection in the line-edit to the list" ) );
    vbox->addWidget( PushButton7 );

    PushButton7_2 = new QPushButton( Layout6, "PushButton7_2" );
    PushButton7_2->setText( i18n( "Delete"  ) );
    QToolTip::add(  PushButton7_2, i18n( "Delete the marked selection from the list" ) );
    vbox->addWidget( PushButton7_2 );

    PushButton7_3 = new QPushButton( Layout6, "PushButton7_3" );
    PushButton7_3->setText( i18n( "Clear"  ) );
    QToolTip::add(  PushButton7_3, i18n( "Clears the entire list of selections" ) );
    vbox->addWidget( PushButton7_3 );
    QSpacerItem* spacer = new QSpacerItem( 20, 54, QSizePolicy::Fixed, QSizePolicy::Expanding );
    vbox->addItem( spacer );
    hbox_2->addLayout( vbox );

    QWidget* Layout18 = new QWidget( this, "Layout18" );
    Layout18->setGeometry( QRect( 10, 280, 379, 30 ) ); 
    hbox_3 = new QHBoxLayout( Layout18 ); 
    hbox_3->setSpacing( 6 );
    hbox_3->setMargin( 0 );
    QSpacerItem* spacer_2 = new QSpacerItem( 205, 20, QSizePolicy::Expanding, QSizePolicy::Fixed );
    hbox_3->addItem( spacer_2 );

    PushButton3 = new QPushButton( Layout18, "PushButton3" );
    PushButton3->setText( i18n( "Ok"  ) );
    hbox_3->addWidget( PushButton3 );

    PushButton3_2 = new QPushButton( Layout18, "PushButton3_2" );
    PushButton3_2->setText( i18n( "Cancel"  ) );
    hbox_3->addWidget( PushButton3_2 );

    // signals and slots connections
    connect( PushButton3_2, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( PushButton3, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( PushButton7, SIGNAL( clicked() ), this, SLOT( addSelection() ) );
    connect( PushButton7_2, SIGNAL( clicked() ), this, SLOT( deleteSelection() ) );
    connect( PushButton7_3, SIGNAL( clicked() ), this, SLOT( clearSelections() ) );
    connect( selection, SIGNAL( activated(const QString&) ), this, SLOT( setEditText(const QString &) ) );
    connect( selection->lineEdit(), SIGNAL( returnPressed() ), this, SLOT( accept() ));
    connect( preSelections, SIGNAL( doubleClicked(QListBoxItem*) ), this, SLOT( acceptFromList(QListBoxItem *) ) );
    connect( preSelections, SIGNAL( highlighted(const QString&) ), selection, SLOT( setEditText(const QString &) ) );
    connect( preSelections, SIGNAL( returnPressed(QListBoxItem*) ), this, SLOT( acceptFromList(QListBoxItem *) ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
KRMaskChoice::~KRMaskChoice()
{
    // no need to delete child widgets, Qt does it all for us
}

void KRMaskChoice::addSelection()
{
    qWarning( "KRMaskChoice::addSelection(): Not implemented yet!" );
}

void KRMaskChoice::clearSelections()
{
    qWarning( "KRMaskChoice::clearSelections(): Not implemented yet!" );
}

void KRMaskChoice::deleteSelection()
{
    qWarning( "KRMaskChoice::deleteSelection(): Not implemented yet!" );
}

void KRMaskChoice::acceptFromList(QListBoxItem *)
{
    qWarning( "KRMaskChoice::acceptFromList(QListBoxItem *): Not implemented yet!" );
}

#include "krmaskchoice.moc"
