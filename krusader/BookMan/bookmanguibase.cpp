/****************************************************************************
** Form implementation generated from reading ui file 'bookmanguibase.ui'
**
** Created: Wed Oct 24 17:47:34 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "bookmanguibase.h"

#include <klocale.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qtoolbutton.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

static const char* const image0_data[] = { 
"22 22 121 2",
"Qt c None",
".# c #000000",
"#N c #080400",
".s c #101008",
"#Y c #101010",
"#X c #101410",
".r c #181008",
".q c #181010",
"#G c #181410",
"#a c #181818",
".H c #201808",
"#1 c #202420",
".b c #291810",
".a c #291c10",
"#2 c #292429",
"#B c #292829",
"#0 c #292c29",
".G c #311c10",
"#Z c #312c29",
"#S c #312c31",
"#l c #313431",
".F c #392018",
".h c #392410",
".p c #392810",
".c c #392818",
".0 c #393439",
"#J c #393831",
".i c #412818",
".E c #4a2c18",
".C c #4a3018",
".D c #523018",
".2 c #523020",
"#m c #523418",
".B c #523420",
"#d c #52504a",
".L c #525052",
"#v c #5a5052",
"#W c #625d5a",
"## c #626162",
"#M c #6a696a",
"#O c #73696a",
"#t c #736d6a",
"#k c #7b7d7b",
".A c #835529",
"#. c #838183",
"#j c #8b8d8b",
".9 c #948d8b",
"#s c #948d94",
"#F c #949194",
"#V c #949594",
".8 c #9c959c",
"#A c #9c999c",
"#R c #9c9d9c",
"#u c #a46d39",
"#i c #a49d9c",
"#T c #a49da4",
"#U c #a4a19c",
".7 c #a4a1a4",
"#h c #a4a5a4",
".K c #ac6d39",
"#c c #ac7139",
"#Q c #acaaa4",
".6 c #acaaac",
".Z c #acaeac",
"#K c #b4aeb4",
".Y c #b4b2b4",
".X c #b4b6b4",
"#H c #bd7941",
"#z c #bdb6b4",
"#L c #bdb6bd",
"#g c #bdbab4",
".5 c #bdbabd",
".W c #bdbebd",
"#P c #c5bebd",
"#r c #c5bec5",
".V c #c5c2c5",
"#q c #c5c6c5",
"#E c #cdc6cd",
"#y c #cdcac5",
".U c #cdcacd",
"#C c #d5cecd",
"#I c #d5ced5",
".T c #d5d2cd",
"#n c #d5d2d5",
".4 c #d5d6d5",
"#p c #ded6d5",
"#x c #dedad5",
".S c #dedade",
".R c #dedede",
"#D c #e6dede",
"#w c #e6dee6",
"#e c #e6e2e6",
".3 c #e6e6e6",
".j c #eea56a",
"#o c #eee6e6",
".Q c #eee6ee",
".P c #eeeaee",
"#f c #eeeeee",
".z c #f6a15a",
".1 c #f6a55a",
".d c #f6ae6a",
".O c #f6f2f6",
".M c #f6f6f6",
".x c #ffae62",
"#b c #ffb26a",
".w c #ffb66a",
".J c #ffb673",
".g c #ffba73",
".o c #ffba7b",
".f c #ffc27b",
".v c #ffc283",
".e c #ffc683",
".t c #ffc68b",
".n c #ffca8b",
".I c #ffca94",
".k c #ffce9c",
".u c #ffd294",
".m c #ffd29c",
".l c #ffd69c",
".y c #ffd6a4",
".N c #fffaff",
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
"22 22 88 2",
"Qt c None",
".2 c #000000",
".3 c #000408",
"## c #000808",
".5 c #000c08",
".a c #001010",
"#v c #001410",
".H c #001418",
".d c #001818",
"#u c #001c18",
".o c #001c20",
"#f c #002020",
"#i c #002429",
"#. c #002829",
".# c #00484a",
".G c #004c4a",
".n c #005052",
".z c #005552",
".1 c #00595a",
"#t c #005d62",
".h c #006162",
".0 c #006d6a",
".Z c #00797b",
"#s c #007d7b",
"#r c #007d83",
".P c #008183",
".9 c #008583",
".Y c #00898b",
".U c #008d94",
".F c #009194",
".t c #00999c",
".y c #009d9c",
".m c #009da4",
".X c #00aaac",
"#o c #00b2b4",
".l c #00bebd",
".C c #00bec5",
".N c #00c2c5",
".4 c #080808",
"#e c #088583",
".g c #08a5a4",
".E c #08c2bd",
"#j c #08c2c5",
".B c #08cecd",
".w c #08d6d5",
".x c #10c6c5",
".v c #10ced5",
"#k c #10e2d5",
"#p c #18656a",
".T c #18797b",
"#q c #188183",
".M c #18cacd",
".s c #18e2de",
".Q c #206162",
".8 c #20dede",
".c c #29696a",
".W c #29cecd",
".7 c #29d2d5",
".r c #29d6d5",
".u c #29dede",
".V c #31babd",
"#n c #31bec5",
".S c #397d83",
"#d c #39eade",
".O c #39eae6",
".L c #41c6c5",
".D c #41eee6",
".R c #528d8b",
"#m c #52cacd",
"#c c #5ad6d5",
"#h c #62d6de",
".A c #6aaaac",
".b c #6aaeac",
".I c #6ab2b4",
".i c #73aeb4",
".p c #73b2b4",
".e c #7bbabd",
".K c #7bced5",
".6 c #7be6e6",
"#g c #83eaee",
".q c #8bdede",
".k c #8bdee6",
"#b c #8beeee",
"#a c #cdf6f6",
"#l c #cdf6ff",
".f c #def6f6",
".J c #defaff",
".j c #ffffff",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQt.#.aQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQt.#.b.c.dQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQt.#.e.f.g.h.dQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQt.#.i.j.k.l.m.n.oQtQtQtQtQtQtQt",
"QtQtQtQtQtQt.#.p.j.q.r.s.l.t.n.dQtQtQtQtQtQt",
"QtQtQtQtQt.#.p.j.k.u.v.w.s.x.y.z.dQtQtQtQtQt",
"QtQtQtQt.#.A.j.q.r.B.l.C.w.D.E.F.G.HQtQtQtQt",
"QtQtQt.#.I.J.K.L.M.N.N.l.B.w.O.t.P.n.HQtQtQt",
"QtQt.#.Q.R.S.T.U.V.W.N.N.w.X.Y.Z.0.1.#.aQtQt",
"QtQt.2.3.4.3.5.2.k.6.7.C.8.9#.####.3.3.2QtQt",
"QtQtQtQtQtQtQt.##a#b#c.N#d#e#fQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQt.#.j#g#h.C.D.9#iQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQt.#.j#g#c#j#k.9#fQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQt.##l#m#n#o.w.Z#iQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQt.##p#q.Z#r#s#t.dQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQt##.d#u#f#u.o#v##QtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt"};

static const char* const image2_data[] = { 
"22 22 61 1",
". c None",
"g c #000000",
"# c #00484a",
"Z c #005552",
"3 c #00555a",
"R c #005d62",
"O c #007d7b",
"s c #008583",
"E c #00858b",
"A c #00898b",
"D c #008d94",
"Y c #00a5a4",
"2 c #00a5ac",
"N c #00aeac",
"y c #00bebd",
"v c #00bec5",
"W c #00c2c5",
"q c #00c6c5",
"X c #00cac5",
"x c #00cecd",
"Q c #086d73",
"m c #088583",
"M c #08cecd",
"P c #10757b",
"f c #109194",
"5 c #10cacd",
"u c #10cecd",
"C c #10dacd",
"r c #10dad5",
"L c #18cacd",
"V c #20cecd",
"p c #20d2d5",
"1 c #29d2d5",
"w c #31e2d5",
"z c #39e6de",
"I c #41b6bd",
"a c #4ababd",
"H c #52bec5",
"K c #52d2d5",
"G c #6ac6c5",
"F c #6acacd",
"t c #6adade",
"o c #73dade",
"l c #83dede",
"6 c #83e2e6",
"j c #8bdade",
"U c #8be2e6",
"J c #94e6e6",
"i c #9ce2e6",
"k c #a4e6ee",
"4 c #acf6f6",
"0 c #acf6ff",
"B c #bdeaee",
"T c #c5f2f6",
"S c #c5f6ff",
"b c #cdeef6",
"e c #d5f2f6",
"h c #defaff",
"d c #eefaff",
"c c #f6ffff",
"n c #ffffff",
"......................",
"......................",
"......................",
".......########.......",
".......#abcdefg.......",
".......#hijklmg.......",
".......#nopqrsg.......",
".......#ntuvwsg.......",
".......#noxyzAg.......",
"..######BtxvCDgggggg..",
"..gEFGHIJKLyMNAOPQRg..",
"...gsShTUVWvyXCrYZg...",
"....gE0t1qWyvxz23g....",
".....gs4xqyvxzYZg.....",
"......gE0xqxz23g......",
".......gs45zYZg.......",
"........gE623g........",
".........gsZg.........",
"..........gg..........",
"......................",
"......................",
"......................"};


/* 
 *  Constructs a BookManGUIBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
BookManGUIBase::BookManGUIBase( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    QPixmap image0( ( const char** ) image0_data );
    QPixmap image1( ( const char** ) image1_data );
    QPixmap image2( ( const char** ) image2_data );
    if ( !name )
	setName( "BookManGUIBase" );
    resize( 407, 414 ); 
    setMinimumSize( QSize( 0, 0 ) );
    QFont f( font() );
    setFont( f ); 
    setCaption( i18n( "Book.Man: Your Bookmark Manager" ) );
    setFocusPolicy( QDialog::TabFocus );
    setSizeGripEnabled( TRUE );
    BookManGUIBaseLayout = new QGridLayout( this );
    BookManGUIBaseLayout->setSpacing( 6 );
    BookManGUIBaseLayout->setMargin( 11 );

    Layout7 = new QHBoxLayout;
    Layout7->setSpacing( 6 );
    Layout7->setMargin( 0 );

    sortedCheckbox = new QCheckBox( this, "sortedCheckbox" );
    sortedCheckbox->setText( i18n( "Sorted bookmarks" ) );
    QToolTip::add(  sortedCheckbox, i18n( "If checked, bookmarks will be sorted, otherwise, custom ordering is enabled" ) );
    QWhatsThis::add(  sortedCheckbox, i18n( "BookMan allows displaying bookmarks in 2 ways:\n"
"1) The bookmarks are sorted by their NAME.\n"
"2) Unsorted: this allows custom ordering by using the UP and DOWN\n"
"   arrows on the \"Edit Bookmarks\" tab." ) );
    Layout7->addWidget( sortedCheckbox );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout7->addItem( spacer );

    okBtn = new QPushButton( this, "okBtn" );
    okBtn->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, okBtn->sizePolicy().hasHeightForWidth() ) );
    okBtn->setText( i18n( "  O&k  " ) );
    okBtn->setAutoDefault( TRUE );
    okBtn->setDefault( TRUE );
    Layout7->addWidget( okBtn );

    closeBtn = new QPushButton( this, "closeBtn" );
    closeBtn->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, closeBtn->sizePolicy().hasHeightForWidth() ) );
    closeBtn->setText( i18n( "&Close" ) );
    closeBtn->setAutoDefault( FALSE );
    closeBtn->setDefault( FALSE );
    Layout7->addWidget( closeBtn );

    BookManGUIBaseLayout->addLayout( Layout7, 1, 0 );

    tabWidget = new QTabWidget( this, "tabWidget" );

    Widget2 = new QWidget( tabWidget, "Widget2" );
    Widget2Layout = new QGridLayout( Widget2 );
    Widget2Layout->setSpacing( 6 );
    Widget2Layout->setMargin( 11 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Widget2Layout->addItem( spacer_2, 2, 0 );

    Layout9 = new QGridLayout;
    Layout9->setSpacing( 6 );
    Layout9->setMargin( 0 );

    nameData = new QLineEdit( Widget2, "nameData" );
    nameData->setFocusPolicy( QLineEdit::StrongFocus );

    Layout9->addWidget( nameData, 0, 1 );

    TextLabel4 = new QLabel( Widget2, "TextLabel4" );
    QFont TextLabel4_font(  TextLabel4->font() );
    TextLabel4_font.setItalic( TRUE );
    TextLabel4->setFont( TextLabel4_font );
    TextLabel4->setText( i18n( "(example: Drive C:)" ) );
    TextLabel4->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    Layout9->addWidget( TextLabel4, 1, 1 );

    addButton = new QPushButton( Widget2, "addButton" );
    addButton->setText( i18n( "Add" ) );

    Layout9->addWidget( addButton, 0, 2 );

    TextLabel2 = new QLabel( Widget2, "TextLabel2" );
    TextLabel2->setText( i18n( "Name:" ) );

    Layout9->addWidget( TextLabel2, 0, 0 );

    Widget2Layout->addLayout( Layout9, 1, 0 );

    Layout9_2 = new QVBoxLayout;
    Layout9_2->setSpacing( 6 );
    Layout9_2->setMargin( 0 );

    TextLabel1 = new QLabel( Widget2, "TextLabel1" );
    QFont TextLabel1_font(  TextLabel1->font() );
    TextLabel1_font.setPointSize( 16 );
    TextLabel1_font.setBold( TRUE );
    TextLabel1->setFont( TextLabel1_font );
    TextLabel1->setFrameShape( QLabel::NoFrame );
    TextLabel1->setFrameShadow( QLabel::Plain );
    TextLabel1->setText( i18n( "Adding A New Bookmark" ) );
    TextLabel1->setAlignment( int( QLabel::AlignCenter ) );
    Layout9_2->addWidget( TextLabel1 );

    Line1 = new QFrame( Widget2, "Line1" );
    Line1->setFrameShadow( QFrame::Raised );
    Line1->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    Layout9_2->addWidget( Line1 );

    Widget2Layout->addLayout( Layout9_2, 0, 0 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Widget2Layout->addItem( spacer_3, 4, 0 );

    Layout8 = new QGridLayout;
    Layout8->setSpacing( 6 );
    Layout8->setMargin( 0 );

    TextLabel3 = new QLabel( Widget2, "TextLabel3" );
    TextLabel3->setText( i18n( "URL:" ) );

    Layout8->addWidget( TextLabel3, 0, 0 );

    browseButton = new QToolButton( Widget2, "browseButton" );
    browseButton->setText(  ""  );
    browseButton->setPixmap( image0 );

    Layout8->addWidget( browseButton, 0, 2 );

    TextLabel5 = new QLabel( Widget2, "TextLabel5" );
    QFont TextLabel5_font(  TextLabel5->font() );
    TextLabel5_font.setItalic( TRUE );
    TextLabel5->setFont( TextLabel5_font );
    TextLabel5->setText( i18n( "(example: /mnt/drivec)" ) );
    TextLabel5->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    Layout8->addMultiCellWidget( TextLabel5, 1, 1, 0, 1 );
    QSpacerItem* spacer_4 = new QSpacerItem( 45, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
    Layout8->addItem( spacer_4, 0, 3 );

    urlData = new QLineEdit( Widget2, "urlData" );
    urlData->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, urlData->sizePolicy().hasHeightForWidth() ) );

    Layout8->addWidget( urlData, 0, 1 );

    Widget2Layout->addLayout( Layout8, 3, 0 );
    tabWidget->insertTab( Widget2, i18n( "Add Bookmark" ) );

    Widget3 = new QWidget( tabWidget, "Widget3" );
    Widget3Layout = new QGridLayout( Widget3 );
    Widget3Layout->setSpacing( 6 );
    Widget3Layout->setMargin( 11 );

    Layout5 = new QVBoxLayout;
    Layout5->setSpacing( 6 );
    Layout5->setMargin( 0 );

    TextLabel1_2 = new QLabel( Widget3, "TextLabel1_2" );
    QFont TextLabel1_2_font(  TextLabel1_2->font() );
    TextLabel1_2_font.setPointSize( 16 );
    TextLabel1_2_font.setBold( TRUE );
    TextLabel1_2->setFont( TextLabel1_2_font );
    TextLabel1_2->setFrameShape( QLabel::NoFrame );
    TextLabel1_2->setFrameShadow( QLabel::Plain );
    TextLabel1_2->setText( i18n( "Bookmark Editor" ) );
    TextLabel1_2->setAlignment( int( QLabel::AlignCenter ) );
    Layout5->addWidget( TextLabel1_2 );

    Line1_2 = new QFrame( Widget3, "Line1_2" );
    Line1_2->setFrameShadow( QFrame::Raised );
    Line1_2->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    Layout5->addWidget( Line1_2 );

    Widget3Layout->addMultiCellLayout( Layout5, 0, 0, 0, 1 );

    bookmarks = new QListView( Widget3, "bookmarks" );
    bookmarks->addColumn( i18n( "Name" ) );
    bookmarks->header()->setClickEnabled( FALSE, bookmarks->header()->count() - 1 );
    bookmarks->addColumn( i18n( "Url" ) );
    bookmarks->header()->setClickEnabled( FALSE, bookmarks->header()->count() - 1 );
    bookmarks->setCursor( QCursor( 0 ) );
    bookmarks->setFrameShape( QListView::StyledPanel );
    bookmarks->setFrameShadow( QListView::Raised );
    bookmarks->setVScrollBarMode( QListView::AlwaysOn );
    bookmarks->setHScrollBarMode( QListView::AlwaysOn );
    bookmarks->setAllColumnsShowFocus( TRUE );

    Widget3Layout->addWidget( bookmarks, 1, 0 );

    Layout7_2 = new QVBoxLayout;
    Layout7_2->setSpacing( 6 );
    Layout7_2->setMargin( 0 );
    QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Layout7_2->addItem( spacer_5 );

    upButton = new QToolButton( Widget3, "upButton" );
    upButton->setText( "" );
    upButton->setPixmap( image1 );
    QToolTip::add(  upButton, i18n( "Press this to move the selected bookmark UP one place in the list" ) );
    Layout7_2->addWidget( upButton );

    downButton = new QToolButton( Widget3, "downButton" );
    downButton->setText( "" );
    downButton->setPixmap( image2 );
    QToolTip::add(  downButton, i18n( "Press this to move the selected bookmark DOWN one place in the list" ) );
    Layout7_2->addWidget( downButton );
    QSpacerItem* spacer_6 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Layout7_2->addItem( spacer_6 );

    editButton = new QPushButton( Widget3, "editButton" );
    editButton->setText( i18n( "Edit" ) );
    Layout7_2->addWidget( editButton );

    removeButton = new QPushButton( Widget3, "removeButton" );
    removeButton->setText( i18n( "Remove" ) );
    Layout7_2->addWidget( removeButton );

    applyButton = new QPushButton( Widget3, "applyButton" );
    applyButton->setText( i18n( "Apply" ) );
    Layout7_2->addWidget( applyButton );
    QSpacerItem* spacer_7 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Layout7_2->addItem( spacer_7 );

    Widget3Layout->addLayout( Layout7_2, 1, 1 );
    tabWidget->insertTab( Widget3, i18n( "Edit Bookmarks" ) );

    BookManGUIBaseLayout->addWidget( tabWidget, 0, 0 );

    // signals and slots connections
    connect( urlData, SIGNAL( returnPressed() ), nameData, SLOT( setFocus() ) );
    connect( editButton, SIGNAL( clicked() ), this, SLOT( slotEditBookmark() ) );
    connect( applyButton, SIGNAL( clicked() ), this, SLOT( slotApply() ) );
    connect( upButton, SIGNAL( clicked() ), this, SLOT( slotUp() ) );
    connect( downButton, SIGNAL( clicked() ), this, SLOT( slotDown() ) );
    connect( removeButton, SIGNAL( clicked() ), this, SLOT( slotRemoveBookmark() ) );
    connect( addButton, SIGNAL( clicked() ), this, SLOT( slotPreAddBookmark() ) );
    connect( nameData, SIGNAL( returnPressed() ), this, SLOT( slotAddBookmark() ) );
    connect( browseButton, SIGNAL( clicked() ), this, SLOT( slotBrowse() ) );
    connect( closeBtn, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( okBtn, SIGNAL( clicked() ), this, SLOT( accept() ) );

    // tab order
    setTabOrder( bookmarks, tabWidget );
    setTabOrder( tabWidget, nameData );
    setTabOrder( nameData, addButton );
    setTabOrder( addButton, urlData );
    setTabOrder( urlData, closeBtn );
    setTabOrder( closeBtn, editButton );
    setTabOrder( editButton, applyButton );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
BookManGUIBase::~BookManGUIBase()
{
    // no need to delete child widgets, Qt does it all for us
}

/*  
 *  Main event handler. Reimplemented to handle application
 *  font changes
 */
