/****************************************************************************
** Form implementation generated from reading ui file 'kgadvanced.ui'
**
** Created: Tue Apr 10 01:13:24 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "kgadvanced.h"

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <klocale.h>

/* 
 *  Constructs a kgAdvanced which is a child of 'parent', with the 
 *  name 'name'.' 
 */
kgAdvanced::kgAdvanced( QWidget* parent,  const char* name )
    : QFrame( parent, name )
{
    if ( !name )
	setName( "kgAdvanced" );
    resize( 430, 354 ); 
    setCaption( i18n( "Advanced" ) );
    setFrameShape( QFrame::NoFrame );
    setFrameShadow( QFrame::MShadow );
    kgAdvancedLayout = new QGridLayout( parent );
    kgAdvancedLayout->setSpacing( 6 );
    kgAdvancedLayout->setMargin( 11 );

    GroupBox6 = new QGroupBox( parent, "GroupBox6" );
    GroupBox6->setTitle( i18n( "Confirmations" ) );

    QWidget* privateLayoutWidget = new QWidget( GroupBox6, "Layout6" );
    privateLayoutWidget->setGeometry( QRect( 11, 21, 420, 140 ) );
    Layout6 = new QVBoxLayout( privateLayoutWidget );
    Layout6->setSpacing( 6 );
    Layout6->setMargin( 0 );

    TextLabel1 = new QLabel( privateLayoutWidget, "TextLabel1" );
    TextLabel1->setText( i18n( "Krusader will request user confirmation for the following operations:" ) );
    Layout6->addWidget( TextLabel1 );

    kgNonEmpty = new QCheckBox( privateLayoutWidget, "kgNonEmpty" );
    kgNonEmpty->setText( i18n( "Deleting non-empty directorie(s)" ) );
    Layout6->addWidget( kgNonEmpty );

    kgDelete = new QCheckBox( privateLayoutWidget, "kgDelete" );
    kgDelete->setText( i18n( "Deleting file(s)" ) );
    Layout6->addWidget( kgDelete );

    kgCopy = new QCheckBox( privateLayoutWidget, "kgCopy" );
    kgCopy->setText( i18n( "Copying file(s)" ) );
    Layout6->addWidget( kgCopy );

    kgMove = new QCheckBox( privateLayoutWidget, "kgMove" );
    kgMove->setText( i18n( "Moving file(s)" ) );
    Layout6->addWidget( kgMove );

    kgAdvancedLayout->addWidget( GroupBox6, 1, 0 );

    GroupBox15 = new QGroupBox( parent, "GroupBox15" );
    GroupBox15->setTitle( i18n( "Fine-Tuning" ) );

    QWidget* privateLayoutWidget_2 = new QWidget( GroupBox15, "Layout5" );
    privateLayoutWidget_2->setGeometry( QRect( 10, 20, 230, 27 ) );
    Layout5 = new QHBoxLayout( privateLayoutWidget_2 );
    Layout5->setSpacing( 6 );
    Layout5->setMargin( 0 );

    TextLabel2_2 = new QLabel( privateLayoutWidget_2, "TextLabel2_2" );
    TextLabel2_2->setText( i18n( "Icon cache size (KB):" ) );
    QToolTip::add(  TextLabel2_2, i18n( "Cache size determines how fast Krusader can display the contents of a panel. However too big a cache might consume your memory." ) );
    Layout5->addWidget( TextLabel2_2 );

    kgCacheSize = new QSpinBox( privateLayoutWidget_2, "kgCacheSize" );
    kgCacheSize->setButtonSymbols( QSpinBox::UpDownArrows );
    kgCacheSize->setMaxValue( 8192 );
    kgCacheSize->setMinValue( 1 );
    QToolTip::add(  kgCacheSize, i18n( "Cache size determines how fast can Krusader display the contents of a panel. However too big a cache might consume your memory." ) );
    Layout5->addWidget( kgCacheSize );

    kgAdvancedLayout->addWidget( GroupBox15, 2, 0 );

    GroupBox2 = new QGroupBox( parent, "GroupBox2" );
    GroupBox2->setTitle( i18n( "General" ) );

    QWidget* privateLayoutWidget_3 = new QWidget( GroupBox2, "Layout3" );
    privateLayoutWidget_3->setGeometry( QRect( 10, 20, 450, 40 ) );
    Layout3 = new QGridLayout( privateLayoutWidget_3 );
    Layout3->setSpacing( 6 );
    Layout3->setMargin( 0 );

    kgRootSwitch = new QCheckBox( privateLayoutWidget_3, "kgRootSwitch" );
    kgRootSwitch->setText( i18n( "I AM ROOT - use with caution !" ) );
    QToolTip::add(  kgRootSwitch, i18n( "Root Switch: if checked, Krusader will try to act as root - thus attempting to perform actions WITHOUT checking permissions !!!" ) );

    Layout3->addWidget( kgRootSwitch, 0, 0 );

    kgAutomount = new QCheckBox( privateLayoutWidget_3, "kgAutomount" );
    kgAutomount->setText( i18n( "Automount filesystems" ) );
    QToolTip::add(  kgAutomount, i18n( "If checked, Krusader will mount FSTAB mount-points when needed." ) );

    Layout3->addWidget( kgAutomount, 0, 1 );

    kgAdvancedLayout->addWidget( GroupBox2, 0, 0 );

    // signals and slots connections
}

/*  
 *  Destroys the object and frees any allocated resources
 */
kgAdvanced::~kgAdvanced()
{
    // no need to delete child widgets, Qt does it all for us
}

void kgAdvanced::slotApplyChanges()
{
    qWarning( "kgAdvanced::slotApplyChanges(): Not implemented yet!" );
}

void kgAdvanced::slotDefaultSettings()
{
    qWarning( "kgAdvanced::slotDefaultSettings(): Not implemented yet!" );
}

#include "kgadvanced.moc"


