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
#include <q3listbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qmessagebox.h>
//Added by qt3to4:
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <klocale.h>
#include <qlineedit.h>

/*
 *  Constructs a KRMaskChoice which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
KRMaskChoice::KRMaskChoice( QWidget* parent )
    : QDialog( parent )
{
    setModal( true );
    resize( 401, 314 );
    setCaption( i18n( "Choose Files"  ) );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5 ) );

    selection = new QComboBox( FALSE, this );
    int height = QFontMetrics( selection->font() ).height();
    height =  height + 5*(height > 14) + 6;
    selection->setGeometry( QRect( 12, 48, 377, height) );
    selection->setEditable( TRUE );
    selection->setInsertPolicy( QComboBox::InsertAtTop );
    selection->setAutoCompletion( TRUE );

    QWidget* Layout7 = new QWidget( this );
    Layout7->setGeometry( QRect( 10, 10, 380, 30 ) ); 
    hbox = new QHBoxLayout( Layout7 );
    hbox->setSpacing( 6 );
    hbox->setContentsMargins( 0, 0, 0, 0 );

    PixmapLabel1 = new QLabel( Layout7 );
    PixmapLabel1->setScaledContents( TRUE );
    PixmapLabel1->setMaximumSize( QSize( 31, 31 ) );
	// now, add space for the pixmap    
    hbox->addWidget( PixmapLabel1 );

    label = new QLabel( Layout7 );
    label->setText( i18n( "Select the following files:"  ) );
    hbox->addWidget( label );

    GroupBox1 = new QGroupBox( this );
    GroupBox1->setGeometry( QRect( 11, 77, 379, 190 ) );
    GroupBox1->setTitle( i18n( "Predefined Selections"  ) );
    QHBoxLayout * gbLayout = new QHBoxLayout( GroupBox1 );

    QWidget* Layout6 = new QWidget( GroupBox1 );
    gbLayout->addWidget( Layout6 );

    Layout6->setGeometry( QRect( 10, 20, 360, 160 ) ); 
    hbox_2 = new QHBoxLayout( Layout6 ); 
    hbox_2->setSpacing( 6 );
    hbox_2->setContentsMargins( 0, 0, 0, 0 );

    preSelections = new Q3ListBox( Layout6 );
    preSelections->setVScrollBarMode( Q3ListBox::AlwaysOn );
    preSelections->setWhatsThis( i18n( "A predefined selection is a file-mask which you use often.\nSome examples are: \"*.c, *.h\", \"*.c, *.o\", etc.\nYou can add these masks to the list by typing them and pressing the Add button.\nDelete removes a predefined selection and Clear removes all of them.\nNotice that the line in which you edit the mask has it's own history, you can scroll it, if needed." ) );
    hbox_2->addWidget( preSelections );

    vbox = new QVBoxLayout; 
    vbox->setSpacing( 6 );
    vbox->setContentsMargins( 0, 0, 0, 0 );

    PushButton7 = new QPushButton( Layout6 );
    PushButton7->setText( i18n( "Add"  ) );
    QToolTip::add(  PushButton7, i18n( "Adds the selection in the line-edit to the list" ) );
    vbox->addWidget( PushButton7 );

    PushButton7_2 = new QPushButton( Layout6 );
    PushButton7_2->setText( i18n( "Delete"  ) );
    QToolTip::add(  PushButton7_2, i18n( "Delete the marked selection from the list" ) );
    vbox->addWidget( PushButton7_2 );

    PushButton7_3 = new QPushButton( Layout6 );
    PushButton7_3->setText( i18n( "Clear"  ) );
    QToolTip::add(  PushButton7_3, i18n( "Clears the entire list of selections" ) );
    vbox->addWidget( PushButton7_3 );
    QSpacerItem* spacer = new QSpacerItem( 20, 54, QSizePolicy::Fixed, QSizePolicy::Expanding );
    vbox->addItem( spacer );
    hbox_2->addLayout( vbox );

    QWidget* Layout18 = new QWidget( this );
    Layout18->setGeometry( QRect( 10, 280, 379, 30 ) ); 
    hbox_3 = new QHBoxLayout( Layout18 ); 
    hbox_3->setSpacing( 6 );
    hbox_3->setContentsMargins( 0, 0, 0, 0 );
    QSpacerItem* spacer_2 = new QSpacerItem( 205, 20, QSizePolicy::Expanding, QSizePolicy::Fixed );
    hbox_3->addItem( spacer_2 );

    PushButton3 = new QPushButton( Layout18 );
    PushButton3->setText( i18n( "OK"  ) );
    hbox_3->addWidget( PushButton3 );

    PushButton3_2 = new QPushButton( Layout18 );
    PushButton3_2->setText( i18n( "Cancel"  ) );
    hbox_3->addWidget( PushButton3_2 );

    // signals and slots connections
    connect( PushButton3_2, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( PushButton3, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( PushButton7, SIGNAL( clicked() ), this, SLOT( addSelection() ) );
    connect( PushButton7_2, SIGNAL( clicked() ), this, SLOT( deleteSelection() ) );
    connect( PushButton7_3, SIGNAL( clicked() ), this, SLOT( clearSelections() ) );
    connect( selection, SIGNAL( activated(const QString&) ), selection, SLOT( setEditText(const QString &) ) );
    connect( selection->lineEdit(), SIGNAL( returnPressed() ), this, SLOT( accept() ));
    connect( preSelections, SIGNAL( doubleClicked(Q3ListBoxItem*) ), this, SLOT( acceptFromList(Q3ListBoxItem *) ) );
    connect( preSelections, SIGNAL( highlighted(const QString&) ), selection, SLOT( setEditText(const QString &) ) );
    connect( preSelections, SIGNAL( returnPressed(Q3ListBoxItem*) ), this, SLOT( acceptFromList(Q3ListBoxItem *) ) );
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

void KRMaskChoice::acceptFromList(Q3ListBoxItem *)
{
    qWarning( "KRMaskChoice::acceptFromList(QListBoxItem *): Not implemented yet!" );
}

#include "krmaskchoice.moc"
