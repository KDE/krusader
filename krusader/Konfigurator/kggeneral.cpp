/****************************************************************************
** Form implementation generated from reading ui file 'kggeneral.ui'
**
** Created: Fri Jun 1 18:20:56 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "kggeneral.h"

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtoolbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <klocale.h>

static const char* const image0_data[] = { 
"16 16 53 1",
". c None",
"# c #000000",
"R c #080808",
"Q c #101010",
"Y c #181c18",
"K c #202020",
"w c #313031",
"X c #313431",
"W c #414041",
"V c #414441",
"g c #4a3018",
"E c #4a484a",
"U c #4a4c4a",
"l c #524c4a",
"h c #52504a",
"j c #525052",
"i c #5a5952",
"J c #6a6962",
"P c #6a6d6a",
"T c #8b8d8b",
"D c #949594",
"S c #9c999c",
"O c #9c9d9c",
"C c #a4a19c",
"M c #a4a1a4",
"N c #a4a5a4",
"I c #acaaac",
"B c #acaeac",
"F c #b47d41",
"v c #b4b2b4",
"u c #b4b6b4",
"A c #bdbabd",
"t c #bdbebd",
"k c #c5854a",
"H c #c5c2c5",
"s c #c5c6c5",
"r c #cdcacd",
"L c #cdcecd",
"z c #d5d2d5",
"q c #d5d6d5",
"x c #dedade",
"p c #dedede",
"a c #e6a562",
"G c #e6e2e6",
"o c #e6e6e6",
"y c #eeeae6",
"n c #eeeeee",
"m c #f6f6f6",
"e c #ffae62",
"d c #ffc283",
"c c #ffc683",
"f c #ffca8b",
"b c #ffd29c",
"......####......",
".....##.####.#..",
"....#.....####..",
"...........###..",
"..........####..",
".####...........",
"#abca#######....",
"#bcdeeeeeeee#...",
"#fdghhhihhjjh###",
"#dklmmnopqrstuvw",
"#dgxpyopzsABCDE.",
"#FhxxGpqrHuICJK.",
"#gLrqqzLHABMDE..",
"#jLtssHtuBNOPQ..",
"RtNIBBINMSDTE...",
"KEEUUUEEVWWXY..."};


/* 
 *  Constructs a kgGeneral which is a child of 'parent', with the 
 *  name 'name'.' 
 */
