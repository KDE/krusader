/***************************************************************************
** Form implementation generated from reading ui file 'kgarchives.ui'
**
** Created: Tue Apr 17 20:21:46 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "kgarchives.h"

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <klocale.h>

/* 
 *  Constructs a kgArchives which is a child of 'parent', with the 
 *  name 'name'.'
 */
kgArchives::kgArchives( QWidget* parent,  const char* name )
    : QFrame( parent, name )
{
    if ( !name )
	setName( "kgArchives" );
    resize( 378, 348 ); 
    setCaption( i18n( "Archives" ) );
    setFrameShape( QFrame::NoFrame );
    setFrameShadow( QFrame::MShadow );
    QToolTip::add(  this, "" );
    kgArchivesLayout = new QGridLayout( parent );
    kgArchivesLayout->setSpacing( 6 );
    kgArchivesLayout->setMargin( 11 );

    GroupBox13 = new QGroupBox( parent, "GroupBox13" );
    GroupBox13->setFrameShape( QGroupBox::Box );
    GroupBox13->setFrameShadow( QGroupBox::Sunken );
    GroupBox13->setTitle( i18n( "General" ) );
    GroupBox13->setColumnLayout(0, Qt::Vertical );
    GroupBox13->layout()->setSpacing( 0 );
    GroupBox13->layout()->setMargin( 0 );
    GroupBox13Layout = new QGridLayout( GroupBox13->layout() );
    GroupBox13Layout->setAlignment( Qt::AlignTop );
    GroupBox13Layout->setSpacing( 6 );
    GroupBox13Layout->setMargin( 11 );

    kgBZip2 = new QCheckBox( GroupBox13, "kgBZip2" );
    kgBZip2->setText( i18n( "BZip2" ) );

    GroupBox13Layout->addWidget( kgBZip2, 1, 6 );

    kgGZip = new QCheckBox( GroupBox13, "kgGZip" );
    kgGZip->setText( i18n( "GZip" ) );

    GroupBox13Layout->addWidget( kgGZip, 1, 3 );

    kgRpm = new QCheckBox( GroupBox13, "kgRpm" );
    kgRpm->setText( i18n( "Rpm" ) );

    GroupBox13Layout->addWidget( kgRpm, 3, 0 );

    kgZip = new QCheckBox( GroupBox13, "kgZip" );
    kgZip->setText( i18n( "Zip" ) );

    GroupBox13Layout->addWidget( kgZip, 2, 0 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupBox13Layout->addMultiCell( spacer, 2, 2, 4, 5 );

    kgTar = new QCheckBox( GroupBox13, "kgTar" );
    kgTar->setText( i18n( "Tar" ) );

    GroupBox13Layout->addWidget( kgTar, 1, 0 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupBox13Layout->addMultiCell( spacer_2, 1, 1, 4, 5 );

    kgArj = new QCheckBox( GroupBox13, "kgArj" );
    kgArj->setText( i18n( "Arj" ) );

    GroupBox13Layout->addWidget( kgArj, 2, 6 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupBox13Layout->addMultiCell( spacer_3, 1, 1, 1, 2 );
    QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupBox13Layout->addMultiCell( spacer_4, 5, 5, 0, 1 );

    kgAutoConfigure = new QPushButton( GroupBox13, "kgAutoConfigure" );
    kgAutoConfigure->setText( i18n( "Auto Configure" ) );

    GroupBox13Layout->addMultiCellWidget( kgAutoConfigure, 5, 5, 2, 4 );
    QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupBox13Layout->addMultiCell( spacer_5, 5, 5, 5, 6 );

    TextLabel1_2 = new QLabel( GroupBox13, "TextLabel1_2" );
    TextLabel1_2->setText( i18n( "Krusader transparently handles the following types of archives:" ) );

    GroupBox13Layout->addMultiCellWidget( TextLabel1_2, 0, 0, 0, 6 );

    kgRar = new QCheckBox( GroupBox13, "kgRar" );
    kgRar->setText( i18n( "Rar" ) );

    GroupBox13Layout->addWidget( kgRar, 2, 3 );

    kgAce = new QCheckBox( GroupBox13, "kgAce" );
    kgAce->setText( i18n( "Ace" ) );

    GroupBox13Layout->addWidget( kgAce, 3, 3 );

    TextLabel1 = new QLabel( GroupBox13, "TextLabel1" );
    TextLabel1->setText( i18n( "The archives that are \"greyed-out\" were unavaible on your\nsystem last time Krusader checked. If you wish Krusader to\nsearch again, click the 'Auto Configure' button." ) );

    GroupBox13Layout->addMultiCellWidget( TextLabel1, 4, 4, 0, 6 );
    QSpacerItem* spacer_6 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupBox13Layout->addMultiCell( spacer_6, 2, 2, 1, 2 );
    QSpacerItem* spacer_7 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupBox13Layout->addMultiCell( spacer_7, 3, 3, 1, 2 );

    kgArchivesLayout->addWidget( GroupBox13, 0, 0 );

    GroupBox2 = new QGroupBox( parent, "GroupBox2" );
    GroupBox2->setTitle( i18n( "Fine-Tuning" ) );

    QWidget* privateLayoutWidget = new QWidget( GroupBox2, "Layout4" );
    privateLayoutWidget->setGeometry( QRect( 10, 20, 330, 60 ) );
    Layout4 = new QVBoxLayout( privateLayoutWidget );
    Layout4->setSpacing( 6 );
    Layout4->setMargin( 0 );

    kgMoveIntoArchives = new QCheckBox( privateLayoutWidget, "kgMoveIntoArchives" );
    kgMoveIntoArchives->setText( i18n( "Allow moving into archives" ) );
    QToolTip::add(  kgMoveIntoArchives, i18n( "This action can be tricky, since system failure during the process\nmight result in misplaced files. If this happens,\nthe files are stored in a temp directory inside /tmp." ) );
    Layout4->addWidget( kgMoveIntoArchives );

    kgTestArchives = new QCheckBox( privateLayoutWidget, "kgTestArchives" );
    kgTestArchives->setText( i18n( "Test archive when finished packing" ) );
    QToolTip::add(  kgTestArchives, i18n( "If checked, Krusader will test the archive's intergrity after packing it." ) );
    Layout4->addWidget( kgTestArchives );

    kgArchivesLayout->addWidget( GroupBox2, 1, 0 );

    // signals and slots connections
    connect( kgAutoConfigure, SIGNAL( clicked() ), this, SLOT( slotAutoConfigure() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
kgArchives::~kgArchives()
{
    // no need to delete child widgets, Qt does it all for us
}

void kgArchives::slotApplyChanges()
{
    qWarning( "kgArchives::slotApplyChanges(): Not implemented yet!" );
}

void kgArchives::slotAutoConfigure()
{
    qWarning( "kgArchives::slotAutoConfigure(): Not implemented yet!" );
}

void kgArchives::slotDefaultSettings()
{
    qWarning( "kgArchives::slotDefaultSettings(): Not implemented yet!" );
}

#include "kgarchives.moc"
