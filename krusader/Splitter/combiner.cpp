/***************************************************************************
                          combiner.cpp  -  description
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

#include "combiner.h"
#include "../VFS/vfs.h"
#include <klocale.h>
#include <kmessagebox.h>
#include <kfileitem.h>
#include <kio/job.h>
#include <qfileinfo.h>

Combiner::Combiner( QWidget* parent,  QString fileNameIn, QString destinationDirIn, bool unixNamingIn ) :
  QProgressDialog( parent, "Krusader::Combiner", true, 0 ), hasValidSplitFile( false ),
  fileCounter ( 0 ), permissions( -1 ), receivedSize( 0 ),
  combineReadJob( 0 ), combineWriteJob( 0 ), unixNaming( unixNamingIn ), readFileName( QString::null )
{
  fileName       = fileNameIn;

  destinationDir = destinationDirIn;
  if( !destinationDir.endsWith("/") )
    destinationDir += "/";

  crcContext = new CRC32();

  splitFile = "";
  
  setTotalSteps( 100 );
  setAutoClose( false );  /* don't close or reset the dialog automatically */
  setAutoReset( false );
}

Combiner::~Combiner()
{
  combineAbortJobs();
  delete crcContext;
}

void Combiner::combine()
{
  setCaption( i18n("Krusader::Combining...") );
  setLabelText( i18n("Combining the file %1...").arg( fileName ));

    /* check whether the .crc file exists */
  splURL = vfs::fromPathOrURL( fileName + ".crc" );
  KFileItem file(KFileItem::Unknown, KFileItem::Unknown, splURL );
  file.refresh();

  if( !file.isReadable() )
  {
    int ret = KMessageBox::questionYesNo(0, i18n("The CRC information file (%1) is missing!\n"
        "Validity checking is impossible without it. Continue combining?")
        .arg( splURL.prettyURL(0,KURL::StripFileProtocol) ) );

    if( ret == KMessageBox::No )
    {
       emit reject();
       return;
    }

    openNextFile();
  }
  else
  {
    permissions = file.permissions() | QFileInfo::WriteUser;
    
    combineReadJob = KIO::get( splURL, false, false );

    connect(combineReadJob, SIGNAL(data(KIO::Job *, const QByteArray &)),
            this, SLOT(combineSplitFileDataReceived(KIO::Job *, const QByteArray &)));
    connect(combineReadJob, SIGNAL(result(KIO::Job*)),
            this, SLOT(combineSplitFileFinished(KIO::Job *)));
  }

  exec();
}

void Combiner::combineSplitFileDataReceived(KIO::Job *, const QByteArray &byteArray)
{
  splitFile += QString( byteArray );
}

void Combiner::combineSplitFileFinished(KIO::Job *job)
{
  combineReadJob = 0;
  QString error;
  
  if( job->error() )
    error = i18n("Error at reading the CRC file (%1)!").arg( splURL.prettyURL(0,KURL::StripFileProtocol) );
  else
  {
    QStringList splitFileContent = QStringList::split( '\n', splitFile );
    
    if( splitFileContent.count() != 3 || !splitFileContent[0].startsWith("filename=") ||
        !splitFileContent[1].startsWith("size=") ||
        !splitFileContent[2].startsWith("crc32=") )
      error = i18n("Not a valid CRC file!");
    else
    {
      hasValidSplitFile = true;
      expectedFileName = splitFileContent[0].mid( 9 );
      expectedCrcSum   = splitFileContent[2].mid( 6 ).rightJustify( 8, '0' );
      
      QString size = splitFileContent[1].mid( 5 );
      sscanf( size.ascii(), "%llu", &expectedSize );
    }
  }
      
  if( !error.isEmpty() )
  {
    int ret = KMessageBox::questionYesNo( 0, error+ "\n" +
      i18n("Validity checking is impossible without a good CRC file. Continue combining?")  );
    if( ret == KMessageBox::No )
    {
       emit reject();
       return;
    }
  }
      
  openNextFile();
}

