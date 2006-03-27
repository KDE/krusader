/***************************************************************************
                     synchronizedialog.cpp  -  description
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

#include "synchronizedialog.h"
#include "../VFS/krpermhandler.h"
#include "../krusader.h"
#include "../defaults.h"
#include <qlayout.h>
#include <qhbox.h>
#include <klocale.h>

SynchronizeDialog::SynchronizeDialog( QWidget* parent,  const char* name, bool modal, WFlags fl,
                                      Synchronizer *sync, int pleftCopyNr, KIO::filesize_t pleftCopySize,
                                      int prightCopyNr, KIO::filesize_t prightCopySize, int pdeleteNr,
                                      KIO::filesize_t pdeleteSize ) : QDialog( parent, name, modal, fl ),
                                      synchronizer( sync ), leftCopyNr ( pleftCopyNr ),
                                      leftCopySize( pleftCopySize ), rightCopyNr ( prightCopyNr ),
                                      rightCopySize( prightCopySize ), deleteNr( pdeleteNr ),
                                      deleteSize( pdeleteSize ), isPause( true ), syncStarted( false )
{
  setCaption( i18n("Krusader::Synchronize") );

  QVBoxLayout *layout = new QVBoxLayout( this, 11, 6, "SynchronizeDialogLayout" );

  cbRightToLeft = new QCheckBox( i18n( "Right to left: Copy 1 file", "Right to left: Copy %n files", leftCopyNr) + " " +
                                 i18n( "(1 byte)", "(%n bytes)", KRpermHandler::parseSize( leftCopySize ).stripWhiteSpace().toInt() ),
                                 this, "labelRightToLeft" );
  cbRightToLeft->setChecked( leftCopyNr != 0 );
  cbRightToLeft->setEnabled( leftCopyNr != 0 );
  layout->addWidget( cbRightToLeft );

  lbRightToLeft = new QLabel( "\t" + i18n( "Ready: %1/1 file, %3/%4", "Ready: %1/%n files, %3/%4", leftCopyNr).arg( 0 )
                             .arg( 0 ).arg( KRpermHandler::parseSize( leftCopySize ).stripWhiteSpace() ),
                             this, "lbRightToLeft" );
  lbRightToLeft->setEnabled( leftCopyNr != 0 );
  layout->addWidget( lbRightToLeft );

  cbLeftToRight = new QCheckBox( i18n( "Left to right: Copy 1 file", "Left to right: Copy %n files", rightCopyNr) + " " +
                                 i18n( "(1 byte)", "(%n bytes)", KRpermHandler::parseSize( rightCopySize ).stripWhiteSpace().toInt() ),
                                 this, "cbLeftToRight" );
  cbLeftToRight->setChecked( rightCopyNr != 0 );
  cbLeftToRight->setEnabled( rightCopyNr != 0 );
  layout->addWidget( cbLeftToRight );

  lbLeftToRight = new QLabel( "\t" + i18n( "Ready: %1/1 file, %3/%4", "Ready: %1/%n files, %3/%4", rightCopyNr ).arg( 0 )
                             .arg( 0 ).arg( KRpermHandler::parseSize( rightCopySize ).stripWhiteSpace() ),
                             this, "lbLeftToRight" );
  lbLeftToRight->setEnabled( rightCopyNr != 0 );
  layout->addWidget( lbLeftToRight );

  cbDeletable = new QCheckBox( i18n( "Left: Delete 1 file", "Left: Delete %n files", deleteNr) + " " +
                               i18n( "(1 byte)", "(%n bytes)", KRpermHandler::parseSize( deleteSize ).stripWhiteSpace().toInt() ),
                               this, "cbDeletable" );
  cbDeletable->setChecked( deleteNr != 0 );
  cbDeletable->setEnabled( deleteNr != 0 );
  layout->addWidget( cbDeletable );

  lbDeletable   = new QLabel( "\t" + i18n( "Ready: %1/1 file, %3/%4", "Ready: %1/%n files, %3/%4", deleteNr ).arg( 0 )
                             .arg( 0 ).arg( KRpermHandler::parseSize( deleteSize ).stripWhiteSpace() ),
                             this, "lbDeletable" );
  lbDeletable->setEnabled( deleteNr != 0 );
  layout->addWidget( lbDeletable );

  progress = new QProgressBar(1000, this);
  progress->setCenterIndicator(true);
  progress->setProgress( 0 );
  progress->setMinimumWidth( 400 );
  layout->addWidget( progress );

  QHBox *hbox = new QHBox( this, "SynchronizeDialogHBox" );
  hbox->setSpacing( 6 );

  cbOverwrite = new QCheckBox( i18n( "Confirm overwrites" ), this, "cbOverWrite" );
  krConfig->setGroup("Synchronize");
  cbOverwrite->setChecked( krConfig->readBoolEntry( "Confirm overwrites", _ConfirmOverWrites  ) );
  layout->addWidget( cbOverwrite );
  
  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  hbox->layout()->addItem( spacer );
  
  btnStart = new QPushButton( hbox, "btnStart" );
  btnStart->setText( i18n( "Start" ) );

  btnPause = new QPushButton( hbox, "btnPause" );
  btnPause->setEnabled( false );
  btnPause->setText( i18n( "Pause" ) );
  
  QPushButton *btnClose = new QPushButton( hbox, "btnClose" );
  btnClose->setText( i18n( "Close" ) );

  layout->addWidget( hbox );

  connect( btnStart,  SIGNAL( clicked() ), this, SLOT( startSynchronization() ) );
  connect( btnPause,  SIGNAL( clicked() ), this, SLOT( pauseOrResume() ) );
  connect( btnClose,  SIGNAL( clicked() ), this, SLOT( reject() ) );
  
  exec();
}

SynchronizeDialog::~SynchronizeDialog()
{
  krConfig->writeEntry("Confirm overwrites", cbOverwrite->isChecked() );  
}

void SynchronizeDialog::startSynchronization()
{
  btnStart->setEnabled( false );
  btnPause->setEnabled( syncStarted = true );
  connect( synchronizer,  SIGNAL( synchronizationFinished() ), this, SLOT( synchronizationFinished() ) );
  connect( synchronizer,  SIGNAL( processedSizes( int, KIO::filesize_t, int, KIO::filesize_t, int, KIO::filesize_t ) ),
                    this, SLOT( processedSizes( int, KIO::filesize_t, int, KIO::filesize_t, int, KIO::filesize_t) ) );
  connect( synchronizer,  SIGNAL( pauseAccepted() ), this, SLOT( pauseAccepted() ) );

  if( !cbRightToLeft->isChecked() ) leftCopySize = 0;
  if( !cbLeftToRight->isChecked() ) rightCopySize = 0;
  if( !cbDeletable->isChecked() )   deleteSize = 0;
  
  synchronizer->synchronize( cbRightToLeft->isChecked(), cbLeftToRight->isChecked(),
                             cbDeletable->isChecked(), !cbOverwrite->isChecked() );
}

void SynchronizeDialog::synchronizationFinished()
{
  QDialog::reject();
}

void SynchronizeDialog::processedSizes( int leftNr, KIO::filesize_t leftSize, int rightNr,
                                        KIO::filesize_t rightSize, int delNr, KIO::filesize_t delSize )
{
  lbRightToLeft->setText( i18n( "\tReady: %1/%2 files, %3/%4" ).arg( leftNr ).arg( leftCopyNr )
                          .arg( KRpermHandler::parseSize( leftSize ).stripWhiteSpace() )
                          .arg( KRpermHandler::parseSize( leftCopySize ).stripWhiteSpace() ) );
  lbLeftToRight->setText( i18n( "\tReady: %1/%2 files, %3/%4" ).arg( rightNr ).arg( rightCopyNr )
                          .arg( KRpermHandler::parseSize( rightSize ).stripWhiteSpace() )
                          .arg( KRpermHandler::parseSize( rightCopySize ).stripWhiteSpace() ) );
  lbDeletable->setText  ( i18n( "\tReady: %1/%2 files, %3/%4" ).arg( delNr ).arg( deleteNr )
                          .arg( KRpermHandler::parseSize( delSize ).stripWhiteSpace() )
                          .arg( KRpermHandler::parseSize( deleteSize ).stripWhiteSpace() ) );

  KIO::filesize_t totalSum      = leftCopySize + rightCopySize + deleteSize;
  KIO::filesize_t processedSum  = leftSize + rightSize + delSize;

  if( totalSum == 0 )
    totalSum++;

  progress->setProgress( (int)(((double)processedSum / (double)totalSum )*1000) );
}

void SynchronizeDialog::pauseOrResume()
{
  if( isPause )
  {
    btnPause->setEnabled( false );
    synchronizer->pause();
  }
  else
  {
    btnPause->setText( i18n( "Pause" ) );
    synchronizer->resume();
    isPause = true;
  }
}

void SynchronizeDialog::pauseAccepted()
{
  btnPause->setText( i18n( "Resume" ) );
  btnPause->setEnabled( true );
  isPause = false;
}

#include "synchronizedialog.moc"
