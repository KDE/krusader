/***************************************************************************
                      advancedfilter.cpp  -  description
                             -------------------
    copyright            : (C) 2003 + by Shie Erlich & Rafi Yanai & Csaba Karai
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

#include "../krusader.h"
#include "advancedfilter.h"
#include "../Dialogs/krdialogs.h"

#include <q3groupbox.h>
//Added by qt3to4:
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPixmap>
#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>
#include <q3buttongroup.h>
#include <qfile.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <time.h>
#include <kiconloader.h>

#define USERSFILE  QString("/etc/passwd")
#define GROUPSFILE QString("/etc/group")

AdvancedFilter::AdvancedFilter ( FilterTabs *tabs, QWidget *parent, const char *name ) : QWidget ( parent, name ), fltTabs ( tabs )
{
	QGridLayout *filterLayout = new QGridLayout ( this );
	filterLayout->setSpacing ( 6 );
	filterLayout->setContentsMargins ( 11, 11, 11, 11 );

	// Options for size

	Q3GroupBox *sizeGroup = new Q3GroupBox ( this, "sizeGroup" );
	sizeGroup->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType ) 5, ( QSizePolicy::SizeType ) 1, sizeGroup->sizePolicy().hasHeightForWidth() ) );
	sizeGroup->setTitle ( i18n ( "Size" ) );
	sizeGroup->setColumnLayout ( 0, Qt::Vertical );
	sizeGroup->layout()->setSpacing ( 0 );
	sizeGroup->layout()->setContentsMargins ( 0, 0, 0, 0 );
	QGridLayout *sizeLayout = new QGridLayout ( sizeGroup->layout() );
	sizeLayout->setAlignment ( Qt::AlignTop );
	sizeLayout->setSpacing ( 6 );
	sizeLayout->setContentsMargins ( 11, 11, 11, 11 );

	biggerThanEnabled = new QCheckBox ( sizeGroup, "biggerThanEnabled" );
	biggerThanEnabled->setText ( i18n ( "&Bigger than" ) );
	sizeLayout->addWidget ( biggerThanEnabled, 0, 0 );

	biggerThanAmount = new QLineEdit ( sizeGroup, "biggerThanAmount" );
	biggerThanAmount->setEnabled ( false );
	biggerThanAmount->setSizePolicy ( QSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Fixed, biggerThanAmount->sizePolicy().hasHeightForWidth() ) );
	sizeLayout->addWidget ( biggerThanAmount, 0, 1 );

	biggerThanType = new KComboBox ( false, sizeGroup );
	biggerThanType->setObjectName ( "biggerThanType" );
	biggerThanType->insertItem ( i18n ( "Bytes" ) );
	biggerThanType->insertItem ( i18n ( "KB" ) );
	biggerThanType->insertItem ( i18n ( "MB" ) );
	biggerThanType->setEnabled ( false );
	sizeLayout->addWidget ( biggerThanType, 0, 2 );

	smallerThanEnabled = new QCheckBox ( sizeGroup, "smallerThanEnabled" );
	smallerThanEnabled->setText ( i18n ( "&Smaller than" ) );
	sizeLayout->addWidget ( smallerThanEnabled, 0, 3 );

	smallerThanAmount = new QLineEdit ( sizeGroup, "smallerThanAmount" );
	smallerThanAmount->setEnabled ( false );
	smallerThanAmount->setSizePolicy ( QSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Fixed, smallerThanAmount->sizePolicy().hasHeightForWidth() ) );
	sizeLayout->addWidget ( smallerThanAmount, 0, 4 );

	smallerThanType = new KComboBox ( false, sizeGroup );
	smallerThanType->setObjectName ( "smallerThanType" );
	smallerThanType->insertItem ( i18n ( "Bytes" ) );
	smallerThanType->insertItem ( i18n ( "KB" ) );
	smallerThanType->insertItem ( i18n ( "MB" ) );
	smallerThanType->setEnabled ( false );
	sizeLayout->addWidget ( smallerThanType, 0, 5 );

	// set a tighter box around the type box

	int height = QFontMetrics ( biggerThanType->font() ).height() +2;
	biggerThanType->setMaximumHeight ( height );
	smallerThanType->setMaximumHeight ( height );

	filterLayout->addWidget ( sizeGroup, 0, 0 );

	// Options for date

	QPixmap iconDate = krLoader->loadIcon ( "date", KIconLoader::Toolbar, 16 );

	Q3ButtonGroup *dateGroup = new Q3ButtonGroup ( this, "dateGroup" );
	dateGroup->setTitle ( i18n ( "Date" ) );
	dateGroup->setExclusive ( true );
	dateGroup->setColumnLayout ( 0, Qt::Vertical );
	dateGroup->layout()->setSpacing ( 0 );
	dateGroup->layout()->setContentsMargins ( 0, 0, 0, 0 );
	QGridLayout *dateLayout = new QGridLayout ( dateGroup->layout() );
	dateLayout->setAlignment ( Qt::AlignTop );
	dateLayout->setSpacing ( 6 );
	dateLayout->setContentsMargins ( 11, 11, 11, 11 );

	modifiedBetweenEnabled = new QRadioButton ( dateGroup, "modifiedBetweenEnabled" );
	modifiedBetweenEnabled->setText ( i18n ( "&Modified between" ) );
	dateGroup->insert ( modifiedBetweenEnabled, 0 );

	dateLayout->addWidget ( modifiedBetweenEnabled, 0, 0, 1, 2 );

	modifiedBetweenData1 = new QLineEdit ( dateGroup, "modifiedBetweenData1" );
	modifiedBetweenData1->setEnabled ( false );
	modifiedBetweenData1->setText ( "" );
	dateLayout->addWidget ( modifiedBetweenData1, 0, 2, 1, 2 );

	modifiedBetweenBtn1 = new QToolButton ( dateGroup, "modifiedBetweenBtn1" );
	modifiedBetweenBtn1->setEnabled ( false );
	modifiedBetweenBtn1->setText ( "" );
	modifiedBetweenBtn1->setPixmap ( iconDate );
	dateLayout->addWidget ( modifiedBetweenBtn1, 0, 4 );

	QLabel *andLabel = new QLabel ( dateGroup, "andLabel" );
	andLabel->setText ( i18n ( "an&d" ) );
	dateLayout->addWidget ( andLabel, 0, 5 );

	modifiedBetweenData2 = new QLineEdit ( dateGroup, "modifiedBetweenData2" );
	modifiedBetweenData2->setEnabled ( false );
	modifiedBetweenData2->setText ( "" );
	andLabel->setBuddy ( modifiedBetweenData2 );
	dateLayout->addWidget ( modifiedBetweenData2, 0, 6 );

	modifiedBetweenBtn2 = new QToolButton ( dateGroup, "modifiedBetweenBtn2" );
	modifiedBetweenBtn2->setEnabled ( false );
	modifiedBetweenBtn2->setText ( "" );
	modifiedBetweenBtn2->setPixmap ( iconDate );
	dateLayout->addWidget ( modifiedBetweenBtn2, 0, 7 );

	notModifiedAfterEnabled = new QRadioButton ( dateGroup, "notModifiedAfterEnabled" );
	notModifiedAfterEnabled->setText ( i18n ( "&Not modified after" ) );
	dateGroup->insert ( notModifiedAfterEnabled, 0 );
	dateLayout->addWidget ( notModifiedAfterEnabled, 1, 0, 1, 2  );

	notModifiedAfterData = new QLineEdit ( dateGroup, "notModifiedAfterData" );
	notModifiedAfterData->setEnabled ( false );
	notModifiedAfterData->setText ( "" );
	dateLayout->addWidget ( notModifiedAfterData, 1, 2, 1, 2 );

	notModifiedAfterBtn = new QToolButton ( dateGroup, "notModifiedAfterBtn" );
	notModifiedAfterBtn->setEnabled ( false );
	notModifiedAfterBtn->setText ( "" );
	notModifiedAfterBtn->setPixmap ( iconDate );
	dateLayout->addWidget ( notModifiedAfterBtn, 1, 4 );

	modifiedInTheLastEnabled = new QRadioButton ( dateGroup, "modifiedInTheLastEnabled" );
	modifiedInTheLastEnabled->setText ( i18n ( "Mod&ified in the last" ) );
	dateGroup->insert ( modifiedInTheLastEnabled, 0 );
	dateLayout->addWidget ( modifiedInTheLastEnabled, 2, 0 );

	modifiedInTheLastData = new QLineEdit ( dateGroup, "modifiedInTheLastData" );
	modifiedInTheLastData->setEnabled ( false );
	modifiedInTheLastData->setText ( "" );
	dateLayout->addWidget ( modifiedInTheLastData, 2, 2 );

	modifiedInTheLastType = new QComboBox ( false, dateGroup, "modifiedInTheLastType" );
	modifiedInTheLastType->insertItem ( i18n ( "days" ) );
	modifiedInTheLastType->insertItem ( i18n ( "weeks" ) );
	modifiedInTheLastType->insertItem ( i18n ( "months" ) );
	modifiedInTheLastType->insertItem ( i18n ( "years" ) );
	modifiedInTheLastType->setEnabled ( false );
	dateLayout->addWidget ( modifiedInTheLastType, 2, 3, 1, 2 );

	notModifiedInTheLastData = new QLineEdit ( dateGroup, "notModifiedInTheLastData" );
	notModifiedInTheLastData->setEnabled ( false );
	notModifiedInTheLastData->setText ( "" );
	dateLayout->addWidget ( notModifiedInTheLastData, 3, 2 );

	QLabel *notModifiedInTheLastLbl = new QLabel ( dateGroup, "notModifiedInTheLastLbl" );
	notModifiedInTheLastLbl->setText ( i18n ( "No&t modified in the last" ) );
	notModifiedInTheLastLbl->setBuddy ( notModifiedInTheLastData );
	dateLayout->addWidget ( notModifiedInTheLastLbl, 3, 0 );

	notModifiedInTheLastType = new QComboBox ( false, dateGroup, "notModifiedInTheLastType" );
	notModifiedInTheLastType->insertItem ( i18n ( "days" ) );
	notModifiedInTheLastType->insertItem ( i18n ( "weeks" ) );
	notModifiedInTheLastType->insertItem ( i18n ( "months" ) );
	notModifiedInTheLastType->insertItem ( i18n ( "years" ) );
	notModifiedInTheLastType->setEnabled ( false );
	dateLayout->addWidget ( notModifiedInTheLastType, 3, 3, 1, 2 );

	filterLayout->addWidget ( dateGroup, 1, 0 );

	// Options for ownership

	Q3GroupBox *ownershipGroup = new Q3GroupBox ( this, "ownershipGroup" );
	ownershipGroup->setTitle ( i18n ( "Ownership" ) );
	ownershipGroup->setColumnLayout ( 0, Qt::Vertical );
	ownershipGroup->layout()->setSpacing ( 0 );
	ownershipGroup->layout()->setContentsMargins ( 0, 0, 0, 0 );
	QGridLayout *ownershipLayout = new QGridLayout ( ownershipGroup->layout() );
	ownershipLayout->setAlignment ( Qt::AlignTop );
	ownershipLayout->setSpacing ( 6 );
	ownershipLayout->setContentsMargins ( 11, 11, 11, 11 );

	QHBoxLayout *hboxLayout = new QHBoxLayout();
	hboxLayout->setSpacing ( 6 );
	hboxLayout->setContentsMargins ( 0, 0, 0, 0 );

	belongsToUserEnabled = new QCheckBox ( ownershipGroup, "belongsToUserEnabled" );
	belongsToUserEnabled->setText ( i18n ( "Belongs to &user" ) );
	hboxLayout->addWidget ( belongsToUserEnabled );

	belongsToUserData = new QComboBox ( false, ownershipGroup, "belongsToUserData" );
	belongsToUserData->setEnabled ( false );
	belongsToUserData->setEditable ( false );
	hboxLayout->addWidget ( belongsToUserData );

	belongsToGroupEnabled = new QCheckBox ( ownershipGroup, "belongsToGroupEnabled" );
	belongsToGroupEnabled->setText ( i18n ( "Belongs to gr&oup" ) );
	hboxLayout->addWidget ( belongsToGroupEnabled );

	belongsToGroupData = new QComboBox ( false, ownershipGroup, "belongsToGroupData" );
	belongsToGroupData->setEnabled ( false );
	belongsToGroupData->setEditable ( false );
	hboxLayout->addWidget ( belongsToGroupData );

	ownershipLayout->addLayout ( hboxLayout, 0, 0, 1, 4 );

	permissionsEnabled = new QCheckBox ( ownershipGroup, "permissionsEnabled" );
	permissionsEnabled->setText ( i18n ( "P&ermissions" ) );
	ownershipLayout->addWidget ( permissionsEnabled, 1, 0 );

	Q3GroupBox *ownerGroup = new Q3GroupBox ( ownershipGroup, "ownerGroup" );
	QHBoxLayout *ownerHBox = new QHBoxLayout( ownerGroup );
	ownerGroup->setTitle ( i18n ( "O&wner" ) );
	int width = 2*height + height / 2;

	ownerR = new QComboBox ( false, ownerGroup, "ownerR" );
	ownerR->insertItem ( i18n ( "?" ) );
	ownerR->insertItem ( i18n ( "r" ) );
	ownerR->insertItem ( i18n ( "-" ) );
	ownerR->setEnabled ( false );
	ownerR->setGeometry ( QRect ( 10, 20, width, height+6 ) );
	ownerHBox->addWidget( ownerR );

	ownerW = new QComboBox ( false, ownerGroup, "ownerW" );
	ownerW->insertItem ( i18n ( "?" ) );
	ownerW->insertItem ( i18n ( "w" ) );
	ownerW->insertItem ( i18n ( "-" ) );
	ownerW->setEnabled ( false );
	ownerW->setGeometry ( QRect ( 10 + width, 20, width, height+6 ) );
	ownerHBox->addWidget( ownerW );

	ownerX = new QComboBox ( false, ownerGroup, "ownerX" );
	ownerX->insertItem ( i18n ( "?" ) );
	ownerX->insertItem ( i18n ( "x" ) );
	ownerX->insertItem ( i18n ( "-" ) );
	ownerX->setEnabled ( false );
	ownerX->setGeometry ( QRect ( 10 + 2*width, 20, width, height+6 ) );
	ownerHBox->addWidget( ownerX );

	ownershipLayout->addWidget ( ownerGroup, 1, 1 );

	Q3GroupBox *groupGroup = new Q3GroupBox ( ownershipGroup, "groupGroup" );
	QHBoxLayout *groupHBox = new QHBoxLayout( groupGroup );
	groupGroup->setTitle ( i18n ( "Grou&p" ) );

	groupR = new QComboBox ( false, groupGroup, "groupR" );
	groupR->insertItem ( i18n ( "?" ) );
	groupR->insertItem ( i18n ( "r" ) );
	groupR->insertItem ( i18n ( "-" ) );
	groupR->setEnabled ( false );
	groupR->setGeometry ( QRect ( 10, 20, width, height+6 ) );
	groupHBox->addWidget( groupR );

	groupW = new QComboBox ( false, groupGroup, "groupW" );
	groupW->insertItem ( i18n ( "?" ) );
	groupW->insertItem ( i18n ( "w" ) );
	groupW->insertItem ( i18n ( "-" ) );
	groupW->setEnabled ( false );
	groupW->setGeometry ( QRect ( 10 + width, 20, width, height+6 ) );
	groupHBox->addWidget( groupW );

	groupX = new QComboBox ( false, groupGroup, "groupX" );
	groupX->insertItem ( i18n ( "?" ) );
	groupX->insertItem ( i18n ( "x" ) );
	groupX->insertItem ( i18n ( "-" ) );
	groupX->setEnabled ( false );
	groupX->setGeometry ( QRect ( 10 + 2*width, 20, width, height+6 ) );
	groupHBox->addWidget( groupX );

	ownershipLayout->addWidget ( groupGroup, 1, 2 );

	Q3GroupBox *allGroup = new Q3GroupBox ( ownershipGroup, "allGroup" );
	QHBoxLayout *allHBox = new QHBoxLayout( allGroup );
	allGroup->setTitle ( i18n ( "A&ll" ) );

	allR = new QComboBox ( false, allGroup, "allR" );
	allR->insertItem ( i18n ( "?" ) );
	allR->insertItem ( i18n ( "r" ) );
	allR->insertItem ( i18n ( "-" ) );
	allR->setEnabled ( false );
	allR->setGeometry ( QRect ( 10, 20, width, height+6 ) );
	allHBox->addWidget( allR );

	allW = new QComboBox ( false, allGroup, "allW" );
	allW->insertItem ( i18n ( "?" ) );
	allW->insertItem ( i18n ( "w" ) );
	allW->insertItem ( i18n ( "-" ) );
	allW->setEnabled ( false );
	allW->setGeometry ( QRect ( 10 + width, 20, width, height+6 ) );
	allHBox->addWidget( allW );

	allX = new QComboBox ( false, allGroup, "allX" );
	allX->insertItem ( i18n ( "?" ) );
	allX->insertItem ( i18n ( "x" ) );
	allX->insertItem ( i18n ( "-" ) );
	allX->setEnabled ( false );
	allX->setGeometry ( QRect ( 10 + 2*width, 20, width, height+6 ) );
	allHBox->addWidget( allX );

	ownershipLayout->addWidget ( allGroup, 1, 3 );

	QLabel *infoLabel = new QLabel ( ownershipGroup, "TextLabel4" );
	QFont infoLabel_font ( infoLabel->font() );
	infoLabel_font.setFamily ( "adobe-helvetica" );
	infoLabel_font.setItalic ( true );
	infoLabel->setFont ( infoLabel_font );
	infoLabel->setText ( i18n ( "Note: a '?' is a wildcard" ) );

	ownershipLayout->addWidget ( infoLabel, 2, 0, 1, 4, Qt::AlignRight );

	filterLayout->addWidget ( ownershipGroup, 2, 0 );

	// Connection table

	connect ( biggerThanEnabled, SIGNAL ( toggled ( bool ) ), biggerThanAmount, SLOT ( setEnabled ( bool ) ) );
	connect ( biggerThanEnabled, SIGNAL ( toggled ( bool ) ), biggerThanType, SLOT ( setEnabled ( bool ) ) );
	connect ( smallerThanEnabled, SIGNAL ( toggled ( bool ) ), smallerThanAmount, SLOT ( setEnabled ( bool ) ) );
	connect ( smallerThanEnabled, SIGNAL ( toggled ( bool ) ), smallerThanType, SLOT ( setEnabled ( bool ) ) );
	connect ( modifiedBetweenEnabled, SIGNAL ( toggled ( bool ) ), modifiedBetweenData1, SLOT ( setEnabled ( bool ) ) );
	connect ( modifiedBetweenEnabled, SIGNAL ( toggled ( bool ) ), modifiedBetweenBtn1, SLOT ( setEnabled ( bool ) ) );
	connect ( modifiedBetweenEnabled, SIGNAL ( toggled ( bool ) ), modifiedBetweenData2, SLOT ( setEnabled ( bool ) ) );
	connect ( modifiedBetweenEnabled, SIGNAL ( toggled ( bool ) ), modifiedBetweenBtn2, SLOT ( setEnabled ( bool ) ) );
	connect ( notModifiedAfterEnabled, SIGNAL ( toggled ( bool ) ), notModifiedAfterData, SLOT ( setEnabled ( bool ) ) );
	connect ( notModifiedAfterEnabled, SIGNAL ( toggled ( bool ) ), notModifiedAfterBtn, SLOT ( setEnabled ( bool ) ) );
	connect ( modifiedInTheLastEnabled, SIGNAL ( toggled ( bool ) ), modifiedInTheLastData, SLOT ( setEnabled ( bool ) ) );
	connect ( modifiedInTheLastEnabled, SIGNAL ( toggled ( bool ) ), modifiedInTheLastType, SLOT ( setEnabled ( bool ) ) );
	connect ( modifiedInTheLastEnabled, SIGNAL ( toggled ( bool ) ), notModifiedInTheLastData, SLOT ( setEnabled ( bool ) ) );
	connect ( modifiedInTheLastEnabled, SIGNAL ( toggled ( bool ) ), notModifiedInTheLastType, SLOT ( setEnabled ( bool ) ) );
	connect ( belongsToUserEnabled, SIGNAL ( toggled ( bool ) ), belongsToUserData, SLOT ( setEnabled ( bool ) ) );
	connect ( belongsToGroupEnabled, SIGNAL ( toggled ( bool ) ), belongsToGroupData, SLOT ( setEnabled ( bool ) ) );
	connect ( permissionsEnabled, SIGNAL ( toggled ( bool ) ), ownerR, SLOT ( setEnabled ( bool ) ) );
	connect ( permissionsEnabled, SIGNAL ( toggled ( bool ) ), ownerW, SLOT ( setEnabled ( bool ) ) );
	connect ( permissionsEnabled, SIGNAL ( toggled ( bool ) ), ownerX, SLOT ( setEnabled ( bool ) ) );
	connect ( permissionsEnabled, SIGNAL ( toggled ( bool ) ), groupR, SLOT ( setEnabled ( bool ) ) );
	connect ( permissionsEnabled, SIGNAL ( toggled ( bool ) ), groupW, SLOT ( setEnabled ( bool ) ) );
	connect ( permissionsEnabled, SIGNAL ( toggled ( bool ) ), groupX, SLOT ( setEnabled ( bool ) ) );
	connect ( permissionsEnabled, SIGNAL ( toggled ( bool ) ), allR, SLOT ( setEnabled ( bool ) ) );
	connect ( permissionsEnabled, SIGNAL ( toggled ( bool ) ), allW, SLOT ( setEnabled ( bool ) ) );
	connect ( permissionsEnabled, SIGNAL ( toggled ( bool ) ), allX, SLOT ( setEnabled ( bool ) ) );
	connect ( modifiedBetweenBtn1, SIGNAL ( clicked() ), this, SLOT ( modifiedBetweenSetDate1() ) );
	connect ( modifiedBetweenBtn2, SIGNAL ( clicked() ), this, SLOT ( modifiedBetweenSetDate2() ) );
	connect ( notModifiedAfterBtn, SIGNAL ( clicked() ), this, SLOT ( notModifiedAfterSetDate() ) );

	// fill the users and groups list

	fillList ( belongsToUserData, USERSFILE );
	fillList ( belongsToGroupData, GROUPSFILE );

	// tab order
	setTabOrder ( biggerThanEnabled, biggerThanAmount );
	setTabOrder ( biggerThanAmount, smallerThanEnabled );
	setTabOrder ( smallerThanEnabled, smallerThanAmount );
	setTabOrder ( smallerThanAmount, modifiedBetweenEnabled );
	setTabOrder ( modifiedBetweenEnabled, modifiedBetweenData1 );
	setTabOrder ( modifiedBetweenData1, modifiedBetweenData2 );
	setTabOrder ( modifiedBetweenData2, notModifiedAfterEnabled );
	setTabOrder ( notModifiedAfterEnabled, notModifiedAfterData );
	setTabOrder ( notModifiedAfterData, modifiedInTheLastEnabled );
	setTabOrder ( modifiedInTheLastEnabled, modifiedInTheLastData );
	setTabOrder ( modifiedInTheLastData, notModifiedInTheLastData );
	setTabOrder ( notModifiedInTheLastData, belongsToUserEnabled );
	setTabOrder ( belongsToUserEnabled, belongsToUserData );
	setTabOrder ( belongsToUserData, belongsToGroupEnabled );
	setTabOrder ( belongsToGroupEnabled, belongsToGroupData );
	setTabOrder ( belongsToGroupData, permissionsEnabled );
	setTabOrder ( permissionsEnabled, ownerR );
	setTabOrder ( ownerR, ownerW );
	setTabOrder ( ownerW, ownerX );
	setTabOrder ( ownerX, groupR );
	setTabOrder ( groupR, groupW );
	setTabOrder ( groupW, groupX );
	setTabOrder ( groupX, allR );
	setTabOrder ( allR, allW );
	setTabOrder ( allW, allX );
	setTabOrder ( allX, biggerThanType );
	setTabOrder ( biggerThanType, smallerThanType );
	setTabOrder ( smallerThanType, modifiedInTheLastType );
	setTabOrder ( modifiedInTheLastType, notModifiedInTheLastType );
}

void AdvancedFilter::modifiedBetweenSetDate1()
{
	changeDate ( modifiedBetweenData1 );
}

void AdvancedFilter::modifiedBetweenSetDate2()
{
	changeDate ( modifiedBetweenData2 );
}

void AdvancedFilter::notModifiedAfterSetDate()
{
	changeDate ( notModifiedAfterData );
}

void AdvancedFilter::changeDate ( QLineEdit *p )
{
	// check if the current date is valid
	QDate d = KGlobal::locale()->readDate ( p->text() );
	if ( !d.isValid() ) d = QDate::currentDate();

	KRGetDate *gd = new KRGetDate ( d, this );
	d = gd->getDate();
	// if a user pressed ESC or closed the dialog, we'll return an invalid date
	if ( d.isValid() )
		p->setText ( KGlobal::locale()->formatDate ( d, KLocale::ShortDate ) );
	delete gd;
}

// bool start: set it to true if this date is the beginning of the search,
// if it's the end, set it to false
void AdvancedFilter::qdate2time_t ( time_t *dest, QDate d, bool start )
{
	struct tm t;
	t.tm_sec   = ( start ? 0 : 59 );
	t.tm_min   = ( start ? 0 : 59 );
	t.tm_hour  = ( start ? 0 : 23 );
	t.tm_mday  = d.day();
	t.tm_mon   = d.month() - 1;
	t.tm_year  = d.year() - 1900;
	t.tm_wday  = d.dayOfWeek() - 1; // actually ignored by mktime
	t.tm_yday  = d.dayOfYear() - 1; // actually ignored by mktime
	t.tm_isdst = -1; // daylight saving time information isn't availble

	( *dest ) = mktime ( &t );
}


void AdvancedFilter::fillList ( QComboBox *list, QString filename )
{
	QFile data ( filename );
	if ( !data.open ( QIODevice::ReadOnly ) )
	{
		krOut << "Search: Unable to read " << filename << " !!!" << endl;
		return;
	}
	// and read it into the temporary array
	Q3TextStream t ( &data );
	while ( !data.atEnd() )
	{
		QString s = t.readLine();
		QString name = s.left ( s.find ( ':' ) );
		list->insertItem ( name );
	}
}

void AdvancedFilter::invalidDateMessage ( QLineEdit *p )
{
	// FIXME p->text() is empty sometimes (to reproduce, set date to "13.09.005")
	KMessageBox::detailedError ( this, i18n ( "Invalid date entered." ),
	                             i18n ( "The date %1 is not valid according to your locale. Please re-enter a valid date (use the date button for easy access).", p->text() ) );
	p->setFocus();
}

bool AdvancedFilter::fillQuery ( KRQuery *query )
{
	KIO::filesize_t minSize = 0, maxSize = 0;

	// size calculations ////////////////////////////////////////////////
	if ( biggerThanEnabled->isChecked() &&
	        ! ( biggerThanAmount->text().simplified() ).isEmpty() )
	{
		minSize = biggerThanAmount->text().toULong();
		switch ( biggerThanType->currentItem() )
		{
			case 1 : minSize *= 1024;
				break;
			case 2 : minSize *= ( 1024*1024 );
				break;
		}
		query->setMinimumFileSize ( minSize );
	}
	if ( smallerThanEnabled->isChecked() &&
	        ! ( smallerThanAmount->text().simplified() ).isEmpty() )
	{
		maxSize = smallerThanAmount->text().toULong();
		switch ( smallerThanType->currentItem() )
		{
			case 1 : maxSize *= 1024;
				break;
			case 2 : maxSize *= ( 1024*1024 );
				break;
		}
		query->setMaximumFileSize ( maxSize );
	}
	// check that minSize is smaller than maxSize
	if ( ( minSize > 0 ) && ( maxSize > 0 ) && ( maxSize < minSize ) )
	{
		KMessageBox::detailedError ( this, i18n ( "Specified sizes are inconsistent!" ),
		                             i18n ( "Please re-enter the values, so that the left side size will be smaller than (or equal to) the right side size." ) );
		biggerThanAmount->setFocus();
		return false;
	}

	// date calculations ////////////////////////////////////////////////////
	if ( modifiedBetweenEnabled->isChecked() )
	{
		// first, if both dates are empty, than don't use them
		if ( ! ( modifiedBetweenData1->text().simplified().isEmpty() &&
		         modifiedBetweenData2->text().simplified().isEmpty() ) )
		{
			// check if date is valid
			QDate d1 = KGlobal::locale()->readDate ( modifiedBetweenData1->text() );
			if ( !d1.isValid() ) { invalidDateMessage ( modifiedBetweenData1 ); return false; }
			QDate d2 = KGlobal::locale()->readDate ( modifiedBetweenData2->text() );
			if ( !d2.isValid() ) { invalidDateMessage ( modifiedBetweenData2 ); return false; }

			if ( d1 > d2 )
			{
				KMessageBox::detailedError ( this, i18n ( "Dates are inconsistent!" ),
				                             i18n ( "The date on the left is later than the date on the right. Please re-enter the dates, so that the left side date will be earlier than the right side date." ) );
				modifiedBetweenData1->setFocus();
				return false;
			}
			// all seems to be ok, create time_t

			time_t newerTime, olderTime;
			qdate2time_t ( &newerTime, d1, true );
			qdate2time_t ( &olderTime, d2, false );
			query->setNewerThan ( newerTime );
			query->setOlderThan ( olderTime );
		}
	}
	else if ( notModifiedAfterEnabled->isChecked() )
	{
		if ( !notModifiedAfterData->text().simplified().isEmpty() )
		{
			QDate d = KGlobal::locale()->readDate ( notModifiedAfterData->text() );
			if ( !d.isValid() ) { invalidDateMessage ( notModifiedAfterData ); return false; }
			time_t olderTime;
			qdate2time_t ( &olderTime, d, false );
			query->setOlderThan ( olderTime );
		}
	}
	else if ( modifiedInTheLastEnabled->isChecked() )
	{
		if ( ! ( modifiedInTheLastData->text().simplified().isEmpty() &&
		         notModifiedInTheLastData->text().simplified().isEmpty() ) )
		{
			QDate d1 = QDate::currentDate(); QDate d2 = QDate::currentDate();
			if ( !modifiedInTheLastData->text().simplified().isEmpty() )
			{
				int tmp1 = modifiedInTheLastData->text().simplified().toInt();
				switch ( modifiedInTheLastType->currentItem() )
				{
					case 1 : tmp1 *= 7;
						break;
					case 2 : tmp1 *= 30;
						break;
					case 3 : tmp1 *= 365;
						break;
				}
				d1 = d1.addDays ( ( -1 ) * tmp1 );
				time_t newerTime;
				qdate2time_t ( &newerTime, d1, true );
				query->setNewerThan ( newerTime );
			}
			if ( !notModifiedInTheLastData->text().simplified().isEmpty() )
			{
				int tmp2 = notModifiedInTheLastData->text().simplified().toInt();
				switch ( notModifiedInTheLastType->currentItem() )
				{
					case 1 : tmp2 *= 7;
						break;
					case 2 : tmp2 *= 30;
						break;
					case 3 : tmp2 *= 365;
						break;
				}
				d2 = d2.addDays ( ( -1 ) * tmp2 );
				time_t olderTime;
				qdate2time_t ( &olderTime, d2, true );
				query->setOlderThan ( olderTime );
			}
			if ( !modifiedInTheLastData->text().simplified().isEmpty() &&
			        !notModifiedInTheLastData->text().simplified().isEmpty() )
			{
				if ( d1 > d2 )
				{
					KMessageBox::detailedError ( this, i18n ( "Dates are inconsistent!" ),
					                             i18n ( "The date on top is later than the date on the bottom. Please re-enter the dates, so that the top date will be earlier than the bottom date." ) );
					modifiedInTheLastData->setFocus();
					return false;
				}
			}
		}
	}
	// permissions and ownership /////////////////////////////////////
	if ( permissionsEnabled->isChecked() )
	{
		QString perm = ownerR->currentText() + ownerW->currentText() + ownerX->currentText() +
		               groupR->currentText() + groupW->currentText() + groupX->currentText() +
		               allR->currentText()   + allW->currentText()   + allX->currentText();
		query->setPermissions ( perm );
	}
	if ( belongsToUserEnabled->isChecked() )
		query->setOwner ( belongsToUserData->currentText() );
	if ( belongsToGroupEnabled->isChecked() )
		query->setGroup ( belongsToGroupData->currentText() );

	return true;
}

void AdvancedFilter::loadFromProfile ( QString name )
{
	KConfigGroup cfg( krConfig, name);

	smallerThanEnabled->setChecked ( cfg.readEntry ( "Smaller Than Enabled", false ) );
	smallerThanAmount->setText ( cfg.readEntry ( "Smaller Than Amount", "" ) );
	smallerThanType->setCurrentIndex ( cfg.readEntry ( "Smaller Than Type", 0 ) );

	biggerThanEnabled->setChecked ( cfg.readEntry ( "Bigger Than Enabled", false ) );
	biggerThanAmount->setText ( cfg.readEntry ( "Bigger Than Amount", "" ) );
	biggerThanType->setCurrentIndex ( cfg.readEntry ( "Bigger Than Type", 0 ) );

	modifiedBetweenEnabled->setChecked ( cfg.readEntry ( "Modified Between Enabled", false ) );
	notModifiedAfterEnabled->setChecked ( cfg.readEntry ( "Not Modified After Enabled", false ) );
	modifiedInTheLastEnabled->setChecked ( cfg.readEntry ( "Modified In The Last Enabled", false ) );

	modifiedBetweenData1->setText ( cfg.readEntry ( "Modified Between 1", "" ) );
	modifiedBetweenData2->setText ( cfg.readEntry ( "Modified Between 2", "" ) );

	notModifiedAfterData->setText ( cfg.readEntry ( "Not Modified After", "" ) );
	modifiedInTheLastData->setText ( cfg.readEntry ( "Modified In The Last", "" ) );
	notModifiedInTheLastData->setText ( cfg.readEntry ( "Not Modified In The Last", "" ) );

	modifiedInTheLastType->setCurrentIndex ( cfg.readEntry ( "Modified In The Last Type", 0 ) );
	notModifiedInTheLastType->setCurrentIndex ( cfg.readEntry ( "Not Modified In The Last Type", 0 ) );

	belongsToUserEnabled->setChecked ( cfg.readEntry ( "Belongs To User Enabled", false ) );
	belongsToGroupEnabled->setChecked ( cfg.readEntry ( "Belongs To Group Enabled", false ) );

	QString user = cfg.readEntry ( "Belongs To User", "" );
	for ( int i = belongsToUserData->count(); i >= 0; i-- )
	{
		belongsToUserData->setCurrentIndex ( i );
		if ( belongsToUserData->currentText() == user )
			break;
	}

	QString group = cfg.readEntry ( "Belongs To Group", "" );
	for ( int i = belongsToGroupData->count(); i >= 0; i-- )
	{
		belongsToGroupData->setCurrentIndex ( i );
		if ( belongsToGroupData->currentText() == group )
			break;
	}

	permissionsEnabled->setChecked ( cfg.readEntry ( "Permissions Enabled", false ) );

	ownerW->setCurrentIndex ( cfg.readEntry ( "Owner Write", 0 ) );
	ownerR->setCurrentIndex ( cfg.readEntry ( "Owner Read", 0 ) );
	ownerX->setCurrentIndex ( cfg.readEntry ( "Owner Execute", 0 ) );
	groupW->setCurrentIndex ( cfg.readEntry ( "Group Write", 0 ) );
	groupR->setCurrentIndex ( cfg.readEntry ( "Group Read", 0 ) );
	groupX->setCurrentIndex ( cfg.readEntry ( "Group Execute", 0 ) );
	allW->setCurrentIndex ( cfg.readEntry ( "All Write", 0 ) );
	allR->setCurrentIndex ( cfg.readEntry ( "All Read", 0 ) );
	allX->setCurrentIndex ( cfg.readEntry ( "All Execute", 0 ) );
}

void AdvancedFilter::saveToProfile ( QString name )
{
	KConfigGroup group( krConfig, name );

	group.writeEntry ( "Smaller Than Enabled", smallerThanEnabled->isChecked() );
	group.writeEntry ( "Smaller Than Amount", smallerThanAmount->text() );
	group.writeEntry ( "Smaller Than Type", smallerThanType->currentItem() );

	group.writeEntry ( "Bigger Than Enabled", biggerThanEnabled->isChecked() );
	group.writeEntry ( "Bigger Than Amount", biggerThanAmount->text() );
	group.writeEntry ( "Bigger Than Type", biggerThanType->currentItem() );

	group.writeEntry ( "Modified Between Enabled", modifiedBetweenEnabled->isChecked() );
	group.writeEntry ( "Not Modified After Enabled", notModifiedAfterEnabled->isChecked() );
	group.writeEntry ( "Modified In The Last Enabled", modifiedInTheLastEnabled->isChecked() );

	group.writeEntry ( "Modified Between 1", modifiedBetweenData1->text() );
	group.writeEntry ( "Modified Between 2", modifiedBetweenData2->text() );

	group.writeEntry ( "Not Modified After", notModifiedAfterData->text() );
	group.writeEntry ( "Modified In The Last", modifiedInTheLastData->text() );
	group.writeEntry ( "Not Modified In The Last", notModifiedInTheLastData->text() );

	group.writeEntry ( "Modified In The Last Type", modifiedInTheLastType->currentItem() );
	group.writeEntry ( "Not Modified In The Last Type", notModifiedInTheLastType->currentItem() );

	group.writeEntry ( "Belongs To User Enabled", belongsToUserEnabled->isChecked() );
	group.writeEntry ( "Belongs To Group Enabled", belongsToGroupEnabled->isChecked() );

	group.writeEntry ( "Belongs To User", belongsToUserData->currentText() );
	group.writeEntry ( "Belongs To Group", belongsToGroupData->currentText() );

	group.writeEntry ( "Permissions Enabled", permissionsEnabled->isChecked() );

	group.writeEntry ( "Owner Write", ownerW->currentItem() );
	group.writeEntry ( "Owner Read", ownerR->currentItem() );
	group.writeEntry ( "Owner Execute", ownerX->currentItem() );
	group.writeEntry ( "Group Write", groupW->currentItem() );
	group.writeEntry ( "Group Read", groupR->currentItem() );
	group.writeEntry ( "Group Execute", groupX->currentItem() );
	group.writeEntry ( "All Write", allW->currentItem() );
	group.writeEntry ( "All Read", allR->currentItem() );
	group.writeEntry ( "All Execute", allX->currentItem() );
}


#include "advancedfilter.moc"
