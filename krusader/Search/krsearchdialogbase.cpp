/****************************************************************************
** Form implementation generated from reading ui file 'krsearchdialogbase.ui'
**
** Created: Wed Oct 24 15:47:21 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "krsearchdialogbase.h"

#include <klocale.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <ksqueezedtextlabel.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include <qtoolbutton.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qlistbox.h>

static const char* const image1_data[] = { 
"16 16 17 1",
". c None",
"# c #000000",
"d c #830000",
"o c #838100",
"e c #838183",
"g c #a4a1a4",
"n c #c50000",
"c c #c55900",
"j c #c5c200",
"h c #c5c2c5",
"k c #ff0000",
"b c #ffaa5a",
"m c #ffc2c5",
"a c #ffdeac",
"i c #ffff00",
"l c #ffffc5",
"f c #ffffff",
".......########.",
"......#abccccdd#",
"......#cceffecc#",
".......#cffffc#.",
".#######cffffc#.",
"#ggggggechffhc#.",
"#ggggggec#ij#c#.",
"#ffffffhc#ij#c#.",
"#fkkkkfhc#lh#c#.",
"#fkfmkfhc#lh#c#.",
"#ffmkkfhchijhn#.",
"#fkfmkfhciijoc#.",
"#fkkkkfhbiijod#.",
"#ffffffhceijecc#",
"#hhhhhhebbcccdd#",
".##############."};

/* 
 *  Constructs a KrSearchBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
KrSearchBase::KrSearchBase( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    QPixmap image1( ( const char** ) image1_data );
    if ( !name )
	setName( "KrSearchBase" );
//    resize( 596, 476 );
//    setMaximumHeight(476); setMaximumWidth(596);
    setCaption( i18n( "Krusader::Search" ) );
    KrSearchBaseLayout = new QGridLayout( this );
    KrSearchBaseLayout->setSpacing( 6 );
    KrSearchBaseLayout->setMargin( 11 );

    Layout9 = new QHBoxLayout;
    Layout9->setSpacing( 6 );
    Layout9->setMargin( 0 );

/*  mainHelpBtn = new QPushButton( this, "mainHelpBtn" );
    mainHelpBtn->setText( i18n( "Help" ) );
    Layout9->addWidget( mainHelpBtn );*/
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout9->addItem( spacer );

    mainSearchBtn = new QPushButton( this, "mainSearchBtn" );
    mainSearchBtn->setText( i18n( "Search" ) );
    mainSearchBtn->setDefault(true);
    Layout9->addWidget( mainSearchBtn );

    mainStopBtn = new QPushButton( this, "mainStopBtn" );
    mainStopBtn->setEnabled( FALSE );
    mainStopBtn->setText( i18n( "Stop" ) );
    Layout9->addWidget( mainStopBtn );

    mainCloseBtn = new QPushButton( this, "mainCloseBtn" );
    mainCloseBtn->setText( i18n( "Close" ) );
    Layout9->addWidget( mainCloseBtn );

    KrSearchBaseLayout->addLayout( Layout9, 1, 0 );

    TabWidget2 = new QTabWidget( this, "TabWidget2" );

    generalFilter = new GeneralFilter( TabWidget2, "generalFilter" );
    TabWidget2->insertTab( generalFilter, i18n( "&General" ) );

    advancedFilter = new AdvancedFilter( TabWidget2, "advancedFilter" );
    TabWidget2->insertTab( advancedFilter, i18n( "&Advanced" ) );
    
    tab_3 = new QWidget( TabWidget2, "tab_3" );
    tabLayout_3 = new QGridLayout( tab_3 );
    tabLayout_3->setSpacing( 6 );
    tabLayout_3->setMargin( 11 );

    Layout11 = new QHBoxLayout;
    Layout11->setSpacing( 6 );
    Layout11->setMargin( 0 );

    foundLabel = new QLabel( tab_3, "foundLabel" );
    foundLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, foundLabel->sizePolicy().hasHeightForWidth() ) );
    foundLabel->setFrameShape( QLabel::StyledPanel );
    foundLabel->setFrameShadow( QLabel::Sunken );
    foundLabel->setText( i18n( "Found 0 matches." ) );
    Layout11->addWidget( foundLabel );

    searchingLabel = new KSqueezedTextLabel( tab_3, "searchingLabel" );
