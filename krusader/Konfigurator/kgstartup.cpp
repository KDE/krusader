/****************************************************************************
** Form implementation generated from reading ui file 'kgstartup.ui'
**
** Created: Mon May 21 16:00:31 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "kgstartup.h"

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
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
 *  Constructs a kgStartup which is a child of 'parent', with the 
 *  name 'name'.' 
 */
kgStartup::kgStartup( QWidget* parent,  const char* name )
    : QFrame( parent, name )
{
    QPixmap image0( ( const char** ) image0_data );
    if ( !name )
	setName( "kgStartup" );
    resize( 511, 396 ); 
    setCaption( i18n( "Krusader's startup state" ) );
    setFrameShape( QFrame::NoFrame );
    setFrameShadow( QFrame::MShadow );
    kgStartupLayout = new QGridLayout( parent );
    kgStartupLayout->setSpacing( 6 );
    kgStartupLayout->setMargin( 11 );

    GroupBox3 = new QGroupBox( parent, "GroupBox3" );
    GroupBox3->setTitle( i18n( "Panels" ) );
    GroupBox3->setColumnLayout(0, Qt::Vertical );
    GroupBox3->layout()->setSpacing( 0 );
    GroupBox3->layout()->setMargin( 0 );
    GroupBox3Layout = new QGridLayout( GroupBox3->layout() );
    GroupBox3Layout->setAlignment( Qt::AlignTop );
    GroupBox3Layout->setSpacing( 6 );
    GroupBox3Layout->setMargin( 11 );

    ButtonGroup1 = new QButtonGroup( GroupBox3, "ButtonGroup1" );
    ButtonGroup1->setFrameShape( QButtonGroup::NoFrame );
    ButtonGroup1->setFrameShadow( QButtonGroup::Plain );
    ButtonGroup1->setTitle( i18n( "" ) );
    ButtonGroup1->setExclusive( TRUE );

    panelsDontSave = new QRadioButton( ButtonGroup1, "panelsDontSave" );
    panelsDontSave->setGeometry( QRect( 3, 28, 250, 19 ) );
    panelsDontSave->setText( i18n( "Start with the following settings:" ) );

    panelsSave = new QRadioButton( ButtonGroup1, "panelsSave" );
    panelsSave->setGeometry( QRect( 3, 3, 194, 19 ) );
    panelsSave->setText( i18n( "Save settings on exit" ) );

    GroupBox3Layout->addMultiCellWidget( ButtonGroup1, 0, 0, 0, 1 );

    panelsTypes = new QComboBox( FALSE, GroupBox3, "panelsTypes" );
    panelsTypes->insertItem( i18n( "List, List" ) );
    panelsTypes->insertItem( i18n( "List, Tree" ) );
    panelsTypes->insertItem( i18n( "List, Quickview" ) );
    panelsTypes->insertItem( i18n( "Tree, List" ) );
    panelsTypes->insertItem( i18n( "Quickview, List" ) );

    GroupBox3Layout->addWidget( panelsTypes, 1, 1 );

    panelsRightType = new QComboBox( FALSE, GroupBox3, "panelsRightType" );
    panelsRightType->insertItem( i18n( "homepage" ) );
    panelsRightType->insertItem( i18n( "work dir" ) );
    panelsRightType->insertItem( i18n( "the last place it was" ) );

    GroupBox3Layout->addWidget( panelsRightType, 3, 1 );

    TextLabel1 = new QLabel( GroupBox3, "TextLabel1" );
    TextLabel1->setText( i18n( "Panel types (left,right):" ) );

    GroupBox3Layout->addWidget( TextLabel1, 1, 0 );

    panelsLeftLabel1 = new QLabel( GroupBox3, "panelsLeftLabel1" );
    panelsLeftLabel1->setText( i18n( "Left panel starts at" ) );

    GroupBox3Layout->addWidget( panelsLeftLabel1, 2, 0 );

    panelsRightLabel1 = new QLabel( GroupBox3, "panelsRightLabel1" );
    panelsRightLabel1->setText( i18n( "Right panel starts at" ) );

    GroupBox3Layout->addWidget( panelsRightLabel1, 3, 0 );

    panelsLeftType = new QComboBox( FALSE, GroupBox3, "panelsLeftType" );
    panelsLeftType->insertItem( i18n( "homepage" ) );
    panelsLeftType->insertItem( i18n( "work dir" ) );
    panelsLeftType->insertItem( i18n( "the last place it was" ) );

    GroupBox3Layout->addWidget( panelsLeftType, 2, 1 );

    panelsRightHomepage = new QLineEdit( GroupBox3, "panelsRightHomepage" );

    GroupBox3Layout->addWidget( panelsRightHomepage, 3, 3 );

    panelsLeftHomepage = new QLineEdit( GroupBox3, "panelsLeftHomepage" );

    GroupBox3Layout->addWidget( panelsLeftHomepage, 2, 3 );

    panelsLeftLabel2 = new QLabel( GroupBox3, "panelsLeftLabel2" );
    panelsLeftLabel2->setText( i18n( "Homepage:" ) );

    GroupBox3Layout->addWidget( panelsLeftLabel2, 2, 2 );

    panelsRightLabel2 = new QLabel( GroupBox3, "panelsRightLabel2" );
    panelsRightLabel2->setText( i18n( "Homepage:" ) );

    GroupBox3Layout->addWidget( panelsRightLabel2, 3, 2 );

    panelsLeftBrowse = new QToolButton( GroupBox3, "panelsLeftBrowse" );
    panelsLeftBrowse->setText( "" );
    panelsLeftBrowse->setPixmap( image0 );

    GroupBox3Layout->addWidget( panelsLeftBrowse, 2, 4 );

    panelsRightBrowse = new QToolButton( GroupBox3, "panelsRightBrowse" );
    panelsRightBrowse->setText("" );
    panelsRightBrowse->setPixmap( image0 );

    GroupBox3Layout->addWidget( panelsRightBrowse, 3, 4 );

    kgStartupLayout->addWidget( GroupBox3, 0, 0 );

    GroupBox20 = new QGroupBox( parent, "GroupBox20" );
    GroupBox20->setTitle( i18n( "User Interface" ) );
    GroupBox20->setColumnLayout(0, Qt::Vertical );
    GroupBox20->layout()->setSpacing( 0 );
    GroupBox20->layout()->setMargin( 0 );
    GroupBox20Layout = new QGridLayout( GroupBox20->layout() );
    GroupBox20Layout->setAlignment( Qt::AlignTop );
    GroupBox20Layout->setSpacing( 6 );
    GroupBox20Layout->setMargin( 11 );

    uiSaveSettings = new QCheckBox( GroupBox20, "uiSaveSettings" );
    uiSaveSettings->setText( i18n( "Save settings on exit" ) );

    GroupBox20Layout->addWidget( uiSaveSettings, 0, 0 );

    uiToolbar = new QCheckBox( GroupBox20, "uiToolbar" );
    uiToolbar->setText( i18n( "Toolbar visible" ) );

    GroupBox20Layout->addWidget( uiToolbar, 1, 0 );

    uiCmdLine = new QCheckBox( GroupBox20, "uiCmdLine" );
    uiCmdLine->setText( i18n( "Command-line visible" ) );

    GroupBox20Layout->addWidget( uiCmdLine, 4, 0 );

    uiTerminalEmulator = new QCheckBox( GroupBox20, "uiTerminalEmulator" );
    uiTerminalEmulator->setText( i18n( "Terminal Emulator visible" ) );

    GroupBox20Layout->addWidget( uiTerminalEmulator, 5, 0 );

    uiPositionSize = new QCheckBox( GroupBox20, "uiPositionSize" );
    uiPositionSize->setText( i18n( "Restore last position and size" ) );

    GroupBox20Layout->addWidget( uiPositionSize, 6, 0 );

    uiFnKeys = new QCheckBox( GroupBox20, "uiFnKeys" );
    uiFnKeys->setText( i18n( "Function keys visible" ) );

    GroupBox20Layout->addWidget( uiFnKeys, 3, 0 );

    uiStatusbar = new QCheckBox( GroupBox20, "uiStatusbar" );
    uiStatusbar->setText( i18n( "Statusbar visible" ) );

    GroupBox20Layout->addWidget( uiStatusbar, 2, 0 );

    kgStartupLayout->addWidget( GroupBox20, 1, 0 );

    // signals and slots connections
    connect( panelsLeftBrowse, SIGNAL( clicked() ), this, SLOT( slotLeftBrowse() ) );
    connect( panelsRightBrowse, SIGNAL( clicked() ), this, SLOT( slotRightBrowse() ) );
    connect( panelsSave, SIGNAL( clicked() ), this, SLOT( slotPanelsSave() ) );
    connect( panelsDontSave, SIGNAL( clicked() ), this, SLOT( slotPanelsDontSave() ) );
    connect( panelsTypes, SIGNAL( activated(const QString&) ), this, SLOT( slotPanelsTypes() ) );
    connect( panelsLeftType, SIGNAL( activated(const QString&) ), this, SLOT( slotPanelsLeftType() ) );
    connect( panelsRightType, SIGNAL( activated(const QString&) ), this, SLOT( slotPanelsRightType() ) );
    connect( uiSaveSettings, SIGNAL( clicked() ), this, SLOT( slotUiSave() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
kgStartup::~kgStartup()
{
    // no need to delete child widgets, Qt does it all for us
}

void kgStartup::slotUiSave()
{
    qWarning( "kgStartup::slotUiSave(): Not implemented yet!" );
}

void kgStartup::slotApplyChanges()
{
    qWarning( "kgStartup::slotApplyChanges(): Not implemented yet!" );
}

void kgStartup::slotDefaultSettings()
{
    qWarning( "kgStartup::slotDefaultSettings(): Not implemented yet!" );
}

void kgStartup::slotLeftBrowse()
{
    qWarning( "kgStartup::slotLeftBrowse(): Not implemented yet!" );
}

void kgStartup::slotPanelsDontSave()
{
    qWarning( "kgStartup::slotPanelsDontSave(): Not implemented yet!" );
}

void kgStartup::slotPanelsLeftType()
{
    qWarning( "kgStartup::slotPanelsLeftType(): Not implemented yet!" );
}

void kgStartup::slotPanelsRightType()
{
    qWarning( "kgStartup::slotPanelsRightType(): Not implemented yet!" );
}

void kgStartup::slotPanelsSave()
{
    qWarning( "kgStartup::slotPanelsSave(): Not implemented yet!" );
}

void kgStartup::slotPanelsTypes()
{
    qWarning( "kgStartup::slotPanelsTypes(): Not implemented yet!" );
}

void kgStartup::slotRightBrowse()
{
    qWarning( "kgStartup::slotRightBrowse(): Not implemented yet!" );
}

#include "kgstartup.moc"
