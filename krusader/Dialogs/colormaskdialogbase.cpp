/****************************************************************************
** Form implementation generated from reading ui file 'colormaskdialog.ui'
**
** Created: Wed Sep 26 21:56:38 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "colormaskdialogbase.h"

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <klocale.h>

static const char* const image0_data[] = { 
"20 20 2 1",
". c #000000",
"# c #0800ff",
"....................",
".##################.",
".##################.",
".##################.",
".##################.",
".##################.",
".##################.",
".##################.",
".##################.",
".##################.",
".##################.",
".##################.",
".##################.",
".##################.",
".##################.",
".##################.",
".##################.",
".##################.",
".##################.",
"...................."};

static const char* const image1_data[] = { 
"20 20 3 1",
". c #000000",
"# c #ff0018",
"a c #ff0020",
"....................",
".##################.",
".#a###a###a###a###a.",
".##################.",
".###a###a###a###a##.",
".##################.",
".#a###a###a###a###a.",
".##################.",
".###a###a###a###a##.",
".##################.",
".#a###a###a###a###a.",
".##################.",
".###a###a###a###a##.",
".##################.",
".#a###a###a###a###a.",
".##################.",
".###a###a###a###a##.",
".##################.",
".#a###a###a###a###a.",
"...................."};

static const char* const image2_data[] = { 
"20 20 3 1",
". c #000000",
"a c #ffee00",
"# c #fff200",
"....................",
".#a#a#a#a#a#a#a#a#a.",
".a#a#a#a#a#a#a#a#a#.",
".#a#a#a#a#a#a#a#a#a.",
".a#a#a#a#a#a#a#a#a#.",
".#a#a#a#a#a#a#a#a#a.",
".a#a#a#a#a#a#a#a#a#.",
".#a#a#a#a#a#a#a#a#a.",
".a#a#a#a#a#a#a#a#a#.",
".#a#a#a#a#a#a#a#a#a.",
".a#a#a#a#a#a#a#a#a#.",
".#a#a#a#a#a#a#a#a#a.",
".a#a#a#a#a#a#a#a#a#.",
".#a#a#a#a#a#a#a#a#a.",
".a#a#a#a#a#a#a#a#a#.",
".#a#a#a#a#a#a#a#a#a.",
".a#a#a#a#a#a#a#a#a#.",
".#a#a#a#a#a#a#a#a#a.",
".a#a#a#a#a#a#a#a#a#.",
"...................."};

static const char* const image3_data[] = { 
"20 20 3 1",
". c #000000",
"# c #00ff20",
"a c #00ff29",
"....................",
".##################.",
".#a#a#a#a#a#a#a#a#a.",
".##################.",
".#a#a#a#a#a#a#a#a#a.",
".##################.",
".#a#a#a#a#a#a#a#a#a.",
".##################.",
".#a#a#a#a#a#a#a#a#a.",
".##################.",
".#a#a#a#a#a#a#a#a#a.",
".##################.",
".#a#a#a#a#a#a#a#a#a.",
".##################.",
".#a#a#a#a#a#a#a#a#a.",
".##################.",
".#a#a#a#a#a#a#a#a#a.",
".##################.",
".#a#a#a#a#a#a#a#a#a.",
"...................."};


/* 
 *  Constructs a colorMaskDialogBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
colorMaskDialogBase::colorMaskDialogBase( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    QPixmap image0( ( const char** ) image0_data );
    QPixmap image1( ( const char** ) image1_data );
    QPixmap image2( ( const char** ) image2_data );
    QPixmap image3( ( const char** ) image3_data );
    if ( !name )
	setName( "colorMaskDialogBase" );
    resize( 434, 239 ); 
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, sizePolicy().hasHeightForWidth() ) );
    setMinimumSize( QSize( 0, 0 ) );
    setMaximumSize( QSize( 1024, 768 ) );
    setCaption( i18n( "Select Compare Mode" ) );
    setSizeGripEnabled( TRUE );
    colorMaskDialogBaseLayout = new QGridLayout( this );
    colorMaskDialogBaseLayout->setSpacing( 6 );
    colorMaskDialogBaseLayout->setMargin( 11 );

    GroupBox1 = new QGroupBox( this, "GroupBox1" );
    GroupBox1->setTitle( i18n( "Left Panel" ) );

    PixmapLabel4 = new QLabel( GroupBox1, "PixmapLabel4" );
    PixmapLabel4->setGeometry( QRect( 160, 110, 20, 20 ) );
    PixmapLabel4->setPixmap( image0 );
    PixmapLabel4->setScaledContents( TRUE );

    PixmapLabel3 = new QLabel( GroupBox1, "PixmapLabel3" );
    PixmapLabel3->setGeometry( QRect( 160, 50, 20, 20 ) );
    PixmapLabel3->setPixmap( image1 );
    PixmapLabel3->setScaledContents( TRUE );

    PixmapLabel5 = new QLabel( GroupBox1, "PixmapLabel5" );
    PixmapLabel5->setGeometry( QRect( 160, 80, 20, 20 ) );
    PixmapLabel5->setPixmap( image2 );
    PixmapLabel5->setScaledContents( TRUE );

    PixmapLabel2 = new QLabel( GroupBox1, "PixmapLabel2" );
    PixmapLabel2->setGeometry( QRect( 160, 20, 20, 20 ) );
    PixmapLabel2->setPixmap( image3 );
    PixmapLabel2->setScaledContents( TRUE );

    identicalLeft = new QCheckBox( GroupBox1, "identicalLeft" );
    identicalLeft->setGeometry( QRect( 10, 80, 140, 20 ) );
    identicalLeft->setText( i18n( "Show identical files" ) );
    identicalLeft->setChecked( TRUE );

    olderLeft = new QCheckBox( GroupBox1, "olderLeft" );
    olderLeft->setGeometry( QRect( 10, 50, 120, 20 ) );
    olderLeft->setText( i18n( "Show older files" ) );
    olderLeft->setChecked( TRUE );

    newerLeft = new QCheckBox( GroupBox1, "newerLeft" );
    newerLeft->setGeometry( QRect( 10, 20, 120, 20 ) );
    newerLeft->setText( i18n( "Show newer files" ) );
    newerLeft->setChecked( TRUE );

    exclusiveLeft = new QCheckBox( GroupBox1, "exclusiveLeft" );
    exclusiveLeft->setGeometry( QRect( 10, 110, 140, 20 ) );
    exclusiveLeft->setText( i18n( "Show exclusive files" ) );
    exclusiveLeft->setChecked( TRUE );

    colorMaskDialogBaseLayout->addWidget( GroupBox1, 0, 0 );

    Layout1 = new QHBoxLayout;
    Layout1->setSpacing( 6 );
    Layout1->setMargin( 0 );

    buttonHelp = new QPushButton( this, "buttonHelp" );
    buttonHelp->setText( i18n( "&Help" ) );
    buttonHelp->setAutoDefault( TRUE );
    Layout1->addWidget( buttonHelp );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( spacer );

    buttonOk = new QPushButton( this, "buttonOk" );
    buttonOk->setText( i18n( "&OK" ) );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    Layout1->addWidget( buttonOk );

    buttonCancel = new QPushButton( this, "buttonCancel" );
    buttonCancel->setText( i18n( "&Cancel" ) );
    buttonCancel->setAutoDefault( TRUE );
    Layout1->addWidget( buttonCancel );

    colorMaskDialogBaseLayout->addMultiCellLayout( Layout1, 2, 2, 0, 1 );

    GroupBox1_2 = new QGroupBox( this, "GroupBox1_2" );
    GroupBox1_2->setTitle( i18n( "Right Panel" ) );

    PixmapLabel3_2 = new QLabel( GroupBox1_2, "PixmapLabel3_2" );
    PixmapLabel3_2->setGeometry( QRect( 160, 50, 20, 20 ) );
    PixmapLabel3_2->setPixmap( image1 );
    PixmapLabel3_2->setScaledContents( TRUE );

    PixmapLabel2_2 = new QLabel( GroupBox1_2, "PixmapLabel2_2" );
    PixmapLabel2_2->setGeometry( QRect( 160, 20, 20, 20 ) );
    PixmapLabel2_2->setPixmap( image3 );
    PixmapLabel2_2->setScaledContents( TRUE );

    PixmapLabel5_2 = new QLabel( GroupBox1_2, "PixmapLabel5_2" );
    PixmapLabel5_2->setGeometry( QRect( 160, 80, 20, 20 ) );
    PixmapLabel5_2->setPixmap( image2 );
    PixmapLabel5_2->setScaledContents( TRUE );

    PixmapLabel4_2 = new QLabel( GroupBox1_2, "PixmapLabel4_2" );
    PixmapLabel4_2->setGeometry( QRect( 160, 110, 20, 20 ) );
    PixmapLabel4_2->setPixmap( image0 );
    PixmapLabel4_2->setScaledContents( TRUE );

    newerRight = new QCheckBox( GroupBox1_2, "newerRight" );
    newerRight->setGeometry( QRect( 10, 20, 120, 20 ) );
    newerRight->setText( i18n( "Show newer files" ) );
    newerRight->setChecked( TRUE );

    olderRight = new QCheckBox( GroupBox1_2, "olderRight" );
    olderRight->setGeometry( QRect( 10, 50, 120, 20 ) );
    olderRight->setText( i18n( "Show older files" ) );
    olderRight->setChecked( TRUE );

    identicalRight = new QCheckBox( GroupBox1_2, "identicalRight" );
    identicalRight->setGeometry( QRect( 10, 80, 140, 20 ) );
    identicalRight->setText( i18n( "Show identical files" ) );
    identicalRight->setChecked( TRUE );

    exclusiveRight = new QCheckBox( GroupBox1_2, "exclusiveRight" );
    exclusiveRight->setGeometry( QRect( 10, 110, 140, 20 ) );
    exclusiveRight->setText( i18n( "Show exclusive files" ) );
    exclusiveRight->setChecked( TRUE );

    colorMaskDialogBaseLayout->addWidget( GroupBox1_2, 0, 1 );

    TextLabel2 = new QLabel( this, "TextLabel2" );
    TextLabel2->setText( i18n( "* Exclusive files don't exist in the other panel.\n"
"* Color masks do not apply to directories" ) );

    colorMaskDialogBaseLayout->addMultiCellWidget( TextLabel2, 1, 1, 0, 1 );

    // signals and slots connections
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
colorMaskDialogBase::~colorMaskDialogBase()
{
    // no need to delete child widgets, Qt does it all for us
}

#include "colormaskdialogbase.moc"
