/****************************************************************************
** Form implementation generated from reading ui file 'kgwelcome.ui'
**
** Created: Sun Feb 18 22:39:46 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "kgwelcome.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kstandarddirs.h>

/* 
 *  Constructs a kgWelcome which is a child of 'parent', with the 
 *  name 'name'.' 
 */
kgWelcome::kgWelcome( QWidget* parent,  const char* name )
    : QFrame( parent, name )
{
    QString pix=KGlobal::dirs()->findResource("appdata","konfig_small.jpg");
    QPixmap image0( pix );
    if ( !name )
    setName( "kgWelcome" );
    resize( 598, 468 ); 
    setFrameShape( QFrame::NoFrame );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, sizePolicy().hasHeightForWidth() ) );
    setCaption( i18n( "Konfigurator" ) );
    kgWelcomeLayout = new QGridLayout( parent );
    kgWelcomeLayout->setSpacing( 6 );
    kgWelcomeLayout->setMargin( 11 );

    PixmapLabel1 = new QLabel( parent, "PixmapLabel1" );
    PixmapLabel1->setPixmap( image0 );
    PixmapLabel1->setScaledContents( TRUE );

    kgWelcomeLayout->addWidget( PixmapLabel1, 0, 0 );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
kgWelcome::~kgWelcome()
{
    // no need to delete child widgets, Qt does it all for us
}

#include "kgwelcome.moc"
