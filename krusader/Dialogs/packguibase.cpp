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

static const char* const image0_data[] = { 
"22 22 121 2",
"#. c #838183",
"#b c #ffb26a",
"#2 c #292429",
"#g c #bdbab4",
"#K c #b4aeb4",
".F c #392018",
".5 c #bdbabd",
"#Z c #312c29",
"#V c #949594",
".A c #835529",
".i c #412818",
".e c #ffc683",
"#S c #312c31",
".t c #ffc68b",
".V c #c5c2c5",
"#R c #9c9d9c",
".0 c #393439",
"#y c #cdcac5",
".U c #cdcacd",
"#h c #a4a5a4",
"#t c #736d6a",
".T c #d5d2cd",
"#n c #d5d2d5",
".j c #eea56a",
".K c #ac6d39",
".R c #dedede",
"#d c #52504a",
".L c #525052",
".3 c #e6e6e6",
".x c #ffae62",
"#f c #eeeeee",
"#z c #bdb6b4",
".f c #ffc27b",
"#L c #bdb6bd",
"#F c #949194",
".C c #4a3018",
".M c #f6f6f6",
".v c #ffc283",
"#P c #c5bebd",
"#r c #c5bec5",
"#A c #9c999c",
".l c #ffd69c",
"#U c #a4a19c",
"#E c #cdc6cd",
".y c #ffd6a4",
".7 c #a4a1a4",
"#O c #73696a",
"#C c #d5cecd",
"#x c #dedad5",
"#I c #d5ced5",
"#X c #101410",
".S c #dedade",
".a c #291c10",
"#e c #e6e2e6",
"Qt c None",
"#1 c #202420",
".P c #eeeaee",
".9 c #948d8b",
"#s c #948d94",
".E c #4a2c18",
".O c #f6f2f6",
"#m c #523418",
".# c #000000",
".8 c #9c959c",
".u c #ffd294",
".B c #523420",
".m c #ffd29c",
"#i c #a49d9c",
"#T c #a49da4",
".s c #101008",
".1 c #f6a55a",
"#p c #ded6d5",
"#Y c #101010",
"#D c #e6dede",
"#a c #181818",
"#H c #bd7941",
".b c #291810",
"#w c #e6dee6",
"#v c #5a5052",
".Z c #acaeac",
"#o c #eee6e6",
".d c #f6ae6a",
".p c #392810",
".Q c #eee6ee",
"#0 c #292c29",
".g c #ffba73",
".X c #b4b6b4",
".c c #392818",
".o c #ffba7b",
"#l c #313431",
".N c #fffaff",
".D c #523018",
".2 c #523020",
"## c #626162",
"#N c #080400",
".k c #ffce9c",
"#M c #6a696a",
"#u c #a46d39",
".z c #f6a15a",
"#G c #181410",
"#k c #7b7d7b",
"#Q c #acaaa4",
".6 c #acaaac",
".G c #311c10",
".w c #ffb66a",
".h c #392410",
"#B c #292829",
".J c #ffb673",
".Y c #b4b2b4",
"#j c #8b8d8b",
".W c #bdbebd",
".n c #ffca8b",
"#W c #625d5a",
"#q c #c5c6c5",
"#J c #393831",
".I c #ffca94",
".4 c #d5d6d5",
"#c c #ac7139",
".r c #181008",
".q c #181010",
".H c #201808",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQt.#.#.#.#.#QtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQt.#.#.#.#.#.#.#.#.#Qt.#QtQtQtQt",
"QtQtQtQtQtQt.#.#QtQtQtQtQtQt.#.#.#.#QtQtQtQt",
"QtQtQtQtQt.#QtQtQtQtQtQtQtQtQt.#.#.#QtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQt.#.#.#.#QtQtQtQt",
"QtQt.a.a.a.bQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"Qt.c.d.e.f.g.hQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
".i.j.k.l.m.n.o.p.q.r.r.s.q.r.r.sQtQtQtQtQtQt",
".h.t.l.u.v.w.x.x.x.x.x.x.x.x.x.x.hQtQtQtQtQt",
".i.n.y.v.z.A.B.C.D.C.E.i.i.h.F.G.a.H.#.#.#.#",
".h.n.I.J.K.L.M.N.M.O.P.Q.R.S.T.U.V.W.X.Y.Z.0",
".i.t.t.1.2.U.P.3.Q.S.4.U.V.5.Y.6.7.8.9#.###a",
".h.t#b#c#d.M#e#f.P.Q.R.4.U.V#g.Y#h#i#j#k#lQt",
".i.g.z#m#n.4#o.P.P.3#e#p#n#q#r.Y.6#i#s#t.#Qt",
".h.w#u#v.R#p#e.Q#e#w#x#n#y.V#z.Y#h#A#.#BQtQt",
".i.1.D.Z#C.4#e.R#D#p#n.U#E.5#z.6.7#F###GQtQt",
".h#H#d#n#q#n.4.4#n#I#y.V.5#z.Z#h#A#.#JQtQtQt",
".i.E#K.X.V#q.U.U.U#q.V.5#L.Z.6#i#F#M.#QtQtQt",
"#N#O.U#K.Y.5.5#P.5.5.Y.Y#Q#h#R#F#k#SQtQtQtQt",
"#a#F#T#A#T.7.7#U.7#A#A#V#F#j.9#k#W#XQtQtQtQt",
"#Y#l#B#Z#0#S#0#S#0#S#B#0#B#B#1#2#YQtQtQtQtQt"};

