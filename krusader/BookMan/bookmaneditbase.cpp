/***************************************************************************
                                bookmaneditbase.cpp
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


/****************************************************************************
** Form implementation generated from reading ui file 'bookmaneditbase.ui'
**
** Created: Fri Aug 11 18:23:00 2000
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "bookmaneditbase.h"

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
#include <klocale.h>

static const char* const image0_data[] = { 
"22 22 10 1",
". c None",
"e c #ff8183",
"# c #b4aeff",
"b c #000000",
"a c #000083",
"h c #830000",
"g c #ff0000",
"c c #acaeff",
"f c #ffffff",
"d c #0000ff",
"................#ab...",
"...............cddab..",
"..............#ddddab.",
".............cddddddab",
"............#dddddddda",
"...........cddddddddda",
"..........#dddddddddab",
".........cdddddddddab.",
"........#dddddddddab..",
".......effdddddddab...",
"......eggffdddddab....",
".....eggggffdddab.....",
"....eggggggffdab......",
"...eggggggggfab.......",
"..eggggggggghb........",
".eegggggggghb.........",
"eeeegggggghb..........",
"eeeeegggghb...........",
".eeeeegghb............",
"..eeeeehb.............",
"...eeehb..............",
"....ehb..............."};

static const char* const image1_data[] = { 
"22 22 159 2",
".a c None",
".t c None",
"#I c #838183",
"#L c #ffb26a",
"aC c #292429",
"#Q c #bdbab4",
"ak c #b4aeb4",
"#d c #392018",
"#D c #bdbabd",
"az c #312c29",
"av c #949594",
"#. c #835529",
".N c #412818",
".I c #ffc683",
"as c #312c31",
".1 c #ffc68b",
"#t c #c5c2c5",
"ar c #9c9d9c",
"#y c #393439",
"#8 c #cdcac5",
".k c None",
".g c None",
"#s c #cdcacd",
".h c None",
"#R c #a4a5a4",
"#3 c #736d6a",
"#r c #d5d2cd",
".M c None",
".6 c None",
".f c None",
"#X c #d5d2d5",
".q c None",
".O c #eea56a",
"#i c #ac6d39",
"#p c #dedede",
"Qt c None",
"#N c #52504a",
"#j c #525052",
".7 c None",
"#B c #e6e6e6",
".5 c #ffae62",
"#P c #eeeeee",
"#9 c #bdb6b4",
".J c #ffc27b",
"al c #bdb6bd",
"ae c #949194",
"#a c #4a3018",
"#k c #f6f6f6",
".3 c #ffc283",
"ap c #c5bebd",
"#1 c #c5bec5",
"a. c #9c999c",
".Q c #ffd69c",
"au c #a4a19c",
"ad c #cdc6cd",
".8 c #ffd6a4",
"#F c #a4a1a4",
"ao c #73696a",
"ab c #d5cecd",
"#7 c #dedad5",
"ai c #d5ced5",
".E c None",
"ax c #101410",
"#q c #dedade",
".x c None",
".C c #291c10",
"#O c #e6e2e6",
".d c None",
".0 c None",
"aB c #202420",
"#n c #eeeaee",
"#H c #948d8b",
"#2 c #948d94",
"#c c #4a2c18",
"#m c #f6f2f6",
".z c None",
".v c None",
"#W c #523418",
".l c #000000",
"#G c #9c959c",
".2 c #ffd294",
".o c None",
"## c #523420",
".R c #ffd29c",
"#S c #a49d9c",
"at c #a49da4",
".X c #101008",
"#z c #f6a55a",
"#Z c #ded6d5",
".w c None",
"ay c #101010",
".m c None",
".c c None",
".A c None",
".s c None",
"ac c #e6dede",
"#K c #181818",
"ah c #bd7941",
".D c #291810",
".e c None",
"#6 c #e6dee6",
".Y c None",
"#5 c #5a5052",
"#x c #acaeac",
"#Y c #eee6e6",
".H c #f6ae6a",
".U c #392810",
"#o c #eee6ee",
"aA c #292c29",
".K c #ffba73",
"#v c #b4b6b4",
".G c #392818",
".T c #ffba7b",
"#V c #313431",
".y c None",
"#l c #fffaff",
".r c None",
"#b c #523018",
".i c None",
"#A c #523020",
"#J c #626162",
"an c #080400",
".P c #ffce9c",
"am c #6a696a",
".b c None",
"#4 c #a46d39",
".9 c #f6a15a",
".B c None",
"af c #181410",
"aa c None",
"#U c #7b7d7b",
".Z c None",
".# c None",
"aq c #acaaa4",
"#E c #acaaac",
"#e c #311c10",
".4 c #ffb66a",
".L c #392410",
"a# c #292829",
"#h c #ffb673",
"#w c #b4b2b4",
"#T c #8b8d8b",
"#u c #bdbebd",
".n c None",
".S c #ffca8b",
"aw c #625d5a",
"#0 c #c5c6c5",
"aj c #393831",
"#g c #ffca94",
".p c None",
"ag c None",
"#C c #d5d6d5",
".F c None",
"#M c #ac7139",
".W c #181008",
".V c #181010",
".j c None",
"#f c #201808",
".u c None",
"Qt.#Qt.#.a.a.a.a.a.a.b.a.c.d.a.d.e.a.a.f.g.a",
".h.a.a.i.j.k.a.a.a.l.l.l.l.l.a.a.m.n.#.a.o.k",
".a.a.a.a.a.a.a.l.l.l.l.l.l.l.l.l.p.l.a.a.a.a",
".q.g.p.a.r.d.l.l.a.a.a.a.a.a.l.l.l.l.a.#.a.d",
"Qt.a.s.a.a.lQt.#.t.a.u.a.e.e.a.l.l.l.a.#.a.a",
".a.v.w.a.x.a.a.a.a.v.y.z.p.A.l.l.l.l.a.B.a.A",
".a.a.C.C.C.D.a.a.E.#.E.#.a.a.a.a.a.a.a.a.a.F",
".a.G.H.I.J.K.L.M.a.a.a.a.a.a.a.a.a.a.a.a.a.a",
".N.O.P.Q.R.S.T.U.V.W.W.X.V.W.W.X.Y.Z.0.a.a.a",
".L.1.Q.2.3.4.5.5.5.5.5.5.5.5.5.5.L.a.6.B.7.k",
".N.S.8.3.9#.###a#b#a#c.N.N.L#d#e.C#f.l.l.l.l",
".L.S#g#h#i#j#k#l#k#m#n#o#p#q#r#s#t#u#v#w#x#y",
".N.1.1#z#A#s#n#B#o#q#C#s#t#D#w#E#F#G#H#I#J#K",
".L.1#L#M#N#k#O#P#n#o#p#C#s#t#Q#w#R#S#T#U#V.a",
".N.K.9#W#X#C#Y#n#n#B#O#Z#X#0#1#w#E#S#2#3.l.a",
".L.4#4#5#p#Z#O#o#O#6#7#X#8#t#9#w#Ra.#Ia#aa.#",
".N#z#b#xab#C#O#pac#Z#X#sad#D#9#E#Fae#Jafag.a",
".Lah#N#X#0#X#C#C#Xai#8#t#D#9#x#Ra.#Iaj.a.a.a",
".N#cak#v#t#0#s#s#s#0#t#Dal#x#E#Saeam.l.a.a.a",
"anao#sak#w#D#Dap#D#D#w#waq#Rarae#Uas.a.a.a.a",
"#Kaeata.at#F#Fau#Fa.a.avae#T#H#Uawax.a.a.a.a",
"ay#Va#azaAasaAasaAasa#aAa#a#aBaCay.a.a.a.a.a"};


/* 
 *  Constructs a BookManEditBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
BookManEditBase::BookManEditBase( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    QPixmap image0( ( const char** ) image0_data );
    QPixmap image1( ( const char** ) image1_data );
    if ( !name )
  	setName( "BookManEditBase" );
    resize( 326, 136 ); 
    setCaption( i18n( "Edit Bookmark"  ) );
    grid = new QGridLayout( this ); 
    grid->setSpacing( 6 );
    grid->setMargin( 11 );

    hbox = new QHBoxLayout; 
    hbox->setSpacing( 6 );
    hbox->setMargin( 0 );
    QSpacerItem* spacer = new QSpacerItem( 130, 20, QSizePolicy::Expanding, QSizePolicy::Fixed );
    hbox->addItem( spacer );

    okButton = new QPushButton( this, "okButton" );
    okButton->setText( i18n( "Ok"  ) );
    hbox->addWidget( okButton );

    cancelButton = new QPushButton( this, "cancelButton" );
    cancelButton->setText( i18n( "Cancel"  ) );
    hbox->addWidget( cancelButton );

    grid->addLayout( hbox, 2, 0 );

    hbox_2 = new QHBoxLayout; 
    hbox_2->setSpacing( 6 );
    hbox_2->setMargin( 0 );

    TextLabel1 = new QLabel( this, "TextLabel1" );
    TextLabel1->setText( i18n( "Name:"  ) );
    hbox_2->addWidget( TextLabel1 );

    nameData = new QLineEdit( this, "nameData" );
    hbox_2->addWidget( nameData );

    clearButton = new QToolButton( this, "clearButton" );
    clearButton->setPixmap( image0 );
    QToolTip::add(  clearButton, i18n( "Clear the Name field" ) );
    hbox_2->addWidget( clearButton );

    grid->addLayout( hbox_2, 0, 0 );

    hbox_3 = new QHBoxLayout; 
    hbox_3->setSpacing( 6 );
    hbox_3->setMargin( 0 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed );
    hbox_3->addItem( spacer_2 );

    TextLabel2 = new QLabel( this, "TextLabel2" );
    TextLabel2->setText( i18n( "Url:"  ) );
    hbox_3->addWidget( TextLabel2 );

    urlData = new QLineEdit( this, "urlData" );
    hbox_3->addWidget( urlData );

    browseButton = new QToolButton( this, "browseButton" );
    browseButton->setPixmap( image1 );
    QToolTip::add(  browseButton, i18n( "Browse for a new path" ) );
    hbox_3->addWidget( browseButton );

    grid->addLayout( hbox_3, 1, 0 );

    // signals and slots connections
    connect( okButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( cancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( clearButton, SIGNAL( clicked() ), nameData, SLOT( clear() ) );
    connect( browseButton, SIGNAL( clicked() ), this, SLOT( browse() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
BookManEditBase::~BookManEditBase()
{
    // no need to delete child widgets, Qt does it all for us
}

void BookManEditBase::browse()
{
    qWarning( "BookManEditBase::browse(): Not implemented yet!" );
}

#include "bookmaneditbase.moc"
