/****************************************************************************
** Form implementation generated from reading ui file 'kglookfeel.ui'
**
** Created: Wed Jun 6 14:58:16 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "kglookfeel.h"

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qtoolbutton.h>
#include <qwidget.h>
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
 *  Constructs a kgLookFeel which is a child of 'parent', with the 
 *  name 'name'.' 
 */
kgLookFeel::kgLookFeel( QWidget* parent,  const char* name )
    : QFrame( parent, name )
{
    QPixmap image0( ( const char** ) image0_data );
    if ( !name )
	setName( "kgLookFeel" );
    resize( 404, 414 ); 
    setCaption( i18n( "Look & Feel" ) );
    setFrameShape( QFrame::NoFrame );
    setFrameShadow( QFrame::MShadow );
    kgLookFeelLayout = new QGridLayout( parent );
    kgLookFeelLayout->setSpacing( 6 );
    kgLookFeelLayout->setMargin( 11 );

    TabWidget2 = new QTabWidget( parent, "TabWidget2" );

    tab = new QWidget( TabWidget2, "tab" );
    tabLayout = new QGridLayout( tab );
    tabLayout->setSpacing( 6 );
    tabLayout->setMargin( 11 );

    GroupBox3 = new QGroupBox( tab, "GroupBox3" );
    GroupBox3->setTitle( i18n( "Look & Feel" ) );
    GroupBox3->setColumnLayout(0, Qt::Vertical );
    GroupBox3->layout()->setSpacing( 0 );
    GroupBox3->layout()->setMargin( 0 );
    GroupBox3Layout = new QGridLayout( GroupBox3->layout() );
    GroupBox3Layout->setAlignment( Qt::AlignTop );
    GroupBox3Layout->setSpacing( 6 );
    GroupBox3Layout->setMargin( 11 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
    GroupBox3Layout->addItem( spacer, 3, 2 );

    TextLabel1 = new QLabel( GroupBox3, "TextLabel1" );
    TextLabel1->setText( i18n( "Panel font:" ) );

    GroupBox3Layout->addWidget( TextLabel1, 3, 0 );

    kgHTMLMinFontSize = new QSpinBox( GroupBox3, "kgHTMLMinFontSize" );
    kgHTMLMinFontSize->setButtonSymbols( QSpinBox::UpDownArrows );
    kgHTMLMinFontSize->setMaxValue( 100 );
    kgHTMLMinFontSize->setMinValue( 1 );
    kgHTMLMinFontSize->setValue( 12 );
    QToolTip::add(  kgHTMLMinFontSize, i18n( "The minimum font size used by the html viewer." ) );

    GroupBox3Layout->addMultiCellWidget( kgHTMLMinFontSize, 5, 5, 2, 3 );

    TextLabel1_2 = new QLabel( GroupBox3, "TextLabel1_2" );
    TextLabel1_2->setText( i18n( "HTML viewer's min font size:" ) );

    GroupBox3Layout->addMultiCellWidget( TextLabel1_2, 5, 5, 0, 1 );

    TextLabel1_3 = new QLabel( GroupBox3, "TextLabel1_3" );
    TextLabel1_3->setText( i18n( "Filelist icon size:" ) );

    GroupBox3Layout->addMultiCellWidget( TextLabel1_3, 4, 4, 0, 1 );

    kgIconSize = new QComboBox( FALSE, GroupBox3, "kgIconSize" );
    kgIconSize->insertItem( i18n( "16" ) );
    kgIconSize->insertItem( i18n( "22" ) );
    kgIconSize->insertItem( i18n( "32" ) );
    kgIconSize->insertItem( i18n( "48" ) );

    GroupBox3Layout->addMultiCellWidget( kgIconSize, 4, 4, 2, 3 );

    kgFontLabel = new QLabel( GroupBox3, "kgFontLabel" );
    kgFontLabel->setText( i18n( "helvetica 12 bold" ) );

    GroupBox3Layout->addWidget( kgFontLabel, 3, 1 );

    kgBrowseFont = new QToolButton( GroupBox3, "kgBrowseFont" );
    kgBrowseFont->setText( "" );
    kgBrowseFont->setPixmap( image0 );

    GroupBox3Layout->addWidget( kgBrowseFont, 3, 3 );

    Layout17_2 = new QGridLayout;
    Layout17_2->setSpacing( 6 );
    Layout17_2->setMargin( 0 );

    kgWarnOnExit = new QCheckBox( GroupBox3, "kgWarnOnExit" );
    kgWarnOnExit->setText( i18n( "Warn on exit" ) );

    Layout17_2->addWidget( kgWarnOnExit, 0, 0 );

    kgShowHidden = new QCheckBox( GroupBox3, "kgShowHidden" );
    kgShowHidden->setText( i18n( "Show hidden files" ) );

    Layout17_2->addWidget( kgShowHidden, 1, 0 );

    kgMarkDirs = new QCheckBox( GroupBox3, "kgMarkDirs" );
    kgMarkDirs->setText( i18n( "Automark directories" ) );

    Layout17_2->addWidget( kgMarkDirs, 1, 1 );

    kgMinimizeToTray = new QCheckBox( GroupBox3, "kgMinimizeToTray" );
    kgMinimizeToTray->setText( i18n( "Minimize to tray" ) );

    Layout17_2->addWidget( kgMinimizeToTray, 0, 1 );

    kgCaseSensativeSort = new QCheckBox( GroupBox3, "kgCaseSensativeSort" );
    kgCaseSensativeSort->setText( i18n( "Case sensitive sorting" ) );

    Layout17_2->addMultiCellWidget( kgCaseSensativeSort, 2, 2, 0, 1 );

    GroupBox3Layout->addMultiCellLayout( Layout17_2, 0, 0, 0, 4 );

    Line1_2 = new QFrame( GroupBox3, "Line1_2" );
    Line1_2->setFrameStyle( QFrame::HLine | QFrame::Sunken );

    GroupBox3Layout->addMultiCellWidget( Line1_2, 1, 2, 0, 4 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupBox3Layout->addMultiCell( spacer_2, 2, 3, 4, 4 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupBox3Layout->addItem( spacer_3, 4, 4 );
    QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupBox3Layout->addItem( spacer_4, 5, 4 );

    TextLabel1_4 = new QLabel( GroupBox3, "TextLabel1_4" );
    TextLabel1_4->setText( i18n( "Mouse Selection Mode:" ) );

    GroupBox3Layout->addMultiCellWidget( TextLabel1_4, 7, 7, 0, 2 );

    Line1_2_2 = new QFrame( GroupBox3, "Line1_2_2" );
    Line1_2_2->setFrameStyle( QFrame::HLine | QFrame::Sunken );

    GroupBox3Layout->addMultiCellWidget( Line1_2_2, 6, 6, 0, 4 );

    ButtonGroup1 = new QButtonGroup( GroupBox3, "ButtonGroup1" );
    ButtonGroup1->setFrameShape( QButtonGroup::NoFrame );
    ButtonGroup1->setTitle( "" );
    ButtonGroup1->setExclusive( TRUE );
    ButtonGroup1->setColumnLayout(0, Qt::Vertical );
    ButtonGroup1->layout()->setSpacing( 0 );
    ButtonGroup1->layout()->setMargin( 0 );
    ButtonGroup1Layout = new QGridLayout( ButtonGroup1->layout() );
    ButtonGroup1Layout->setAlignment( Qt::AlignTop );
    ButtonGroup1Layout->setSpacing( 6 );
    ButtonGroup1Layout->setMargin( 11 );

    kgMouseSelectionClassic = new QRadioButton( ButtonGroup1, "kgMouseSelectionClassic" );
    kgMouseSelectionClassic->setText( i18n( "Classic (both keys combined)" ) );

    ButtonGroup1Layout->addWidget( kgMouseSelectionClassic, 0, 0 );

    kgMouseSelectionLeft = new QRadioButton( ButtonGroup1, "kgMouseSelectionLeft" );
    kgMouseSelectionLeft->setText( i18n( "Left mouse button selects" ) );

    ButtonGroup1Layout->addWidget( kgMouseSelectionLeft, 1, 0 );

    kgMouseSelectionRight = new QRadioButton( ButtonGroup1, "kgMouseSelectionRight" );
    kgMouseSelectionRight->setText( i18n( "Right button selects (Windows Commander style)" ) );

    ButtonGroup1Layout->addWidget( kgMouseSelectionRight, 2, 0 );

    GroupBox3Layout->addMultiCellWidget( ButtonGroup1, 8, 8, 0, 4 );

    tabLayout->addWidget( GroupBox3, 0, 0 );
    TabWidget2->insertTab( tab, i18n( "General" ) );

    tab_2 = new QWidget( TabWidget2, "tab_2" );
    TabWidget2->insertTab( tab_2, i18n( "Toolbar" ) );

    tab_3 = new QWidget( TabWidget2, "tab_3" );
    TabWidget2->insertTab( tab_3, i18n( "Key-bindings" ) );

    kgLookFeelLayout->addWidget( TabWidget2, 0, 0 );

    // signals and slots connections
    connect( kgBrowseFont, SIGNAL( clicked() ), this, SLOT( slotBrowseFont() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
kgLookFeel::~kgLookFeel()
{
    // no need to delete child widgets, Qt does it all for us
}

void kgLookFeel::slotBrowseFont()
{
    qWarning( "kgLookFeel::slotBrowseFont(): Not implemented yet!" );
}

void kgLookFeel::slotApplyChanges()
{
    qWarning( "kgLookFeel::slotApplyChanges(): Not implemented yet!" );
}

void kgLookFeel::slotChangeFont()
{
    qWarning( "kgLookFeel::slotChangeFont(): Not implemented yet!" );
}

void kgLookFeel::slotDefaultSettings()
{
    qWarning( "kgLookFeel::slotDefaultSettings(): Not implemented yet!" );
}

#include "kglookfeel.moc"