//    searchingLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, searchingLabel->sizePolicy().hasHeightForWidth() ) );
    searchingLabel->setFrameShape( QLabel::StyledPanel );
    searchingLabel->setFrameShadow( QLabel::Sunken );
    searchingLabel->setText( "" );
    Layout11->addWidget( searchingLabel );

    tabLayout_3->addLayout( Layout11, 1, 0 );

    resultsList = new QListView( tab_3, "resultsList" );
    resultsList->addColumn( i18n( "Name" ) );
    resultsList->addColumn( i18n( "Location" ) );
    resultsList->addColumn( i18n( "Size" ) );
    resultsList->addColumn( i18n( "Date" ) );
    resultsList->addColumn( i18n( "Permissions" ) );

    tabLayout_3->addWidget( resultsList, 0, 0 );
    TabWidget2->insertTab( tab_3, i18n( "&Results" ) );

    KrSearchBaseLayout->addWidget( TabWidget2, 0, 0 );

    // signals and slots connections
    connect( mainSearchBtn, SIGNAL( clicked() ), this, SLOT( startSearch() ) );
    connect( mainStopBtn, SIGNAL( clicked() ), this, SLOT( stopSearch() ) );
    connect( resultsList, SIGNAL( returnPressed(QListViewItem*) ), this, SLOT( resultClicked(QListViewItem*) ) );
    connect( resultsList, SIGNAL( doubleClicked(QListViewItem*) ), this, SLOT( resultClicked(QListViewItem*) ) );
    connect( resultsList, SIGNAL( rightButtonClicked(QListViewItem*,const QPoint&,int) ), this, SLOT( rightClickMenu(QListViewItem*, const QPoint&, int) ) );
    connect( mainCloseBtn, SIGNAL( clicked() ), this, SLOT( closeDialog() ) );

    // tab order
    setTabOrder( mainSearchBtn, mainCloseBtn );
    setTabOrder( mainCloseBtn, mainStopBtn );
    setTabOrder( mainStopBtn, TabWidget2 );
    setTabOrder( TabWidget2, resultsList );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
KrSearchBase::~KrSearchBase()
{
    // no need to delete child widgets, Qt does it all for us
}

/*  
 *  Main event handler. Reimplemented to handle application
 *  font changes
 */
bool KrSearchBase::event( QEvent* ev )
{
    bool ret = QDialog::event( ev ); 
//     if ( ev->type() == QEvent::ApplicationFontChange ) {
// 	QFont TextLabel4_font(  TextLabel4->font() );
// 	TextLabel4_font.setFamily( "adobe-helvetica" );
// 	TextLabel4_font.setItalic( TRUE );
// 	TextLabel4->setFont( TextLabel4_font ); 
//     }
    return ret;
}

void KrSearchBase::closeDialog()
{
    qWarning( "KrSearchBase::closeDialog(): Not implemented yet!" );
}

void KrSearchBase::resultClicked(QListViewItem*)
{
    qWarning( "KrSearchBase::resultClicked(QListViewItem*): Not implemented yet!" );
}

void KrSearchBase::rightClickMenu(QListViewItem*, const QPoint&, int)
{
    qWarning( "KrSearchBase::rightClickMenu(QListViewItem*, const QPoint&, int): Not implemented yet!" );
}

void KrSearchBase::startSearch()
{
    qWarning( "KrSearchBase::startSearch(): Not implemented yet!" );
}

void KrSearchBase::stopSearch()
{
    qWarning( "KrSearchBase::stopSearch(): Not implemented yet!" );
}

#include "krsearchdialogbase.moc"