kgGeneral::kgGeneral( QWidget* parent,  const char* name )
    : QFrame( parent, name )
{
    QPixmap image0( ( const char** ) image0_data );
    if ( !name )
	setName( "kgGeneral" );
    resize( 393, 277 ); 
    setCaption( i18n( "General" ) );
    setFrameShape( QFrame::NoFrame );
    setFrameShadow( QFrame::MShadow );
    kgGeneralLayout = new QGridLayout( parent );
    kgGeneralLayout->setSpacing( 6 );
    kgGeneralLayout->setMargin( 11 );

    GroupBox2 = new QGroupBox( parent, "GroupBox2" );
    GroupBox2->setTitle( i18n( "General" ) );
    GroupBox2->setColumnLayout(0, Qt::Vertical );
    GroupBox2->layout()->setSpacing( 0 );
    GroupBox2->layout()->setMargin( 0 );
    GroupBox2Layout = new QGridLayout( GroupBox2->layout() );
    GroupBox2Layout->setAlignment( Qt::AlignTop );
    GroupBox2Layout->setSpacing( 6 );
    GroupBox2Layout->setMargin( 11 );

    Layout8 = new QGridLayout;
    Layout8->setSpacing( 6 );
    Layout8->setMargin( 0 );

    kgTerminal = new QLineEdit( GroupBox2, "kgTerminal" );

    Layout8->addWidget( kgTerminal, 2, 1 );

    TextLabel2 = new QLabel( GroupBox2, "TextLabel2" );
    TextLabel2->setText( i18n( "Terminal:" ) );

    Layout8->addWidget( TextLabel2, 2, 0 );

    QLabel* hint = new QLabel(GroupBox2,"hint");
    hint->setText(i18n("Hint: use 'internal editor' if you want to use Krusader's fast built-in editor") );
    Layout8->addMultiCellWidget(hint,1,1,0,2);

    kgEditor = new QLineEdit( GroupBox2, "kgEditor" );

    Layout8->addWidget( kgEditor, 0, 1 );

    kgBrowseEditor = new QToolButton( GroupBox2, "kgBrowseEditor" );
    kgBrowseEditor->setText( "" );
    kgBrowseEditor->setPixmap( image0 );

    Layout8->addWidget( kgBrowseEditor, 0, 2 );

    TextLabel1_2 = new QLabel( GroupBox2, "TextLabel1_2" );
    TextLabel1_2->setText( i18n( "Editor:" ) );

    Layout8->addWidget( TextLabel1_2, 0, 0 );

    kgBrowseTerminal = new QToolButton( GroupBox2, "kgBrowseTerminal" );
    kgBrowseTerminal->setText( "" );
    kgBrowseTerminal->setPixmap( image0 );

    Layout8->addWidget( kgBrowseTerminal, 2, 2 );

    GroupBox2Layout->addMultiCellLayout( Layout8, 3, 3, 0, 2 );

    Line2_2 = new QFrame( GroupBox2, "Line2_2" );
    Line2_2->setFrameStyle( QFrame::HLine | QFrame::Sunken );

    GroupBox2Layout->addMultiCellWidget( Line2_2, 4, 4, 0, 2 );

    kgTempDir = new QLineEdit( GroupBox2, "kgTempDir" );

    GroupBox2Layout->addWidget( kgTempDir, 5, 1 );

    kgBrowseTempDir = new QToolButton( GroupBox2, "kgBrowseTempDir" );
    kgBrowseTempDir->setText( "" );
    kgBrowseTempDir->setPixmap( image0 );

    GroupBox2Layout->addWidget( kgBrowseTempDir, 5, 2 );

    TextLabel2_2 = new QLabel( GroupBox2, "TextLabel2_2" );
    TextLabel2_2->setText( i18n( "Temp Directory:" ) );

    GroupBox2Layout->addWidget( TextLabel2_2, 5, 0 );

    TextLabel1 = new QLabel( GroupBox2, "TextLabel1" );
    TextLabel1->setText( i18n( "Note: you must have full permissions for the temporary directory !" ) );

    GroupBox2Layout->addMultiCellWidget( TextLabel1, 6, 6, 0, 2 );

    ButtonGroup1 = new QButtonGroup( GroupBox2, "ButtonGroup1" );
    ButtonGroup1->setFrameShape( QButtonGroup::NoFrame );
    ButtonGroup1->setFrameShadow( QButtonGroup::Sunken );
    ButtonGroup1->setTitle( "" );
    ButtonGroup1->setExclusive( TRUE );
    ButtonGroup1->setRadioButtonExclusive( TRUE );
    ButtonGroup1->setColumnLayout(0, Qt::Vertical );
    ButtonGroup1->layout()->setSpacing( 0 );
    ButtonGroup1->layout()->setMargin( 0 );
    ButtonGroup1Layout = new QGridLayout( ButtonGroup1->layout() );
    ButtonGroup1Layout->setAlignment( Qt::AlignTop );
    ButtonGroup1Layout->setSpacing( 6 );
    ButtonGroup1Layout->setMargin( 11 );

    kgMoveToTrash = new QRadioButton( ButtonGroup1, "kgMoveToTrash" );
    kgMoveToTrash->setText( i18n( "Move to trash" ) );

    ButtonGroup1Layout->addWidget( kgMoveToTrash, 0, 1 );

    kgDeleteFiles = new QRadioButton( ButtonGroup1, "kgDeleteFiles" );
    kgDeleteFiles->setText( i18n( "Delete files" ) );

    ButtonGroup1Layout->addWidget( kgDeleteFiles, 0, 0 );

    GroupBox2Layout->addMultiCellWidget( ButtonGroup1, 0, 0, 0, 2 );

    Line2 = new QFrame( GroupBox2, "Line2" );
    Line2->setFrameStyle( QFrame::HLine | QFrame::Sunken );

    GroupBox2Layout->addMultiCellWidget( Line2, 2, 2, 0, 2 );

    kgMimetypeMagic = new QCheckBox( GroupBox2, "kgMimetypeMagic" );
    kgMimetypeMagic->setText( i18n( "Use mimetype magic" ) );
    QToolTip::add(  kgMimetypeMagic, i18n( "Mimetype magic allows better distinction of file types, but is slower" ) );

    GroupBox2Layout->addMultiCellWidget( kgMimetypeMagic, 1, 1, 0, 2 );

    kgGeneralLayout->addWidget( GroupBox2, 0, 0 );

    // signals and slots connections
    connect( kgBrowseEditor, SIGNAL( clicked() ), this, SLOT( slotBrowseEditor() ) );
    connect( kgBrowseTerminal, SIGNAL( clicked() ), this, SLOT( slotBrowseTerminal() ) );
    connect( kgBrowseTempDir, SIGNAL( clicked() ), this, SLOT( slotBrowseTempDir() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
kgGeneral::~kgGeneral()
{
    // no need to delete child widgets, Qt does it all for us
}

void kgGeneral::slotBrowseTempDir()
{
    qWarning( "kgGeneral::slotBrowseTempDir(): Not implemented yet!" );
}

void kgGeneral::slotApplyChanges()
{
    qWarning( "kgGeneral::slotApplyChanges(): Not implemented yet!" );
}

void kgGeneral::slotBrowseEditor()
{
    qWarning( "kgGeneral::slotBrowseEditor(): Not implemented yet!" );
}

void kgGeneral::slotBrowseTerminal()
{
    qWarning( "kgGeneral::slotBrowseTerminal(): Not implemented yet!" );
}

void kgGeneral::slotDefaultSettings()
{
    qWarning( "kgGeneral::slotDefaultSettings(): Not implemented yet!" );
}

#include "kggeneral.moc"
