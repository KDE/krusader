/***************************************************************************
                                 packguibase.cpp
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
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
#include "packguibase.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qspinbox.h>
#include <qslider.h>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QVBoxLayout>
#include <kiconloader.h>
#include <kcolorscheme.h>
#include <kcombobox.h>
#include <kmessagebox.h>
#include <kio/global.h>
#include <khistorycombobox.h>
#include "../krusader.h"

/* 
 *  Constructs a PackGUIBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
PackGUIBase::PackGUIBase( QWidget* parent )
    : QDialog( parent ), expanded( false )
{
    setModal( true );
    resize( 430, 140 );
    setCaption( i18n( "Pack" ) );
    grid = new QGridLayout( this );
    grid->setSpacing( 6 );
    grid->setContentsMargins( 11, 11, 11, 11 );

    hbox = new QHBoxLayout;
    hbox->setSpacing( 6 );
    hbox->setContentsMargins( 0, 0, 0, 0 );

    TextLabel3 = new QLabel( this );
    TextLabel3->setText( i18n( "To archive"  ) );
    hbox->addWidget( TextLabel3 );

    nameData = new QLineEdit( this );
    hbox->addWidget( nameData );

    typeData = new QComboBox( this );
    typeData->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0 ) );
    connect( typeData, SIGNAL( activated( const QString & ) ), this,  SLOT( checkConsistency() ) );
    connect( typeData, SIGNAL( highlighted( const QString & ) ), this,  SLOT( checkConsistency() ) );
    hbox->addWidget( typeData );

    grid->addLayout( hbox, 1, 0 );

    hbox_2 = new QHBoxLayout;
    hbox_2->setSpacing( 6 );
    hbox_2->setContentsMargins( 0, 0, 0, 0 );

    TextLabel5 = new QLabel( this );
    TextLabel5->setText( i18n( "In directory"  ) );
    hbox_2->addWidget( TextLabel5 );

    dirData = new QLineEdit( this );
    hbox_2->addWidget( dirData );

    browseButton = new QToolButton( this );
    browseButton->setIconSet( SmallIcon( "fileopen" ) );
    hbox_2->addWidget( browseButton );
    QSpacerItem* spacer = new QSpacerItem( 48, 20, QSizePolicy::Fixed, QSizePolicy::Fixed );
    hbox_2->addItem( spacer );

    grid->addLayout( hbox_2, 2, 0 );

    hbox_3 = new QHBoxLayout;
    hbox_3->setSpacing( 6 );
    hbox_3->setContentsMargins( 0, 0, 0, 0 );

    PixmapLabel1 = new QLabel( this );
    PixmapLabel1->setPixmap( krLoader->loadIcon("package", KIconLoader::Desktop, 32) );
    PixmapLabel1->setScaledContents( TRUE );
    PixmapLabel1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0 ) );
    hbox_3->addWidget( PixmapLabel1 );

    TextLabel1 = new QLabel( this );
    TextLabel1->setText( i18n( "Pack"  ) );
    hbox_3->addWidget( TextLabel1 );

    grid->addLayout( hbox_3, 0, 0 );


    hbox_4 = new QHBoxLayout;
    hbox_4->setSpacing( 6 );
    hbox_4->setContentsMargins( 0, 0, 0, 0 );

    QSpacerItem* spacer_3 = new QSpacerItem( 20, 26, QSizePolicy::Fixed, QSizePolicy::Expanding );
    hbox_4->addItem( spacer_3 );
    grid->addLayout( hbox_4, 3, 0 );

    advancedWidget = new QWidget( this );

    hbox_5 = new QGridLayout( advancedWidget );
    hbox_5->setSpacing( 6 );
    hbox_5->setContentsMargins( 0, 0, 0, 0 );


    QVBoxLayout *compressLayout = new QVBoxLayout;
    compressLayout->setSpacing( 6 );
    compressLayout->setContentsMargins( 0, 0, 0, 0 );

    multipleVolume = new QCheckBox( i18n( "Multiple volume archive" ), advancedWidget );
    connect( multipleVolume, SIGNAL( toggled( bool ) ), this, SLOT( checkConsistency() ) );
    compressLayout->addWidget( multipleVolume, 0, 0 );

    QHBoxLayout * volumeHbox = new QHBoxLayout;

    QSpacerItem* spacer_5 = new QSpacerItem( 20, 26, QSizePolicy::Fixed, QSizePolicy::Fixed );
    volumeHbox->addItem( spacer_5 );

    TextLabel7 = new QLabel( i18n("Size:" ), advancedWidget );
    volumeHbox->addWidget( TextLabel7 );
    
    volumeSpinBox = new QSpinBox( advancedWidget );
    volumeSpinBox->setMinValue( 1 );
    volumeSpinBox->setMaxValue( 9999 );
    volumeSpinBox->setValue( 1440 );
    volumeHbox->addWidget( volumeSpinBox );

    volumeUnitCombo = new QComboBox( advancedWidget );
    volumeUnitCombo->addItem( "B" );
    volumeUnitCombo->addItem( "KB" );
    volumeUnitCombo->addItem( "MB" );
    volumeUnitCombo->setCurrentItem( 1 );
    volumeHbox->addWidget( volumeUnitCombo );

    compressLayout->addLayout ( volumeHbox );

    setCompressionLevel = new QCheckBox( i18n( "Set compression level" ), advancedWidget );
    connect( setCompressionLevel, SIGNAL( toggled( bool ) ), this, SLOT( checkConsistency() ) );
    compressLayout->addWidget( setCompressionLevel, 0, 0 );

    QHBoxLayout * sliderHbox = new QHBoxLayout;

    QSpacerItem* spacer_6 = new QSpacerItem( 20, 26, QSizePolicy::Fixed, QSizePolicy::Fixed );
    sliderHbox->addItem( spacer_6 );

    QWidget * sliderVBoxWidget = new QWidget( advancedWidget );
    QVBoxLayout *sliderVBox = new QVBoxLayout( sliderVBoxWidget );

    compressionSlider = new QSlider( 1, 9, 1, 5, Qt::Horizontal, sliderVBoxWidget );
    compressionSlider->setTickmarks( QSlider::TicksBelow );
    sliderVBox->addWidget( compressionSlider );

    QWidget * minmaxWidget = new QWidget( sliderVBoxWidget );
    sliderVBox->addWidget( minmaxWidget );

    QHBoxLayout * minmaxHbox = new QHBoxLayout( minmaxWidget );

    minLabel = new QLabel( i18n("MIN"), minmaxWidget );
    maxLabel = new QLabel( i18n("MAX"), minmaxWidget );
    maxLabel->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    minmaxHbox->addWidget( minLabel );
    minmaxHbox->addWidget( maxLabel );

    sliderHbox->addWidget( sliderVBoxWidget );

    compressLayout->addLayout( sliderHbox );

    compressLayout->addStretch( 0 );
    hbox_5->addLayout( compressLayout, 0, 0 );

    QFrame *vline = new QFrame( advancedWidget );
    vline->setFrameStyle( QFrame::VLine | QFrame::Sunken );
    vline->setMinimumWidth( 20 );
    hbox_5->addWidget( vline, 0, 1 );


    QGridLayout * passwordGrid = new QGridLayout;
    passwordGrid->setSpacing( 6 );
    passwordGrid->setContentsMargins( 0, 0, 0, 0 );

    TextLabel4 = new QLabel( advancedWidget );
    TextLabel4->setText( i18n( "Password"  ) );
    passwordGrid->addWidget( TextLabel4, 0, 0 );

    password = new QLineEdit( advancedWidget );
    password->setEchoMode( QLineEdit::Password );
    connect( password, SIGNAL( textChanged ( const QString & ) ), this, SLOT( checkConsistency() ) );

    passwordGrid->addWidget( password, 0, 1 );

    TextLabel6 = new QLabel( advancedWidget );
    TextLabel6->setText( i18n( "Again"  ) );
    passwordGrid->addWidget( TextLabel6, 1, 0 );

    passwordAgain = new QLineEdit( advancedWidget );
    passwordAgain->setEchoMode( QLineEdit::Password );
    connect( passwordAgain, SIGNAL( textChanged ( const QString & ) ), this, SLOT( checkConsistency() ) );

    passwordGrid->addWidget( passwordAgain, 1, 1 );

    QHBoxLayout *consistencyHbox = new QHBoxLayout;

    QSpacerItem* spacer_cons = new QSpacerItem( 48, 20, QSizePolicy::Expanding, QSizePolicy::Fixed );
    consistencyHbox->addItem( spacer_cons );

    passwordConsistencyLabel = new QLabel( advancedWidget );
    consistencyHbox->addWidget( passwordConsistencyLabel );
    passwordGrid->addLayout ( consistencyHbox, 2, 0, 1, 2 );

    encryptHeaders = new QCheckBox( i18n( "Encrypt headers" ), advancedWidget );
    passwordGrid->addWidget ( encryptHeaders, 3, 0, 1, 2 );

    QSpacerItem* spacer_psw = new QSpacerItem( 20, 20, QSizePolicy::Fixed, QSizePolicy::Expanding );
    passwordGrid->addItem( spacer_psw, 4, 0 );

    hbox_5->addLayout( passwordGrid, 0, 2 );

    hbox_7 = new QHBoxLayout;
    hbox_7->setSpacing( 6 );
    hbox_7->setContentsMargins( 0, 0, 0, 0 );

    TextLabel8 = new QLabel( i18n( "Command line switches:" ), advancedWidget );
    TextLabel8->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    hbox_7->addWidget( TextLabel8 );

    commandLineSwitches = new KHistoryComboBox( advancedWidget );
    commandLineSwitches->setMaxCount(25);  // remember 25 items
    commandLineSwitches->setDuplicatesEnabled(false);
    KConfigGroup group( krConfig, "Archives");
    QStringList list = group.readEntry("Command Line Switches", QStringList() );
    commandLineSwitches->setHistoryItems(list);

    hbox_7->addWidget( commandLineSwitches );
    
    hbox_5->addLayout( hbox_7, 1, 0, 1, 3 );


    advancedWidget->hide();
    checkConsistency();

    grid->addWidget( advancedWidget, 4, 0 );
    
    hbox_6 = new QHBoxLayout;
    hbox_6->setSpacing( 6 );
    hbox_6->setContentsMargins( 0, 0, 0, 0 );

    advancedButton = new QPushButton( this );
    advancedButton->setText( i18n( "&Advanced" ) + " >>" );
    hbox_6->addWidget( advancedButton );

    QSpacerItem* spacer_2 = new QSpacerItem( 140, 20, QSizePolicy::Expanding, QSizePolicy::Fixed );
    hbox_6->addItem( spacer_2 );

    okButton = new QPushButton( this );
    okButton->setText( i18n( "Ok"  ) );
    okButton->setDefault( true );
    hbox_6->addWidget( okButton );

    cancelButton = new QPushButton( this );
    cancelButton->setText( i18n( "Cancel"  ) );
    hbox_6->addWidget( cancelButton );

    grid->addLayout( hbox_6, 6, 0 );

    // signals and slots connections
    connect( okButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( advancedButton, SIGNAL( clicked() ), this, SLOT( expand() ) );
    connect( cancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( browseButton, SIGNAL( clicked() ), this, SLOT( browse() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
PackGUIBase::~PackGUIBase()
{
    // no need to delete child widgets, Qt does it all for us
}

void PackGUIBase::browse()
{
    qWarning( "PackGUIBase::browse(): Not implemented yet!" );
}

void PackGUIBase::expand() {
    expanded = !expanded;
    
    advancedButton->setText( i18n( "&Advanced" ) + ( expanded ?  " <<" : " >>" ) );

    if( expanded )
      advancedWidget->show();
    else {
      advancedWidget->hide();
      layout()->activate();
      QSize minSize = minimumSize();
      resize( width(), minSize.height() );
    }
    show();
}

void PackGUIBase::checkConsistency() {
    if( password->text().isEmpty() && passwordAgain->text().isEmpty()) {
      passwordConsistencyLabel->setPaletteForegroundColor( KColorScheme(QPalette::Active, KColorScheme::View).foreground().color() );
      passwordConsistencyLabel->setText( i18n( "No password specified" ) );
    }
    else
    if( password->text() == passwordAgain->text() ) {
      passwordConsistencyLabel->setPaletteForegroundColor( KColorScheme(QPalette::Active, KColorScheme::View).foreground().color() );
      passwordConsistencyLabel->setText( i18n( "The passwords are equal" ) );
    }
    else {
      passwordConsistencyLabel->setPaletteForegroundColor( Qt::red );
      passwordConsistencyLabel->setText( i18n( "The passwords are different" ) );
    }

    QString packer = typeData->currentText();

    bool passworded = false;
    if( packer == "7z" || packer == "rar" || packer == "zip" || packer == "arj" )
      passworded = true;

    passwordConsistencyLabel->setEnabled( passworded );
    password->setEnabled( passworded );
    passwordAgain->setEnabled( passworded );
    TextLabel4->setEnabled( passworded );
    TextLabel6->setEnabled( passworded );

    encryptHeaders->setEnabled( packer == "rar" );

    multipleVolume->setEnabled( packer == "rar" || packer == "arj" );
    bool volumeEnabled = multipleVolume->isEnabled() && multipleVolume->isChecked();
    volumeSpinBox->setEnabled( volumeEnabled );
    volumeUnitCombo->setEnabled( volumeEnabled );
    TextLabel7->setEnabled( volumeEnabled );

    /* TODO */
    setCompressionLevel->setEnabled( packer == "rar" || packer == "arj" || packer == "zip" ||
                                     packer == "7z" );
    bool sliderEnabled = setCompressionLevel->isEnabled() && setCompressionLevel->isChecked();
    compressionSlider->setEnabled( sliderEnabled );
    minLabel->setEnabled( sliderEnabled );
    maxLabel->setEnabled( sliderEnabled );
}