void Combiner::openNextFile()
{  
  if( unixNaming )
  {
    if( readFileName.isNull() )
      readFileName = fileName;
    else
    {
      int pos = readFileName.length()-1;
      QChar ch;
      
      do
      {
        ch = readFileName.at( pos ).latin1() + 1;
        if( ch == QChar( 'Z' + 1 ) )
          ch = 'A';
        if( ch == QChar( 'z' + 1 ) )
          ch = 'a';
        readFileName[ pos ] = ch;
        pos--;
      } while( pos >=0 && ch.upper() == QChar( 'A' ) );
    }    
  }
  else
  {
    QString index( "%1" );      /* determining the filename */
    index = index.arg(++fileCounter).rightJustify( 3, '0' );
    readFileName = fileName + "." + index;
  }

  readURL = vfs::fromPathOrURL( readFileName );

      /* creating a write job */
  combineReadJob = KIO::get( readURL, false, false );

  connect(combineReadJob, SIGNAL(data(KIO::Job *, const QByteArray &)),
          this, SLOT(combineDataReceived(KIO::Job *, const QByteArray &)));
  connect(combineReadJob, SIGNAL(result(KIO::Job*)),
          this, SLOT(combineReceiveFinished(KIO::Job *)));
  if( hasValidSplitFile )
    connect(combineReadJob, SIGNAL(percent (KIO::Job *, unsigned long)),
                            this, SLOT(combineWritePercent(KIO::Job *, unsigned long)));
  
}

void Combiner::combineDataReceived(KIO::Job *, const QByteArray &byteArray)
{
  if( byteArray.size() == 0 )
    return;

  crcContext->update( (unsigned char *)byteArray.data(), byteArray.size() );
  transferArray = byteArray.copy();

  receivedSize += byteArray.size();

  if( combineWriteJob == 0 )
  {
    writeURL = vfs::fromPathOrURL(destinationDir + vfs::fromPathOrURL( fileName ).fileName() );
    if( hasValidSplitFile )
      writeURL.setFileName( expectedFileName );
    else if( unixNaming )
      writeURL.setFileName( vfs::fromPathOrURL( fileName ).fileName() + ".out" );
    
    combineWriteJob = KIO::put( writeURL, permissions, true, false, false );    

    connect(combineWriteJob, SIGNAL(dataReq(KIO::Job *, QByteArray &)),
                             this, SLOT(combineDataSend(KIO::Job *, QByteArray &)));
    connect(combineWriteJob, SIGNAL(result(KIO::Job*)),
                             this, SLOT(combineSendFinished(KIO::Job *)));
  }  

  if( combineWriteJob )
  {
    if( combineReadJob ) combineReadJob->suspend();    /* start writing */
    combineWriteJob->resume();
  }
}

void Combiner::combineReceiveFinished(KIO::Job *job)
{
  combineReadJob = 0;   /* KIO automatically deletes the object after Finished signal */
    
  if( job->error() )
  {
    if( combineWriteJob )         /* write out the remaining part of the file */
      combineWriteJob->resume();
    
    if( fileCounter == 1 )
    {
      combineAbortJobs();
      KMessageBox::questionYesNo(0, i18n("Can't open the first splitted file of %1!")
                                 .arg( fileName ) );
      emit reject();
      return;
    }    

    if( hasValidSplitFile )
    {
      QString crcResult = QString( "%1" ).arg( crcContext->result(), 0, 16 ).upper().stripWhiteSpace()
                                         .rightJustify(8, '0');
      
      if( receivedSize != expectedSize )
        error = i18n("Incorrect filesize! The file might have been corrupted!");
      else if ( crcResult != expectedCrcSum.upper().stripWhiteSpace() )
        error = i18n("Incorrect CRC checksum! The file might have been corrupted!");
    }
    return;
  }  
  openNextFile();
}

void Combiner::combineDataSend(KIO::Job *, QByteArray &byteArray)
{
  byteArray = transferArray;
  transferArray = QByteArray();

  if( combineReadJob )
  {
    combineReadJob->resume();    /* start reading */
    combineWriteJob->suspend();
  }
}

void Combiner::combineSendFinished(KIO::Job *job)
{
  combineWriteJob = 0;  /* KIO automatically deletes the object after Finished signal */

  if( job->error() )    /* any error occurred? */
  {
    combineAbortJobs();
    KMessageBox::error(0, i18n("Error at writing file %1!").arg( writeURL.prettyURL(0,KURL::StripFileProtocol) ) );
    emit reject();
    return;
  }

  if( !error.isEmpty() )   /* was any error message at reading ? */
  {
    combineAbortJobs();             /* we cannot write out it in combineReceiveFinished */
    KMessageBox::error(0, error );  /* because emit accept closes it in this function */
    emit reject();
    return;
  }

  emit accept();
}

void Combiner::combineAbortJobs()
{
  if( combineReadJob )
    combineReadJob->kill();
  if( combineWriteJob )
    combineWriteJob->kill();

  combineReadJob = combineWriteJob = 0;
}

void Combiner::combineWritePercent(KIO::Job *, unsigned long)
{
  int percent = (int)((((double)receivedSize / expectedSize ) * 100. ) + 0.5 );
  setProgress( percent );
}

#include "combiner.moc"