bool BookManGUIBase::event( QEvent* ev )
{
    bool ret = QDialog::event( ev ); 
    if ( ev->type() == QEvent::ApplicationFontChange ) {
	QFont TextLabel4_font(  TextLabel4->font() );
	TextLabel4_font.setItalic( TRUE );
	TextLabel4->setFont( TextLabel4_font ); 
	QFont TextLabel1_font(  TextLabel1->font() );
	TextLabel1_font.setPointSize( 16 );
	TextLabel1_font.setBold( TRUE );
	TextLabel1->setFont( TextLabel1_font ); 
	QFont TextLabel5_font(  TextLabel5->font() );
	TextLabel5_font.setItalic( TRUE );
	TextLabel5->setFont( TextLabel5_font ); 
	QFont TextLabel1_2_font(  TextLabel1_2->font() );
	TextLabel1_2_font.setPointSize( 16 );
	TextLabel1_2_font.setBold( TRUE );
	TextLabel1_2->setFont( TextLabel1_2_font ); 
    }
    return ret;
}

void BookManGUIBase::slotBrowse()
{
    qWarning( "BookManGUIBase::slotBrowse(): Not implemented yet!" );
}

void BookManGUIBase::slotAddBookmark()
{
    qWarning( "BookManGUIBase::slotAddBookmark(): Not implemented yet!" );
}

void BookManGUIBase::slotApply()
{
    qWarning( "BookManGUIBase::slotApply(): Not implemented yet!" );
}

void BookManGUIBase::slotDown()
{
    qWarning( "BookManGUIBase::slotDown(): Not implemented yet!" );
}

void BookManGUIBase::slotEditBookmark()
{
    qWarning( "BookManGUIBase::slotEditBookmark(): Not implemented yet!" );
}

void BookManGUIBase::slotPreAddBookmark()
{
    qWarning( "BookManGUIBase::slotPreAddBookmark(): Not implemented yet!" );
}

void BookManGUIBase::slotRemoveBookmark()
{
    qWarning( "BookManGUIBase::slotRemoveBookmark(): Not implemented yet!" );
}

void BookManGUIBase::slotUp()
{
    qWarning( "BookManGUIBase::slotUp(): Not implemented yet!" );
}