bool PackGUIBase::extraProperties( QMap<QString,QString> & inMap ) {
    inMap.clear();

    if( password->isEnabled() && passwordAgain->isEnabled() ) {
      if( password->text() != passwordAgain->text() ) {
        KMessageBox::error( this, i18n( "Cannot pack! The passwords are different!" ) );
        return false;
      }

      if( !password->text().isEmpty() ) {
        inMap[ "Password" ] = password->text();

        if( encryptHeaders->isEnabled() && encryptHeaders->isChecked() )
          inMap[ "EncryptHeaders" ] = "1";
      }
    }

    if( multipleVolume->isEnabled() && multipleVolume->isChecked() ) {
      KIO::filesize_t size = volumeSpinBox->value();

      switch( volumeUnitCombo->currentIndex() ) {
      case 2:
        size *= 1000;
      case 1:
        size *= 1000;
      default:
        break;
      }

      if( size < 10000 ) {
        KMessageBox::error( this, i18n( "Invalid volume size!" ) );
        return false;
      }

      QString sbuffer;
      sbuffer.sprintf("%llu",size);

      inMap[ "VolumeSize" ] = sbuffer;
    }

    if( setCompressionLevel->isEnabled() && setCompressionLevel->isChecked() ) {
      inMap[ "CompressionLevel" ] = QString("%1").arg( compressionSlider->value() );
    }

    QString cmdArgs = commandLineSwitches->currentText().trimmed();
    if( !cmdArgs.isEmpty() ) {
      bool firstChar = true;
      QChar quote = '\0';
      
      for( int i=0; i < cmdArgs.length(); i++ ) {
         QChar ch( cmdArgs[ i ] );
         if( ch.isSpace() )
           continue;

         if( ch == quote ) {
           quote = '\0';
           continue;
         }
         
         if( firstChar && ch != '-' ) {
           KMessageBox::error( this, i18n( "Invalid command line switch!\nSwitch must start with '-'!" ) );
           return false;
         }
         
         firstChar = false;

         if( quote == '"' )
           continue;
         if( quote == '\0' && ( ch == '\'' || ch == '"' ) )
           quote = ch;
         if( ch == '\\' ) {
           if( i == cmdArgs.length() - 1 ) {
             KMessageBox::error( this, i18n( "Invalid command line switch!\nBackslash cannot be the last character" ) );
             return false;
           }
           i++;
         }
      }

      if( quote != '\0' ) {
             KMessageBox::error( this, i18n( "Invalid command line switch!\nUnclosed quotation mark!" ) );
             return false;
      }

      commandLineSwitches->addToHistory( cmdArgs );
      QStringList list = commandLineSwitches->historyItems();
      KConfigGroup group( krConfig, "Archives");
      group.writeEntry("Command Line Switches", list);

      inMap[ "CommandLineSwitches" ] = cmdArgs;      
    }
    return true;
}

#include "packguibase.moc"