static const char* const image1_data[] = { 
"32 32 180 2",
"#O c #ff7d08",
"aS c #ffb262",
"aK c #f67108",
"#f c #8b4020",
"#. c #fff2de",
"#k c #ffb26a",
"#9 c #ac2808",
"ax c #bd3c00",
"ap c #b44400",
"aM c #b43000",
"#h c #fff2e6",
"#g c #944c31",
"aG c #bd3c08",
".B c #e68939",
".I c #ffc683",
"#m c #ffc68b",
"ay c #cd4c08",
".v c #ee9141",
".n c #ffc694",
"#6 c #ffdaa4",
".c c #ffa552",
"#U c #ffdaac",
"#N c #de6900",
".u c #f69952",
".w c #ffa55a",
"ab c #de6908",
"#C c #ffdab4",
"#H c #e67100",
"aW c #e65d00",
".Z c #ffdabd",
"aP c #e65d08",
"#7 c #940c08",
"aQ c #ee6500",
"at c #9c1400",
".z c #d57118",
".h c #201408",
"#0 c #ff7900",
".U c #833420",
"#D c #ffae5a",
".t c #f6e2cd",
".T c #8b3c18",
"#B c #ffeed5",
"aX c #f66d08",
".e c #ffae62",
"au c #a41c08",
".D c #c57541",
"#x c #8b3c20",
"av c #ac2400",
".X c #ffeede",
".V c #834839",
".x c #b4755a",
"#e c #e68531",
"aL c #ffc27b",
"#4 c #bd956a",
"#G c #ffc283",
"aO c #cd4800",
"#J c #bd9573",
".s c #ffc28b",
"ag c #ffc294",
"#Y c #c55d00",
".# c #000408",
"ai c #ffd69c",
"#S c #c59d7b",
"#5 c #f6ca9c",
".1 c #c55d08",
"#A c #ffd6a4",
"ae c #dea573",
".y c #cd6508",
".0 c #ffd6ac",
"#2 c #dea57b",
"#w c #cd6510",
"#y c #ffd6b4",
"aI c #d55908",
"#V c #8b1000",
"#d c #d56d10",
"a# c #943008",
"#W c #941c08",
".O c #d56d18",
".A c #de7920",
"a. c #a42c00",
".d c #ffaa5a",
"Qt c None",
"aR c #ff7508",
"#o c #de7929",
"#j c #ffead5",
".4 c #e68129",
"aD c #ffbe73",
".3 c #e68131",
"aw c #bd3408",
"## c #ffbe7b",
".5 c #9c4c31",
"#3 c #bd916a",
"#I c #eebe8b",
".r c #ffbe83",
"aN c #cd4408",
".a c #000000",
"#T c #f6c694",
".2 c #c55900",
"#E c #ffd29c",
"#L c #6a1008",
".K c #4a4039",
".j c #ffd2a4",
".8 c #6a2410",
".M c #cd6108",
"#t c #ffd2ac",
"az c #d55500",
".l c #ff0400",
".q c #830408",
".g c #ff0408",
"#c c #d56918",
"#i c #ffe6cd",
"#Q c #deaa7b",
".p c #ffe6d5",
".R c #cd7541",
"aa c #b44c00",
"aC c #ffba73",
"#1 c #eeba83",
"#u c #ffba7b",
".Q c #ee8531",
".W c #fffaf6",
".C c #e69141",
"aE c #ffce8b",
"#X c #621800",
"ah c #ffce94",
"#R c #c59573",
".L c #c55500",
".i c #ffce9c",
"aq c #c55508",
"#F c #cd9d73",
".N c #cd5d00",
"#M c #ffcea4",
".b c #080408",
".G c #945d4a",
"aJ c #de5d00",
".Y c #524441",
".J c #f6d6b4",
".f c #ff0000",
"aj c #8b0800",
"#v c #d56510",
"aU c #e66508",
"ak c #941408",
"aV c #ee6d00",
"#r c #ffe2c5",
"#p c #b46939",
".o c #ffe2cd",
"aB c #f67500",
"ad c #ff8108",
".P c #e67920",
"aF c #ac2c00",
".F c #ac755a",
"#z c #ffb66a",
"am c #ac2c08",
"an c #b43400",
"#l c #ffb673",
"ao c #bd4008",
".9 c #fff6ee",
"aH c #c54800",
".H c #ffb67b",
"ar c #f6be83",
"as c #ffca8b",
"af c #c5916a",
"#P c #f6be8b",
"#K c #bd9d7b",
"#n c #ffca94",
".m c #ffca9c",
"#b c #cd5900",
".S c #a45531",
"aT c #de5900",
"#8 c #941000",
"#q c #7b3018",
"#s c #ffdebd",
".E c #b46531",
"#Z c #e67508",
".7 c #7b3020",
".k c #8b0408",
"al c #9c1800",
"#a c #ffdec5",
".6 c #833818",
"aA c #ee6908",
"ac c #ff7d00",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.#.a.#.a.#QtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQt.b.c.d.c.d.c.bQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQt.a.e.f.g.f.d.aQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQt.b.c.g.f.g.d.bQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQt.a.d.f.g.f.d.aQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQt.b.d.g.f.g.d.bQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQt.h.d.f.g.f.d.aQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQt.b.a.b.d.g.f.g.d.b.a.bQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQt.#.a.#.c.d.c.d.f.g.f.d.c.d.c.#QtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQt.a.b.i.j.a.k.f.g.f.g.f.g.l.g.f.k.aQtQtQtQtQtQtQtQt",
"QtQtQtQtQt.#.a.m.n.m.o.p.a.q.f.g.f.g.f.g.f.q.a.d.a.#QtQtQtQtQtQt",
"QtQtQt.a.b.r.s.r.p.t.u.v.w.a.k.f.g.f.g.f.k.a.x.c.e.e.b.aQtQtQtQt",
"QtQt.a.e.d.e.o.p.y.z.A.B.C.w.a.q.f.g.f.q.a.D.E.F.G.H.I.J.K.#QtQt",
"QtQtQt.a.b.a.b.L.M.N.O.P.Q.v.w.a.k.f.k.a.R.E.S.T.U.V.W.a.X.Y.bQt",
"QtQtQtQtQt.#.d.Z.0.1.2.O.z.3.4.w.a.q.a.D.E.5.6.7.8.W.9#..aQtQtQt",
"QtQtQtQt.b.c.d.e##.Z#a#b#c#d.3#e.w.a.R.E#f#g.7.X#h#i.X#j#..aQtQt",
"QtQtQt.#.c.e.d#k#l#m#n#j#i#c.z#o.O#p.E#f#q#r#s#t.0#r#j.X#j#..aQt",
"QtQt.b.c.d.c.d.d#u.r.m#t#a.X#h#v#w.E#x#t#y#z.r.s#t#A#r#i.p#B.b.a",
"Qt.#.a.d.c.d.d#k#l.s#n.0#C.p#j.W.9.W#t.d.c.d#D#u.r.m#E.p.a.#QtQt",
"QtQtQt.a.b.c.d.e#u.r.m#E#s#s.X#j.b#F.b.c.d.c.d.e#u#l.b.aQtQtQtQt",
"QtQtQtQtQt.#.a.e#z#G.n#t#C#i#B.##H#I#J.#.d.d.c.d.a.##K.#QtQtQtQt",
"QtQtQtQtQt.a#L.a.b.r#n#M.Z#r.b#N#O#P#Q#R.b.c.b.a#S#T#U.aQtQtQtQt",
"QtQtQt.#.a.##V#W#X.#.a.0#A.##Y#Z#0#m#1#2#3.##4#5.i#U#6.#QtQtQtQt",
"Qt.a.b.a.b.a#7#8#9a.a#.a.baaabacad#G.rae#2afagahai#A#U.aQtQtQtQt",
"QtQtQt.#.a.#ajakalamanaoapaq#N#Oac#m##.rarasas#Eah#E#E.#QtQtQtQt",
"QtQtQtQtQt.a.batauavawaxayazaAaBadaC.raD#m.Ias.IahaE#E.aQtQtQtQt",
"QtQtQtQtQtQtQt.#.a#9aFaGaHaIaJaK#0aC#l###uaL#G#m.I.#.aQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQt.a.baMaNaOaPaQaRaSaC#z###u##.a.bQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQt.#.aayaTaUaVaSaS#z#z.#.aQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQt.a.baWaX.c.d.a.bQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.#.a.#.aQtQtQtQtQtQtQtQtQtQtQtQtQt"};


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
    QPixmap image0( ( const char** ) image0_data );
    QPixmap image1( ( const char** ) image1_data );
    if ( !name )
	setName( "PackGUIBase" );
    resize( 336, 192 ); 
    setCaption( i18n( "Pack"  ) );
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
    browseButton->setPixmap( image0 );
    hbox_2->addWidget( browseButton );
    QSpacerItem* spacer = new QSpacerItem( 40, 20, QSizePolicy::Fixed, QSizePolicy::Fixed );
    hbox_2->addItem( spacer );

    grid->addLayout( hbox_2, 2, 0 );

    hbox_3 = new QHBoxLayout;
    hbox_3->setSpacing( 6 );
    hbox_3->setMargin( 0 );

    PixmapLabel1 = new QLabel( this, "PixmapLabel1" );
    PixmapLabel1->setPixmap( image1 );
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

    moveCheckbox = new QCheckBox( this, "moveCheckbox" );
    moveCheckbox->setText( i18n( "Move into archive"  ) );
    hbox_5->addWidget( moveCheckbox );
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
