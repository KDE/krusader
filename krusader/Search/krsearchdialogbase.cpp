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

    tab_2 = new QWidget( TabWidget2, "tab_2" );
    tabLayout_2 = new QGridLayout( tab_2 );
    tabLayout_2->setSpacing( 6 );
    tabLayout_2->setMargin( 11 );

    GroupBox29 = new QGroupBox( tab_2, "GroupBox29" );
    GroupBox29->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)1, GroupBox29->sizePolicy().hasHeightForWidth() ) );
    GroupBox29->setTitle( i18n( "Size" ) );

    smallerThanEnabled = new QCheckBox( GroupBox29, "smallerThanEnabled" );
    smallerThanEnabled->setGeometry( QRect( 270, 20, 100, 21 ) );
    smallerThanEnabled->setText( i18n( "&Smaller than" ) );

    smallerThanAmount = new QLineEdit( GroupBox29, "smallerThanAmount" );
    smallerThanAmount->setEnabled( FALSE );
    smallerThanAmount->setGeometry( QRect( 370, 20, 71, 22 ) );
    smallerThanAmount->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, smallerThanAmount->sizePolicy().hasHeightForWidth() ) );

    smallerThanType = new KComboBox( FALSE, GroupBox29, "smallerThanType" );
    smallerThanType->insertItem( i18n( "Bytes" ) );
    smallerThanType->insertItem( i18n( "KB" ) );
    smallerThanType->insertItem( i18n( "MB" ) );
    smallerThanType->setEnabled( FALSE );
    smallerThanType->setGeometry( QRect( 450, 20, 70, 21 ) );

    biggerThanEnabled = new QCheckBox( GroupBox29, "biggerThanEnabled" );
    biggerThanEnabled->setGeometry( QRect( 10, 20, 100, 21 ) );
    biggerThanEnabled->setText( i18n( "&Bigger than" ) );

    biggerThanType = new KComboBox( FALSE, GroupBox29, "biggerThanType" );
    biggerThanType->insertItem( i18n( "Bytes" ) );
    biggerThanType->insertItem( i18n( "KB" ) );
    biggerThanType->insertItem( i18n( "MB" ) );
    biggerThanType->setEnabled( FALSE );
    biggerThanType->setGeometry( QRect( 190, 20, 70, 21 ) );

    biggerThanAmount = new QLineEdit( GroupBox29, "biggerThanAmount" );
    biggerThanAmount->setEnabled( FALSE );
    biggerThanAmount->setGeometry( QRect( 110, 20, 71, 22 ) );
    biggerThanAmount->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, biggerThanAmount->sizePolicy().hasHeightForWidth() ) );

    tabLayout_2->addWidget( GroupBox29, 0, 0 );

    ButtonGroup2 = new QButtonGroup( tab_2, "ButtonGroup2" );
    ButtonGroup2->setTitle( i18n( "Date" ) );
    ButtonGroup2->setExclusive( TRUE );
    ButtonGroup2->setColumnLayout(0, Qt::Vertical );
    ButtonGroup2->layout()->setSpacing( 0 );
    ButtonGroup2->layout()->setMargin( 0 );
    ButtonGroup2Layout = new QGridLayout( ButtonGroup2->layout() );
    ButtonGroup2Layout->setAlignment( Qt::AlignTop );
    ButtonGroup2Layout->setSpacing( 6 );
    ButtonGroup2Layout->setMargin( 11 );

    modifiedBetweenData1 = new QLineEdit( ButtonGroup2, "modifiedBetweenData1" );
    modifiedBetweenData1->setEnabled( FALSE );
    modifiedBetweenData1->setText( "" );

    ButtonGroup2Layout->addMultiCellWidget( modifiedBetweenData1, 0, 0, 2, 3 );

    modifiedBetweenBtn1 = new QToolButton( ButtonGroup2, "modifiedBetweenBtn1" );
    modifiedBetweenBtn1->setEnabled( FALSE );
    modifiedBetweenBtn1->setText( "" );
    modifiedBetweenBtn1->setPixmap( image1 );

    ButtonGroup2Layout->addWidget( modifiedBetweenBtn1, 0, 4 );

    TextLabel2_2 = new QLabel( ButtonGroup2, "TextLabel2_2" );
    TextLabel2_2->setText( i18n( "an&d" ) );

    ButtonGroup2Layout->addWidget( TextLabel2_2, 0, 5 );

    modifiedBetweenData2 = new QLineEdit( ButtonGroup2, "modifiedBetweenData2" );
    modifiedBetweenData2->setEnabled( FALSE );
    modifiedBetweenData2->setText( "" );
    TextLabel2_2->setBuddy(modifiedBetweenData2);

    ButtonGroup2Layout->addWidget( modifiedBetweenData2, 0, 6 );

    modifiedBetweenBtn2 = new QToolButton( ButtonGroup2, "modifiedBetweenBtn2" );
    modifiedBetweenBtn2->setEnabled( FALSE );
    modifiedBetweenBtn2->setText( "" );
    modifiedBetweenBtn2->setPixmap( image1 );

    ButtonGroup2Layout->addWidget( modifiedBetweenBtn2, 0, 7 );

    notModifiedAfterBtn = new QToolButton( ButtonGroup2, "notModifiedAfterBtn" );
    notModifiedAfterBtn->setEnabled( FALSE );
    notModifiedAfterBtn->setText( "" );
    notModifiedAfterBtn->setPixmap( image1 );

    ButtonGroup2Layout->addWidget( notModifiedAfterBtn, 1, 4 );

    notModifiedAfterData = new QLineEdit( ButtonGroup2, "notModifiedAfterData" );
    notModifiedAfterData->setEnabled( FALSE );
    notModifiedAfterData->setText( "" );

    ButtonGroup2Layout->addMultiCellWidget( notModifiedAfterData, 1, 1, 2, 3 );

    modifiedInTheLastEnabled = new QRadioButton( ButtonGroup2, "modifiedInTheLastEnabled" );
    modifiedInTheLastEnabled->setText( i18n("Mod&ified in the last") );
    ButtonGroup2->insert( modifiedInTheLastEnabled, 0 );

    ButtonGroup2Layout->addWidget( modifiedInTheLastEnabled, 2, 0 );

    modifiedInTheLastData = new QLineEdit( ButtonGroup2, "modifiedInTheLastData" );
    modifiedInTheLastData->setEnabled( FALSE );
    modifiedInTheLastData->setText( "" );

    ButtonGroup2Layout->addWidget( modifiedInTheLastData, 2, 2 );

    modifiedInTheLastType = new QComboBox( FALSE, ButtonGroup2, "modifiedInTheLastType" );
    modifiedInTheLastType->insertItem( i18n( "days" ) );
    modifiedInTheLastType->insertItem( i18n( "weeks" ) );
    modifiedInTheLastType->insertItem( i18n( "months" ) );
    modifiedInTheLastType->insertItem( i18n( "years" ) );
    modifiedInTheLastType->setEnabled( FALSE );

    ButtonGroup2Layout->addMultiCellWidget( modifiedInTheLastType, 2, 2, 3, 4 );

    notModifiedInTheLastType = new QComboBox( FALSE, ButtonGroup2, "notModifiedInTheLastType" );
    notModifiedInTheLastType->insertItem( i18n( "days" ) );
    notModifiedInTheLastType->insertItem( i18n( "weeks" ) );
    notModifiedInTheLastType->insertItem( i18n( "months" ) );
    notModifiedInTheLastType->insertItem( i18n( "years" ) );
    notModifiedInTheLastType->setEnabled( FALSE );

    ButtonGroup2Layout->addMultiCellWidget( notModifiedInTheLastType, 3, 3, 3, 4 );

    notModifiedInTheLastData = new QLineEdit( ButtonGroup2, "notModifiedInTheLastData" );
    notModifiedInTheLastData->setEnabled( FALSE );
    notModifiedInTheLastData->setText( "" );

    ButtonGroup2Layout->addWidget( notModifiedInTheLastData, 3, 2 );

    TextLabel3_2 = new QLabel( ButtonGroup2, "TextLabel3_2" );
    TextLabel3_2->setText( i18n( "No&t modified in the last" ) );
    TextLabel3_2->setBuddy(notModifiedInTheLastData);

    ButtonGroup2Layout->addWidget( TextLabel3_2, 3, 0 );

    notModifiedAfterEnabled = new QRadioButton( ButtonGroup2, "notModifiedAfterEnabled" );
    notModifiedAfterEnabled->setText( i18n( "&Not modified after" ) );
    ButtonGroup2->insert( notModifiedAfterEnabled, 0 );

    ButtonGroup2Layout->addMultiCellWidget( notModifiedAfterEnabled, 1, 1, 0, 1 );

    modifiedInTheLast = new QLabel( ButtonGroup2, "modifiedInTheLast" );
    modifiedInTheLast->setText( i18n( "" ) );

    ButtonGroup2Layout->addWidget( modifiedInTheLast, 2, 1 );

    modifiedBetweenEnabled = new QRadioButton( ButtonGroup2, "modifiedBetweenEnabled" );
    modifiedBetweenEnabled->setText( i18n( "&Modified between" ) );
    modifiedInTheLast->setBuddy(modifiedBetweenEnabled);
    ButtonGroup2->insert( modifiedBetweenEnabled, 0 );

    ButtonGroup2Layout->addMultiCellWidget( modifiedBetweenEnabled, 0, 0, 0, 1 );

    tabLayout_2->addWidget( ButtonGroup2, 1, 0 );

    GroupBox181 = new QGroupBox( tab_2, "GroupBox181" );
    GroupBox181->setTitle( i18n( "Ownership" ) );
    GroupBox181->setColumnLayout(0, Qt::Vertical );
    GroupBox181->layout()->setSpacing( 0 );
    GroupBox181->layout()->setMargin( 0 );
    GroupBox181Layout = new QGridLayout( GroupBox181->layout() );
    GroupBox181Layout->setAlignment( Qt::AlignTop );
    GroupBox181Layout->setSpacing( 6 );
    GroupBox181Layout->setMargin( 11 );

    Layout10 = new QHBoxLayout;
    Layout10->setSpacing( 6 );
    Layout10->setMargin( 0 );

    belongsToUserEnabled = new QCheckBox( GroupBox181, "belongsToUserEnabled" );
    belongsToUserEnabled->setText( i18n( "Belongs to &user" ) );
    Layout10->addWidget( belongsToUserEnabled );

    belongsToUserData = new QComboBox( FALSE, GroupBox181, "belongsToUserData" );
    belongsToUserData->setEnabled( FALSE );
    belongsToUserData->setEditable( FALSE );
    Layout10->addWidget( belongsToUserData );

    belongsToGroupEnabled = new QCheckBox( GroupBox181, "belongsToGroupEnabled" );
    belongsToGroupEnabled->setText( i18n( "Belongs to gr&oup" ) );
    Layout10->addWidget( belongsToGroupEnabled );

    belongsToGroupData = new QComboBox( FALSE, GroupBox181, "belongsToGroupData" );
    belongsToGroupData->setEnabled( FALSE );
    belongsToGroupData->setEditable( FALSE );
    Layout10->addWidget( belongsToGroupData );

    GroupBox181Layout->addMultiCellLayout( Layout10, 0, 0, 0, 3 );

    GroupBox206 = new QGroupBox( GroupBox181, "GroupBox206" );
    GroupBox206->setTitle( i18n( "O&wner" ) );

    ownerW = new QComboBox( FALSE, GroupBox206, "ownerW" );
    ownerW->insertItem( i18n( "?" ) );
    ownerW->insertItem( i18n( "w" ) );
    ownerW->insertItem( i18n( "-" ) );
    ownerW->setEnabled( FALSE );
    ownerW->setGeometry( QRect( 50, 20, 40, 22 ) );

    ownerR = new QComboBox( FALSE, GroupBox206, "ownerR" );
    ownerR->insertItem( i18n( "?" ) );
    ownerR->insertItem( i18n( "r" ) );
    ownerR->insertItem( i18n( "-" ) );
    ownerR->setEnabled( FALSE );
    ownerR->setGeometry( QRect( 10, 20, 40, 22 ) );

    ownerX = new QComboBox( FALSE, GroupBox206, "ownerX" );
    ownerX->insertItem( i18n( "?" ) );
    ownerX->insertItem( i18n( "x" ) );
    ownerX->insertItem( i18n( "-" ) );
    ownerX->setEnabled( FALSE );
    ownerX->setGeometry( QRect( 90, 20, 40, 22 ) );

    GroupBox181Layout->addWidget( GroupBox206, 1, 1 );

    permissionsEnabled = new QCheckBox( GroupBox181, "permissionsEnabled" );
    permissionsEnabled->setText( i18n( "P&ermissions" ) );

    GroupBox181Layout->addWidget( permissionsEnabled, 1, 0 );

    GroupBox206_2 = new QGroupBox( GroupBox181, "GroupBox206_2" );
    GroupBox206_2->setTitle( i18n( "Grou&p" ) );

    groupW = new QComboBox( FALSE, GroupBox206_2, "groupW" );
    groupW->insertItem( i18n( "?" ) );
    groupW->insertItem( i18n( "w" ) );
    groupW->insertItem( i18n( "-" ) );
    groupW->setEnabled( FALSE );
    groupW->setGeometry( QRect( 50, 20, 40, 22 ) );

    groupR = new QComboBox( FALSE, GroupBox206_2, "groupR" );
    groupR->insertItem( i18n( "?" ) );
    groupR->insertItem( i18n( "r" ) );
    groupR->insertItem( i18n( "-" ) );
    groupR->setEnabled( FALSE );
    groupR->setGeometry( QRect( 10, 20, 40, 22 ) );

    groupX = new QComboBox( FALSE, GroupBox206_2, "groupX" );
    groupX->insertItem( i18n( "?" ) );
    groupX->insertItem( i18n( "x" ) );
    groupX->insertItem( i18n( "-" ) );
    groupX->setEnabled( FALSE );
    groupX->setGeometry( QRect( 90, 20, 40, 22 ) );

    GroupBox181Layout->addWidget( GroupBox206_2, 1, 2 );

    GroupBox206_3 = new QGroupBox( GroupBox181, "GroupBox206_3" );
    GroupBox206_3->setTitle( i18n( "A&ll" ) );

    allW = new QComboBox( FALSE, GroupBox206_3, "allW" );
    allW->insertItem( i18n( "?" ) );
    allW->insertItem( i18n( "w" ) );
    allW->insertItem( i18n( "-" ) );
    allW->setEnabled( FALSE );
    allW->setGeometry( QRect( 50, 20, 40, 22 ) );

    allX = new QComboBox( FALSE, GroupBox206_3, "allX" );
    allX->insertItem( i18n( "?" ) );
    allX->insertItem( i18n( "x" ) );
    allX->insertItem( i18n( "-" ) );
    allX->setEnabled( FALSE );
    allX->setGeometry( QRect( 90, 20, 40, 22 ) );

    allR = new QComboBox( FALSE, GroupBox206_3, "allR" );
    allR->insertItem( i18n( "?" ) );
    allR->insertItem( i18n( "r" ) );
    allR->insertItem( i18n( "-" ) );
    allR->setEnabled( FALSE );
    allR->setGeometry( QRect( 10, 20, 40, 22 ) );

    GroupBox181Layout->addWidget( GroupBox206_3, 1, 3 );

    TextLabel4 = new QLabel( GroupBox181, "TextLabel4" );
    QFont TextLabel4_font(  TextLabel4->font() );
    TextLabel4_font.setFamily( "adobe-helvetica" );
    TextLabel4_font.setItalic( TRUE );
    TextLabel4->setFont( TextLabel4_font );
    TextLabel4->setText( i18n( "Note: a '?' is a wildcard" ) );

    GroupBox181Layout->addMultiCellWidget( TextLabel4, 2, 2, 0, 3, Qt::AlignRight );

    tabLayout_2->addWidget( GroupBox181, 2, 0 );
    TabWidget2->insertTab( tab_2, i18n( "&Advanced" ) );

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
    connect( biggerThanEnabled, SIGNAL( toggled(bool) ), biggerThanAmount, SLOT( setEnabled(bool) ) );
    connect( biggerThanEnabled, SIGNAL( toggled(bool) ), biggerThanType, SLOT( setEnabled(bool) ) );
    connect( smallerThanEnabled, SIGNAL( toggled(bool) ), smallerThanAmount, SLOT( setEnabled(bool) ) );
    connect( smallerThanEnabled, SIGNAL( toggled(bool) ), smallerThanType, SLOT( setEnabled(bool) ) );
    connect( modifiedBetweenEnabled, SIGNAL( toggled(bool) ), modifiedBetweenData1, SLOT( setEnabled(bool) ) );
    connect( modifiedBetweenEnabled, SIGNAL( toggled(bool) ), modifiedBetweenBtn1, SLOT( setEnabled(bool) ) );
    connect( modifiedBetweenEnabled, SIGNAL( toggled(bool) ), modifiedBetweenData2, SLOT( setEnabled(bool) ) );
    connect( modifiedBetweenEnabled, SIGNAL( toggled(bool) ), modifiedBetweenBtn2, SLOT( setEnabled(bool) ) );
    connect( notModifiedAfterEnabled, SIGNAL( toggled(bool) ), notModifiedAfterData, SLOT( setEnabled(bool) ) );
    connect( notModifiedAfterEnabled, SIGNAL( toggled(bool) ), notModifiedAfterBtn, SLOT( setEnabled(bool) ) );
    connect( modifiedInTheLastEnabled, SIGNAL( toggled(bool) ), modifiedInTheLastData, SLOT( setEnabled(bool) ) );
    connect( modifiedInTheLastEnabled, SIGNAL( toggled(bool) ), modifiedInTheLastType, SLOT( setEnabled(bool) ) );
    connect( modifiedInTheLastEnabled, SIGNAL( toggled(bool) ), notModifiedInTheLastData, SLOT( setEnabled(bool) ) );
    connect( modifiedInTheLastEnabled, SIGNAL( toggled(bool) ), notModifiedInTheLastType, SLOT( setEnabled(bool) ) );
    connect( belongsToUserEnabled, SIGNAL( toggled(bool) ), belongsToUserData, SLOT( setEnabled(bool) ) );
    connect( belongsToGroupEnabled, SIGNAL( toggled(bool) ), belongsToGroupData, SLOT( setEnabled(bool) ) );
    connect( permissionsEnabled, SIGNAL( toggled(bool) ), ownerR, SLOT( setEnabled(bool) ) );
    connect( permissionsEnabled, SIGNAL( toggled(bool) ), ownerW, SLOT( setEnabled(bool) ) );
    connect( permissionsEnabled, SIGNAL( toggled(bool) ), ownerX, SLOT( setEnabled(bool) ) );
    connect( permissionsEnabled, SIGNAL( toggled(bool) ), groupR, SLOT( setEnabled(bool) ) );
    connect( permissionsEnabled, SIGNAL( toggled(bool) ), groupW, SLOT( setEnabled(bool) ) );
    connect( permissionsEnabled, SIGNAL( toggled(bool) ), groupX, SLOT( setEnabled(bool) ) );
    connect( permissionsEnabled, SIGNAL( toggled(bool) ), allR, SLOT( setEnabled(bool) ) );
    connect( permissionsEnabled, SIGNAL( toggled(bool) ), allW, SLOT( setEnabled(bool) ) );
    connect( permissionsEnabled, SIGNAL( toggled(bool) ), allX, SLOT( setEnabled(bool) ) );
    connect( resultsList, SIGNAL( returnPressed(QListViewItem*) ), this, SLOT( resultClicked(QListViewItem*) ) );
    connect( resultsList, SIGNAL( doubleClicked(QListViewItem*) ), this, SLOT( resultClicked(QListViewItem*) ) );
    connect( resultsList, SIGNAL( rightButtonClicked(QListViewItem*,const QPoint&,int) ), this, SLOT( rightClickMenu(QListViewItem*, const QPoint&, int) ) );
    connect( mainCloseBtn, SIGNAL( clicked() ), this, SLOT( closeDialog() ) );
    connect( modifiedBetweenBtn1, SIGNAL( clicked() ), this, SLOT( modifiedBetweenSetDate1() ) );
    connect( modifiedBetweenBtn2, SIGNAL( clicked() ), this, SLOT( modifiedBetweenSetDate2() ) );
    connect( notModifiedAfterBtn, SIGNAL( clicked() ), this, SLOT( notModifiedAfterSetDate() ) );

    // tab order
    setTabOrder( mainSearchBtn, mainCloseBtn );
    setTabOrder( mainCloseBtn, biggerThanEnabled );
    setTabOrder( biggerThanEnabled, biggerThanAmount );
    setTabOrder( biggerThanAmount, smallerThanEnabled );
    setTabOrder( smallerThanEnabled, smallerThanAmount );
    setTabOrder( smallerThanAmount, modifiedBetweenEnabled );
    setTabOrder( modifiedBetweenEnabled, modifiedBetweenData1 );
    setTabOrder( modifiedBetweenData1, modifiedBetweenData2 );
    setTabOrder( modifiedBetweenData2, notModifiedAfterEnabled );
    setTabOrder( notModifiedAfterEnabled, notModifiedAfterData );
    setTabOrder( notModifiedAfterData, modifiedInTheLastEnabled );
    setTabOrder( modifiedInTheLastEnabled, modifiedInTheLastData );
    setTabOrder( modifiedInTheLastData, notModifiedInTheLastData );
    setTabOrder( notModifiedInTheLastData, belongsToUserEnabled );
    setTabOrder( belongsToUserEnabled, belongsToUserData );
    setTabOrder( belongsToUserData, belongsToGroupEnabled );
    setTabOrder( belongsToGroupEnabled, belongsToGroupData );
    setTabOrder( belongsToGroupData, permissionsEnabled );
    setTabOrder( permissionsEnabled, ownerR );
    setTabOrder( ownerR, ownerW );
    setTabOrder( ownerW, ownerX );
    setTabOrder( ownerX, groupR );
    setTabOrder( groupR, groupW );
    setTabOrder( groupW, groupX );
    setTabOrder( groupX, allR );
    setTabOrder( allR, allW );
    setTabOrder( allW, allX );
    setTabOrder( allX, mainStopBtn );
    setTabOrder( mainStopBtn, TabWidget2 );
    setTabOrder( TabWidget2, biggerThanType );
    setTabOrder( biggerThanType, smallerThanType );
    setTabOrder( smallerThanType, modifiedInTheLastType );
    setTabOrder( modifiedInTheLastType, notModifiedInTheLastType );
    setTabOrder( notModifiedInTheLastType, resultsList );
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
    if ( ev->type() == QEvent::ApplicationFontChange ) {
	QFont TextLabel4_font(  TextLabel4->font() );
	TextLabel4_font.setFamily( "adobe-helvetica" );
	TextLabel4_font.setItalic( TRUE );
	TextLabel4->setFont( TextLabel4_font ); 
    }
    return ret;
}

void KrSearchBase::closeDialog()
{
    qWarning( "KrSearchBase::closeDialog(): Not implemented yet!" );
}

void KrSearchBase::modifiedBetweenSetDate1()
{
    qWarning( "KrSearchBase::modifiedBetweenSetDate1(): Not implemented yet!" );
}

void KrSearchBase::modifiedBetweenSetDate2()
{
    qWarning( "KrSearchBase::modifiedBetweenSetDate2(): Not implemented yet!" );
}

void KrSearchBase::notModifiedAfterSetDate()
{
    qWarning( "KrSearchBase::notModifiedAfterSetDate(): Not implemented yet!" );
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
