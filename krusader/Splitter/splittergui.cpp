/***************************************************************************
                        splittergui.cpp  -  description
                             -------------------
    copyright            : (C) 2003 by Csaba Karai
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

#include "splittergui.h"
#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>
#include <kmessagebox.h>

PredefinedDevice SplitterGUI::predefinedDevices[] = {
  {i18n( "1.44 MB (3.5\")" ),   1457664},
  {i18n( "1.2 MB (5.25\")" ),   1213952},
  {i18n( "720 kB (3.5\")"  ),    730112},
  {i18n( "360 kB (5.25\")" ),    362496},
  {i18n( "100 MB (ZIP)"    ), 100431872},
  {i18n( "250 MB (ZIP)"    ), 250331136}};

SplitterGUI::SplitterGUI( QWidget* parent,  QString fileName, QString defaultDir ) :
    QDialog( parent, "Krusader::SplitterGUI", true, 0 ), splittedFileSize( 0 ),
    userDefinedSize ( 0x100000 ), lastSelectedDevice( 0 ), resultCode( QDialog::Rejected )
{
  fileToSplit = fileName;
  predefinedDeviceNum = sizeof( predefinedDevices ) / sizeof( PredefinedDevice );

  QGridLayout *grid = new QGridLayout( this );
  grid->setSpacing( 6 );
  grid->setMargin( 11 );

  QLabel *splitterLabel = new QLabel( this, "SplitterLabel" );
  splitterLabel->setText( i18n( "Split the file %1 to directory:"  ).arg( fileName ) );
  splitterLabel->setMinimumWidth( 400 );
  grid->addWidget( splitterLabel,0 ,0 );

  urlReq = new KURLRequester( this, "DestinationDirectory" );
  urlReq->setURL( defaultDir );
  urlReq->setMode( KFile::Directory );
  grid->addWidget( urlReq, 1 ,0 );

  QHBox *splitSizeLine = new QHBox( this, "splitSizeLine" );
     
  deviceCombo = new QComboBox( splitSizeLine, "deviceCombo" );
  for( int i=0; i != predefinedDeviceNum; i++ )
    deviceCombo->insertItem( predefinedDevices[i].name );
  deviceCombo->insertItem( i18n( "User Defined" ) );

  QLabel *spacer = new QLabel( splitSizeLine );
  spacer->setText( " "  );
  spacer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );

  QLabel *bytesPerFile = new QLabel( splitSizeLine, "BytesPerFile" );
  bytesPerFile->setText( i18n( "Max file size:"  ) );

  spinBox = new SplitterSpinBox( splitSizeLine, "spinbox" );
  spinBox->setMinimumWidth( 85 );
  spinBox->setEnabled( false );
    
  sizeCombo = new QComboBox( splitSizeLine, "sizeCombo" );
  sizeCombo->insertItem( i18n( "Byte" ) );
  sizeCombo->insertItem( i18n( "kByte" ) );
  sizeCombo->insertItem( i18n( "MByte" ) );
  sizeCombo->insertItem( i18n( "GByte" ) );

  grid->addWidget( splitSizeLine,2 ,0 );

  QFrame *separator = new QFrame( this, "separatorLine" );
  separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  separator->setFixedHeight( separator->sizeHint().height() );

  grid->addWidget( separator,3 ,0 );
  
  QHBoxLayout *splitButtons = new QHBoxLayout;
  splitButtons->setSpacing( 6 );
  splitButtons->setMargin( 0 );

  QSpacerItem* spacer2 = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
  splitButtons->addItem( spacer2 );

  QPushButton *splitBtn = new QPushButton( this, "splitBtn" );
  splitBtn->setText( i18n("&Split") );
  splitButtons->addWidget( splitBtn );
  
  QPushButton *cancelBtn = new QPushButton( this, "cancelBtn" );
  cancelBtn->setText( i18n("&Cancel") );
  splitButtons->addWidget( cancelBtn );

  grid->addLayout( splitButtons,4 ,0 );
        
  setCaption(i18n("Krusader::Splitter"));

  connect( sizeCombo, SIGNAL( activated(int) ), this, SLOT( sizeComboActivated( int ) ) );
  connect( deviceCombo, SIGNAL( activated(int) ), this, SLOT( predefinedComboActivated( int ) ) );
  connect( cancelBtn, SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect( splitBtn , SIGNAL( clicked() ), this, SLOT( splitPressed() ) );
  
  predefinedComboActivated( 0 );  
  resultCode = exec();
}

void SplitterGUI::sizeComboActivated( int item )
{
  switch ( item )
  {
  case 0:
    spinBox->setDivision( 1 );          /* byte */
    break;
  case 1:
    spinBox->setDivision( 0x400 );      /* kbyte */
    break;
  case 2:
    spinBox->setDivision( 0x100000 );   /* Mbyte */
    break;
  case 3:
    spinBox->setDivision( 0x40000000 ); /* Gbyte */
    break;
  }
}

void SplitterGUI::predefinedComboActivated( int item )
{
  int capacity = userDefinedSize;

  if( item < predefinedDeviceNum )
  {
    if( lastSelectedDevice == predefinedDeviceNum )
      userDefinedSize = spinBox->value();
    
    spinBox->setEnabled( false );
    capacity = predefinedDevices[item].capacity;
  }
  else
    spinBox->setEnabled( true );
  
  spinBox->setValue( capacity );
  
  if( capacity >= 0x40000000 )           /* Gbyte */
  {
    sizeCombo->setCurrentItem( 3 );
    spinBox->setDivision( 0x40000000 );
  }
  else if( capacity >= 0x100000 )        /* Mbyte */
  {
    sizeCombo->setCurrentItem( 2 );
    spinBox->setDivision( 0x100000 );
  }
  else if( capacity >= 0x400 )           /* kbyte */
  {
    sizeCombo->setCurrentItem( 1 );
    spinBox->setDivision( 0x400 );
  }
  else
  {
    sizeCombo->setCurrentItem( 0 );     /* byte */
    spinBox->setDivision( 1 );
  }
  
  lastSelectedDevice = item;  
}

void SplitterGUI::splitPressed()
{
  if( KURL::fromPathOrURL( urlReq->url() ).isMalformed() )
  {
    KMessageBox::error( this, i18n("The directory path URL is malformed!") );
    return;
  }
  
  emit accept();
}

void SplitterGUI::keyPressEvent( QKeyEvent *e )
{
  switch ( e->key() )
  {
  case Key_Enter :
  case Key_Return :
    emit splitPressed();
    return;
  default:
    QDialog::keyPressEvent( e );
  }
}
