/***************************************************************************
                          splitter.cpp  -  description
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

#include "splitter.h"
#include <qlayout.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kio/job.h>
#include <kfileitem.h>
#include <qfileinfo.h>
    
Splitter::Splitter( QWidget* parent,  QString fileNameIn, QString destinationDirIn ) :
  QProgressDialog( parent, "Krusader::Splitter", true, 0 ), splitSize( 0 )
{
  fileName       = KURL::fromPathOrURL( fileNameIn );

  destinationDir = destinationDirIn;
  if( !destinationDir.endsWith("/") )
    destinationDir += "/";

  crcContext = new CRC32();
  
  setTotalSteps( 100 );
  setAutoClose( false );  /* don't close or reset the dialog automatically */
  setAutoReset( false );
}

Splitter::~Splitter()
{
  splitAbortJobs();
  delete crcContext;
}

void Splitter::split( int splitSizeIn )
{
  KFileItem file(KFileItem::Unknown, KFileItem::Unknown, fileName );
  file.refresh();

  permissions = file.permissions() | QFileInfo::WriteUser;
  
  splitSize = splitSizeIn;

  setCaption( i18n("Krusader::Splitting...") );
  setLabelText( i18n("Splitting the file %1...").arg( fileName.path() ));

  if( file.isDir() )
  {
    KMessageBox::error(0, i18n("Can't split a directory!"));
    return;
  }
  
  fileSize = 0;
  fileNumber = 0;

  splitReadJob = KIO::get( fileName, false, false );
    
  connect(splitReadJob, SIGNAL(data(KIO::Job *, const QByteArray &)),
                        this, SLOT(splitDataReceived(KIO::Job *, const QByteArray &)));
  connect(splitReadJob, SIGNAL(result(KIO::Job*)),
                        this, SLOT(splitReceiveFinished(KIO::Job *)));
  connect(splitReadJob, SIGNAL(percent (KIO::Job *, unsigned long)),
                        this, SLOT(splitReceivePercent(KIO::Job *, unsigned long)));

  splitWriteJob = 0;
  noValidWriteJob = true;

  exec();
}

void Splitter::splitDataReceived(KIO::Job *, const QByteArray &byteArray)
{
  if( byteArray.size() == 0 )
    return;

  crcContext->update( (unsigned char *)byteArray.data(), byteArray.size() );  
  fileSize += byteArray.size();

  if( noValidWriteJob )
    splitCreateWriteJob();

  transferArray = byteArray.copy();
  if(splitWriteJob)
  {
    splitReadJob->suspend();    /* start writing */
    splitWriteJob->resume();
  }
}

void Splitter::splitReceiveFinished(KIO::Job *job)
{
  splitReadJob = 0;   /* KIO automatically deletes the object after Finished signal */
  
  if( splitWriteJob )        /* write out the end of the file */
    splitWriteJob->resume();

  if( job->error() )    /* any error occurred? */
  {
    splitAbortJobs();
    KMessageBox::error(0, i18n("Error at reading file %1!").arg( fileName.path() ) );
    emit reject();
    return;
  }

  QString crcResult = QString( "%1" ).arg( crcContext->result(), 0, 16 ).upper().stripWhiteSpace()
                                     .rightJustify(8, '0');

  splitFile = QString( "filename=%1\n" ).arg( fileName.fileName()     )+
              QString( "size=%1\n" )    .arg( KIO::number( fileSize ) )+
              QString( "crc32=%1\n" )   .arg( crcResult );
}

void Splitter::splitReceivePercent (KIO::Job *, unsigned long percent)
{
  setProgress( percent );
}

void Splitter::splitCreateWriteJob()
{
  QString index( "%1" );                   /* making the splitted filename */
  index = index.arg(++fileNumber).rightJustify( 3, '0' );
  QString outFileName = fileName.fileName() + "." + index;
  writeURL = KURL::fromPathOrURL( destinationDir + outFileName );

      /* creating a write job */
  splitWriteJob = KIO::put( writeURL, permissions, true, false, false );
  outputFileSize = 0;
  connect(splitWriteJob, SIGNAL(dataReq(KIO::Job *, QByteArray &)),
                         this, SLOT(splitDataSend(KIO::Job *, QByteArray &)));
  connect(splitWriteJob, SIGNAL(result(KIO::Job*)),
                         this, SLOT(splitSendFinished(KIO::Job *)));
  noValidWriteJob = false;
}

void Splitter::splitDataSend(KIO::Job *, QByteArray &byteArray)
{
  int bufferLen = transferArray.size();

  if( noValidWriteJob )      /* splitted file should be closed ? */
  {
    byteArray = QByteArray();  /* giving empty buffer which indicates closing */
  }
  else if( outputFileSize + bufferLen > splitSize ) /* maximum length reached? */
  {
    int shortLen = splitSize - outputFileSize;
    
    byteArray.duplicate( transferArray.data(), shortLen );
    transferArray.duplicate( transferArray.data() + shortLen, bufferLen - shortLen );
    
    noValidWriteJob = true;   /* close the current segment */
  }
  else
  {
    outputFileSize += bufferLen;  /* write the whole buffer out to the split file */

    byteArray = transferArray;
    transferArray = QByteArray();
  
    if(splitReadJob)
    {
      splitReadJob->resume();    /* start reading */
      splitWriteJob->suspend();
    }
  }
}

void Splitter::splitSendFinished(KIO::Job *job)
{
  splitWriteJob = 0;  /* KIO automatically deletes the object after Finished signal */

  if( job->error() )    /* any error occurred? */
  {
    splitAbortJobs();
    KMessageBox::error(0, i18n("Error at writing file %1!").arg( writeURL.path() ) );
    emit reject();
    return;
  }
  
  if( transferArray.size() ) /* any data remained in the transfer buffer? */
    splitCreateWriteJob();      /* create a new write job */
  else
  {
      /* writing the split information file out */
    writeURL      = KURL::fromPathOrURL( destinationDir + fileName.fileName() + ".crc" );
    splitWriteJob = KIO::put( writeURL, permissions, true, false, false );
    connect(splitWriteJob, SIGNAL(dataReq(KIO::Job *, QByteArray &)),
                           this, SLOT(splitFileSend(KIO::Job *, QByteArray &)));
    connect(splitWriteJob, SIGNAL(result(KIO::Job*)),
                           this, SLOT(splitFileFinished(KIO::Job *)));
  }
}

void Splitter::splitAbortJobs()
{
  if( splitReadJob )
    splitReadJob->kill();
  if( splitWriteJob )
    splitWriteJob->kill();

  splitReadJob = splitWriteJob = 0;
}

void Splitter::splitFileSend(KIO::Job *, QByteArray &byteArray)
{
  const char *content = splitFile.ascii();
  byteArray.duplicate( content, strlen ( content ) );
  splitFile = "";
}

void Splitter::splitFileFinished(KIO::Job *job)
{
  splitWriteJob = 0;  /* KIO automatically deletes the object after Finished signal */

  if( job->error() )    /* any error occurred? */
  {
    KMessageBox::error(0, i18n("Error at writing file %1!").arg( writeURL.path() ) );
    emit reject();
    return;
  }

  emit accept();
}
