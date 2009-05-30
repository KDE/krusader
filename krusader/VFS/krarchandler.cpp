/***************************************************************************
                                krarchandler.cpp
                            -------------------
   copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
   email                : krusader@users.sourceforge.net
   web site		 : http://krusader.sourceforge.net
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
// QT includes
// KDE includes
#include <klocale.h>
#include <kmessagebox.h>
#include <kio/passworddialog.h> 
#include <qfile.h>
#include <kstandarddirs.h>
#include <ktar.h>
#include <kio/global.h>
// Krusader includes
#include "krarchandler.h"
#include "../krusader.h"
#include "../krusaderview.h"
#include "../defaults.h"
#include "../krservices.h"
#include "../Dialogs/krpleasewait.h"
#include <unistd.h> // for usleep

class DefaultKRarcObserver : public KRarcObserver
{
public:
  DefaultKRarcObserver() {}
  virtual ~DefaultKRarcObserver() {}

  virtual void processEvents()
  {
    usleep( 1000 );
    qApp->processEvents();
  }

  virtual void subJobStarted( const QString & jobTitle, int count )
  {
    krApp->startWaiting( jobTitle, count, true );
  }

  virtual void subJobStopped()
  {
    krApp->stopWait();
  }

  virtual bool wasCancelled()
  {
    return krApp->wasWaitingCancelled();
  }

  virtual void error( const QString & error )
  {
    KMessageBox::error( krApp, error, i18n( "Error" ) );
  }

  virtual void detailedError( const QString & error, const QString & details )
  {
    KMessageBox::detailedError (krApp, error, details, i18n("Error" ) );
  }

  virtual void incrementProgress( int c )
  {
    krApp->plzWait->incProgress( c );
  }
};


static QStringList arcProtocols = QString("tar;bzip;bzip2;lzma;gzip;krarc;zip").split(";");

KWallet::Wallet * KRarcHandler::wallet = 0;
KRarcObserver   * KRarcHandler::defaultObserver = new DefaultKRarcObserver();

QStringList KRarcHandler::supportedPackers() {
  QStringList packers;

  // we will simply try to find the packers here..
  if ( KrServices::cmdExist( "tar" ) ) packers.append( "tar" );
  if ( KrServices::cmdExist( "gzip" ) ) packers.append( "gzip" );
  if ( KrServices::cmdExist( "bzip2" ) ) packers.append( "bzip2" );
  if ( KrServices::cmdExist( "lzma" ) ) packers.append( "lzma" );
  if ( KrServices::cmdExist( "unzip" ) ) packers.append( "unzip" );
  if ( KrServices::cmdExist( "zip" ) ) packers.append( "zip" );
  if ( KrServices::cmdExist( "lha" ) ) packers.append( "lha" );
  if ( KrServices::cmdExist( "cpio" ) ) packers.append( "cpio" );
  if ( KrServices::cmdExist( "unrar" ) ) packers.append( "unrar" );
  if ( KrServices::cmdExist( "rar" ) ) packers.append( "rar" );
  if ( KrServices::cmdExist( "arj" ) ) packers.append( "arj" );
  if ( KrServices::cmdExist( "unarj" ) ) packers.append( "unarj" );
  if ( KrServices::cmdExist( "unace" ) ) packers.append( "unace" );
  if ( KrServices::cmdExist( "dpkg"  ) ) packers.append( "dpkg"  );
  if ( KrServices::cmdExist( "7z"  ) || KrServices::cmdExist( "7za" ) ) packers.append( "7z"  );
  if ( KrServices::cmdExist( "rpm"   ) && KrServices::cmdExist( "rpm2cpio" ) ) packers.append( "rpm" );
  // kDebug() << "Supported Packers:" << endl;
  //QStringList::Iterator it;
  //for( it = packers.begin(); it != packers.end(); ++it )
  // kDebug() << *it << endl;

  return packers;
  }

bool KRarcHandler::arcSupported( QString type ) {
  // lst will contain the supported unpacker list...
  KConfigGroup group( krConfig, "Archives" );
  QStringList lst = group.readEntry( "Supported Packers", QStringList() );

  if ( type == "-zip" && lst.contains( "unzip" ) )
    return true;
  else if ( type == "-tar" && lst.contains( "tar" ) )
    return true;
  else if ( type == "-tbz" && lst.contains( "tar" ) )
    return true;
  else if ( type == "-tgz" && lst.contains( "tar" ) )
    return true;
  else if ( type == "-tlz" && lst.contains( "tar" ) )
    return true;
  else if ( type == "tarz" && lst.contains( "tar" ) )
    return true;
  else if ( type == "gzip" && lst.contains( "gzip" ) )
    return true;
  else if ( type == "zip2" && lst.contains( "bzip2" ) )
    return true;
  else if ( type == "lzma" && lst.contains( "lzma" ) )
    return true;
  else if ( type == "-lha" && lst.contains( "lha" ) )
    return true;
  else if ( type == "-ace" && lst.contains( "unace" ) )
    return true;
  else if ( type == "-rpm" && lst.contains( "cpio" ) )
    return true;
  else if ( type == "cpio" && lst.contains( "cpio" ) )
    return true;
  else if ( type == "-rar" && ( lst.contains( "unrar" ) || lst.contains( "rar" ) ) )
    return true;
  else if ( type == "-arj" && ( lst.contains( "unarj" ) || lst.contains( "arj" ) ) )
    return true;
  else if ( type == "-deb" && ( lst.contains( "dpkg"  ) && lst.contains( "tar" ) ) )
    return true;
  else if ( type == "-7z" && lst.contains( "7z"  ) )
    return true;
  // not supported : (
  return false;
  }

bool KRarcHandler::arcHandled( QString type ) {
  // first check if supported
  if ( !arcSupported( type ) ) return false;

  KConfigGroup group( krConfig, "Archives" );
  if ( ( type == "-tgz" && group.readEntry( "Do GZip" , _DoGZip ) ) ||
       ( type == "tarz" && group.readEntry( "Do GZip" , _DoGZip ) ) ||
       ( type == "-tar" && group.readEntry( "Do Tar" , _DoTar ) ) ||
       ( type == "-tbz" && group.readEntry( "Do BZip2", _DoBZip2 ) ) ||
       ( type == "-tlz" && group.readEntry( "Do LZMA", _DoLZMA ) ) ||
       ( type == "gzip" && group.readEntry( "Do GZip" , _DoGZip ) ) ||
       ( type == "zip2" && group.readEntry( "Do BZip2", _DoBZip2 ) ) ||
       ( type == "-zip" && group.readEntry( "Do UnZip", _DoUnZip ) ) ||
       ( type == "-lha" && group.readEntry( "Do Lha", _DoUnZip ) ) ||
       ( type == "-rar" && group.readEntry( "Do UnRar", _DoUnRar ) ) ||
       ( type == "-arj" && group.readEntry( "Do UnArj", _DoUnarj ) ) ||
       ( type == "-ace" && group.readEntry( "Do UnAce", _DoUnAce ) ) ||
       ( type == "cpio" && group.readEntry( "Do RPM" , _DoRPM ) ) ||
       ( type == "-rpm" && group.readEntry( "Do RPM"  , _DoRPM   ) ) ||
       ( type == "-deb" && group.readEntry( "Do DEB"  , _DoDEB   ) ) ||
       ( type == "-7z"  && group.readEntry( "Do 7z"  , _Do7z ) ) )
    return true;
  else
    return false;
  }


long KRarcHandler::arcFileCount( QString archive, QString type, QString password, KRarcObserver *observer ) {
  int divideWith = 1;

  // first check if supported
  if ( !arcSupported( type ) ) return 0;

  // bzip an gzip archive contains only one file
  if ( type == "zip2" || type == "gzip" || type == "lzma" ) return 1L;

  // set the right lister to do the job
  QStringList lister;

  if ( type == "-zip" ) lister << KrServices::fullPathName( "unzip" ) << "-ZTs";
  else if ( type == "-tar" ) lister << KrServices::fullPathName( "tar" ) << "-tvf";
  else if ( type == "-tgz" ) lister << KrServices::fullPathName( "tar" ) << "-tvzf";
  else if ( type == "tarz" ) lister << KrServices::fullPathName( "tar" ) << "-tvzf";
  else if ( type == "-tbz" ) lister << KrServices::fullPathName( "tar" ) << "-tjvf";
  else if ( type == "-tlz" ) lister << KrServices::fullPathName( "tar" ) << "--lzma" << "-tvf";
  else if ( type == "-lha" ) lister << KrServices::fullPathName( "lha" ) << "l";
  else if ( type == "-rar" ) lister << KrServices::fullPathName( KrServices::cmdExist( "rar" ) ? "rar" : "unrar" ) << "l" << "-v";
  else if ( type == "-ace" ) lister << KrServices::fullPathName( "unace" ) << "l";
  else if ( type == "-arj" ) { if( KrServices::cmdExist( "arj" ) )
                                 lister << KrServices::fullPathName( "arj" ) << "v" << "-y" << "-v",
                                 divideWith = 4;
                               else
                                 lister << KrServices::fullPathName( "unarj" ) << "l";
                             }
  else if ( type == "-rpm" ) lister << KrServices::fullPathName( "rpm" ) << "--dump" << "-lpq";
  else if ( type == "-deb" ) lister << KrServices::fullPathName( "dpkg" ) << "-c";
  else if ( type == "-7z" )  lister << KrServices::fullPathName( "7z" ) << "-y" << "l";
  else return 0L;
  
  if ( !password.isNull() ) {
    if ( type == "-arj" )
      lister << QString("-g%1").arg(password);
    if ( type == "-ace" || type == "-rar" || type == "-7z" )
      lister << QString("-p%1").arg(password);
  }

  // tell the user to wait
  observer->subJobStarted( i18n( "Counting files in archive" ), 0 );

  // count the number of files in the archive
  long count = 1;
  KProcess list;
  list << lister << archive;
  if( type == "-ace" && QFile( "/dev/ptmx" ).exists() )  // Don't remove, unace crashes if missing!!!
    list.setStandardInputFile("/dev/ptmx");
  list.setOutputChannelMode(KProcess::SeparateChannels); // without this output redirection has no effect
  list.start();
  // TODO make use of asynchronous process starting. waitForStarted(int msec = 30000) is blocking
  // it would be better to connect to started(), error() and finished()
  if (list.waitForStarted()) while ( list.state() == QProcess::Running ) {
    observer->processEvents();
    if( observer->wasCancelled() )
      list.kill();
    }
  ; // busy wait - need to find something better...
  
  observer->subJobStopped();
  
  if( list.exitStatus() != QProcess::NormalExit || !checkStatus( type, list.exitCode() ) ) {
    observer->detailedError ( i18n( "Failed to list the content of the archive (%1)!", archive ), 
                                QString::fromLocal8Bit(list.readAllStandardError()) );
    return 0;
  }

  count = list.readAllStandardOutput().count('\n');

  //make sure you call stopWait after this function return...
  //  observer->subJobStopped();

  return count / divideWith;
  }

bool KRarcHandler::unpack( QString archive, QString type, QString password, QString dest, KRarcObserver *observer ) {
  KConfigGroup group( krConfig, "Archives" );
  if ( group.readEntry( "Test Before Unpack", _TestBeforeUnpack ) ) {
    // test first - or be sorry later...
    if ( type != "-rpm" && type != "-deb" && !test( archive, type, password, 0, observer ) ) {
      observer->error( i18n( "Failed to unpack" ) + " \"" + archive + "\" !" );
      return false;
      }
    }

  // count the files in the archive
  long count = arcFileCount( archive, type, password, observer );
  if ( count == 0 ) return false; // not supported
  if ( count == 1 ) count = 0 ;

  // choose the right packer for the job
  QString cpioName;
  QStringList packer;

  // set the right packer to do the job
  if ( type == "-zip" ) packer << KrServices::fullPathName( "unzip" ) << "-o" ;
  else if ( type == "-tar" ) packer << KrServices::fullPathName( "tar" ) << "-xvf";
  else if ( type == "-tgz" ) packer << KrServices::fullPathName( "tar" ) << "-xvzf";
  else if ( type == "tarz" ) packer << KrServices::fullPathName( "tar" ) << "-xvzf";
  else if ( type == "-tbz" ) packer << KrServices::fullPathName( "tar" ) << "-xjvf";
  else if ( type == "-tlz" ) packer << KrServices::fullPathName( "tar" ) << "--lzma" << "-xvf";
  else if ( type == "gzip" ) packer << KrServices::fullPathName( "gzip" ) << "-cd";
  else if ( type == "zip2" ) packer << KrServices::fullPathName( "bzip2" ) << "-cdk";
  else if ( type == "lzma" ) packer << KrServices::fullPathName( "lzma" ) << "-cdk";
  else if ( type == "-lha" ) packer << KrServices::fullPathName( "lha" ) << "xf";
  else if ( type == "-rar" ) packer << KrServices::fullPathName( KrServices::cmdExist( "rar" ) ? "rar" : "unrar" ) << "-y" << "x";
  else if ( type == "-ace" ) packer << KrServices::fullPathName( "unace" ) << "x";
  else if ( type == "-arj" ) { if (KrServices::cmdExist( "arj" ))
                                 packer << KrServices::fullPathName( "arj" ) << "-y" << "-v" << "x";
                               else
                                 packer << KrServices::fullPathName( "unarj" ) << "x";
                             }
  else if ( type == "-7z" )  packer << KrServices::fullPathName( "7z" ) << "-y" << "x";
  else if ( type == "-rpm" ) {
    QString tempDir = KStandardDirs::locateLocal("tmp",QString());

    cpioName = tempDir+"/contents.cpio"; // TODO use KTemporaryFile (setAutoRemove(false) when asynchrone)

    KrLinecountingProcess cpio;
    cpio << KrServices::fullPathName( "rpm2cpio" ) << archive;
    cpio.setStandardOutputFile(cpioName); // TODO maybe no tmpfile but a pipe (setStandardOutputProcess(packer))
    cpio.start();
    if( !cpio.waitForFinished() || cpio.exitStatus() != QProcess::NormalExit || !checkStatus( "cpio", cpio.exitCode() ) ) {
      observer->detailedError ( i18n( "Failed to convert rpm (%1) to cpio!", archive ), cpio.getErrorMsg() );
      return 0;
    }
    
    archive = cpioName;
    packer << KrServices::fullPathName( "cpio" ) << "--force-local" << "--no-absolute-filenames" <<  "-iuvdF";
  }
  else if ( type == "-deb" ) {
    QString tempDir = KStandardDirs::locateLocal("tmp",QString());

    cpioName = tempDir+"/contents.tar"; // TODO use KTemporaryFile (setAutoRemove(false) when asynchrone)

    KrLinecountingProcess dpkg;
    dpkg << KrServices::fullPathName( "dpkg" ) << "--fsys-tarfile" << archive;
    dpkg.setStandardOutputFile(cpioName); // TODO maybe no tmpfile but a pipe (setStandardOutputProcess(packer))
    dpkg.start();
    if( !dpkg.waitForFinished() || dpkg.exitStatus() != QProcess::NormalExit || !checkStatus( "-deb", dpkg.exitCode() ) ) {
      observer->detailedError ( i18n( "Failed to convert deb (%1) to tar!", archive ), dpkg.getErrorMsg() );
      return 0;
    }
    
    archive = cpioName;
    packer << KrServices::fullPathName( "tar" ) << "xvf";
  }
  else return false;

  if ( !password.isNull() ) {
    if ( type == "-zip" ) 
      packer << "-P" << password;
    if ( type == "-arj" )
      packer << QString("-g%1").arg(password);
    if ( type == "-ace" || type == "-rar" || type == "-7z" )
      packer << QString("-p%1").arg(password);
  }

  // unpack the files
  KrLinecountingProcess proc;
  proc << packer << archive;
  if( type == "zip2" || type=="gzip" || type == "lzma" ){
    QString arcname = archive.mid(archive.lastIndexOf("/")+1);
    if( arcname.contains(".") ) arcname = arcname.left(arcname.lastIndexOf("."));
    proc.setStandardOutputFile( dest+"/"+arcname );
  }
  if( type == "-ace" && QFile( "/dev/ptmx" ).exists() ) // Don't remove, unace crashes if missing!!!
    proc.setStandardInputFile("/dev/ptmx");
  
  proc.setWorkingDirectory( dest );

  // tell the user to wait
  observer->subJobStarted( i18n( "Unpacking File(s)" ), count );
  if ( count != 0 ) {
    connect( &proc, SIGNAL( newOutputLines( int ) ),
             observer, SLOT( incrementProgress( int ) ) );
    if( type == "-rpm" )
      connect( &proc, SIGNAL( newErrorLines( int ) ),
               observer, SLOT( incrementProgress( int ) ) );
  }

  // start the unpacking process
  proc.start();
  // TODO make use of asynchronous process starting. waitForStarted(int msec = 30000) is blocking
  // it would be better to connect to started(), error() and finished()
  if (proc.waitForStarted()) while ( proc.state() == QProcess::Running ) {
    observer->processEvents();
    if( observer->wasCancelled() )
      proc.kill();
    }
  ; // busy wait - need to find something better...
  observer->subJobStopped();

  if( !cpioName.isEmpty() )
    QFile( cpioName ).remove();    /* remove the cpio file */
  
  // check the return value
  if ( proc.exitStatus() != QProcess::NormalExit || !checkStatus( type, proc.exitCode() ) ) {
    observer->detailedError ( i18n( "Failed to unpack %1!", archive ), 
                                observer->wasCancelled() ? i18n( "User cancelled." ) : proc.getErrorMsg() );
    return false;
    }
  return true; // SUCCESS
  }

bool KRarcHandler::test( QString archive, QString type, QString password, long count, KRarcObserver *observer ) {
  // choose the right packer for the job
  QStringList packer;

  // set the right packer to do the job
  if ( type == "-zip" ) packer << KrServices::fullPathName( "unzip" ) << "-t";
  else if ( type == "-tar" ) packer << KrServices::fullPathName( "tar" ) << "-tvf";
  else if ( type == "-tgz" ) packer << KrServices::fullPathName( "tar" ) << "-tvzf";
  else if ( type == "tarz" ) packer << KrServices::fullPathName( "tar" ) << "-tvzf";
  else if ( type == "-tbz" ) packer << KrServices::fullPathName( "tar" ) << "-tjvf";
  else if ( type == "-tlz" ) packer << KrServices::fullPathName( "tar" ) << "--lzma" << "-tvf";
  else if ( type == "gzip" ) packer << KrServices::fullPathName( "gzip" ) << "-tv";
  else if ( type == "zip2" ) packer << KrServices::fullPathName( "bzip2" ) << "-tv";
  else if ( type == "lzma" ) packer << KrServices::fullPathName( "lzma" ) << "-tv";
  else if ( type == "-rar" ) packer << KrServices::fullPathName( KrServices::cmdExist( "rar" ) ? "rar" : "unrar" ) << "t";
  else if ( type == "-ace" ) packer << KrServices::fullPathName( "unace" ) << "t";
  else if ( type == "-lha" ) packer << KrServices::fullPathName( "lha" ) << "t";
  else if ( type == "-arj" ) packer << KrServices::fullPathName( KrServices::cmdExist( "arj" ) ? "arj" : "unarj" ) << "t";
  else if ( type == "cpio" ) packer << KrServices::fullPathName( "cpio" ) << "--only-verify-crc" << "-tvF";
  else if ( type == "-7z" )  packer << KrServices::fullPathName( "7z" ) << "-y" << "t";
  else return false;

  if ( !password.isNull() ) {
    if ( type == "-zip" ) 
      packer << "-P" << password;
    if ( type == "-arj" )
      packer << QString("-g%1").arg(password);
    if ( type == "-ace" || type == "-rar" || type == "-7z" )
      packer << QString("-p%1").arg(password);
  }

  // unpack the files
  KrLinecountingProcess proc;
  proc << packer << archive;

  if( type == "-ace" && QFile( "/dev/ptmx" ).exists() ) // Don't remove, unace crashes if missing!!!
    proc.setStandardInputFile("/dev/ptmx");
  
  // tell the user to wait
  observer->subJobStarted( i18n( "Testing Archive" ), count );
  if ( count != 0 )
    connect( &proc, SIGNAL( newOutputLines( int ) ),
             observer, SLOT( incrementProgress( int ) ) );

  // start the unpacking process
  proc.start();
  // TODO make use of asynchronous process starting. waitForStarted(int msec = 30000) is blocking
  // it would be better to connect to started(), error() and finished()
  if (proc.waitForStarted()) while ( proc.state() == QProcess::Running ) {
    observer->processEvents();
    if( observer->wasCancelled() )
      proc.kill();
    }
  ; // busy wait - need to find something better...
  observer->subJobStopped();

  // check the return value
  if ( proc.exitStatus() != QProcess::NormalExit || !checkStatus( type, proc.exitCode() ) )
    return false;

  return true; // SUCCESS
  }

bool KRarcHandler::pack( QStringList fileNames, QString type, QString dest, long count, QMap<QString,QString> extraProps, KRarcObserver *observer ) {
  // set the right packer to do the job
  QStringList packer;

  if      ( type == "zip" ) { packer << KrServices::fullPathName( "zip" ) << "-ry"; type = "-zip"; }
  else if ( type == "tar" ) { packer << KrServices::fullPathName( "tar" ) << "-cvf"; type = "-tar"; }
  else if ( type == "tar.gz" ) { packer << KrServices::fullPathName( "tar" ) << "-cvzf"; type = "-tgz"; }
  else if ( type == "tar.bz2" ) { packer << KrServices::fullPathName( "tar" ) << "-cvjf"; type = "-tbz"; }
  else if ( type == "tar.lzma" ) { packer << KrServices::fullPathName( "tar" ) << "--lzma" << "-cvf"; type = "-tlz"; }
  else if ( type == "rar" ) { packer << KrServices::fullPathName( "rar" ) << "-r" << "a"; type = "-rar"; }
  else if ( type == "lha" ) { packer << KrServices::fullPathName( "lha" ) << "a"; type = "-lha"; }
  else if ( type == "arj" ) { packer << KrServices::fullPathName( "arj" ) << "-r" << "-y" << "a"; type = "-arj"; }
  else if ( type == "7z" ) {  packer << KrServices::fullPathName( "7z" ) << "-y" << "a"; type = "-7z"; }
  else return false;

  QString password = QString();
  
  if( extraProps.count( "Password" ) > 0 ) {
    password = extraProps[ "Password" ];

    if ( !password.isNull() ) {
      if ( type == "-zip" ) 
        packer << "-P" << password;
      else if ( type == "-arj" )
        packer << QString("-g%1").arg(password);
      else if ( type == "-ace" || type == "-7z" )
        packer << QString("-p%1").arg(password);
      else if ( type == "-rar" ) {
        if( extraProps.count( "EncryptHeaders" ) > 0 )
          packer << QString("-hp%1").arg(password);
        else
          packer << QString("-p%1").arg(password);
      }
      else
        password = QString();
    }
  }

  if( extraProps.count( "VolumeSize" ) > 0 ) {
     QString sizeStr = extraProps[ "VolumeSize" ];
     KIO::filesize_t size = sizeStr.toLongLong();

     if( size >= 10000 ) {
       if( type == "-arj" || type == "-rar" )
           packer << QString( "-v%1b" ).arg( sizeStr );
     }
  }

  if( extraProps.count( "CompressionLevel" ) > 0 ) {
     int level = extraProps[ "CompressionLevel" ].toInt() - 1;
     if ( level < 0 )
       level = 0;
     if ( level > 8 )
       level = 8;

     if( type == "-rar" ) {
       static const int rarLevels[] = { 0, 1, 2, 2, 3, 3, 4, 4, 5 };
       packer << QString( "-m%1" ).arg( rarLevels[ level ] );
     }
     else if( type == "-arj" ) {
       static const int arjLevels[] = { 0, 4, 4, 3, 3, 2, 2, 1, 1 };
       packer << QString( "-m%1" ).arg( arjLevels[ level ] );
     }
     else if( type == "-zip" ) {
       static const int zipLevels[] = { 0, 1, 2, 4, 5, 6, 7, 8, 9 };
       packer << QString( "-%1" ).arg( zipLevels[ level ] );
     }
     else if( type == "-7z" ) {
       static const int sevenZipLevels[] = { 0, 1, 2, 4, 5, 6, 7, 8, 9 };
       packer << QString( "-mx%1" ).arg( sevenZipLevels[ level ] );
     }
  }

  if( extraProps.count( "CommandLineSwitches" ) > 0 )
     packer << QString( "%1" ).arg( extraProps[ "CommandLineSwitches" ] );
  
  // prepare to pack
  KrLinecountingProcess proc;
  proc << packer << dest;

  for ( QStringList::Iterator file = fileNames.begin(); file != fileNames.end(); ++file ) {
    proc << *file;
    }

  // tell the user to wait
  observer->subJobStarted( i18n( "Packing File(s)" ), count );
  if ( count != 0 )
    connect( &proc, SIGNAL( newOutputLines( int ) ),
             observer, SLOT( incrementProgress( int ) ) );

  // start the packing process
  proc.start();
  // TODO make use of asynchronous process starting. waitForStarted(int msec = 30000) is blocking
  // it would be better to connect to started(), error() and finished()
  if (proc.waitForStarted()) while ( proc.state() == QProcess::Running ) {
    observer->processEvents();
    if( observer->wasCancelled() )
      proc.kill();
    }
  ; // busy wait - need to find something better...
  observer->subJobStopped();

  // check the return value
  if ( proc.exitStatus() != QProcess::NormalExit || !checkStatus( type, proc.exitCode() ) ) {
    observer->detailedError ( i18n( "Failed to pack %1!", dest ), 
                                observer->wasCancelled() ? i18n( "User cancelled." ) : proc.getErrorMsg() );
    return false;
    }

  KConfigGroup group( krConfig, "Archives" );
  if ( group.readEntry( "Test Archives", _TestArchives ) &&
       !test( dest, type, password, count, observer ) ) {
    observer->error( i18n( "Failed to pack: " ) + dest );
    return false;
    }
  return true; // SUCCESS
  }

QString KRarcHandler::getPassword( QString path ) {
	QString password;
	
	QString key = "krarc-" + path;
	
	if( !KWallet::Wallet::keyDoesNotExist(KWallet::Wallet::NetworkWallet(), KWallet::Wallet::PasswordFolder(), key ) ) {
		if( !KWallet::Wallet::isOpen( KWallet::Wallet::NetworkWallet() )  && wallet != 0 ) {
			delete wallet;
			wallet = 0;
		}
		if( wallet == 0 )
		{
			QWidget * actWindow = QApplication::activeWindow();
			if( actWindow == 0 )
				actWindow = MAIN_VIEW;
			wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(), actWindow->winId() );
		}
		if ( wallet && wallet->hasFolder( KWallet::Wallet::PasswordFolder() ) ) {
			wallet->setFolder( KWallet::Wallet::PasswordFolder() );
			QMap<QString,QString> map;
			if ( wallet->readMap( key, map ) == 0 ) {
				QMap<QString, QString>::ConstIterator it = map.constFind( "password" );
				if ( it != map.constEnd() )
					password = it.value();
			}
		}
	}
	
	bool keep = true;
	QString user = "archive";
	KIO::PasswordDialog passDlg( i18n("This archive is encrypted, please supply the password:"),
	                             user, true );
	passDlg.setPassword( password );
	if (passDlg.exec() == KIO::PasswordDialog::Accepted) {
		password = passDlg.password();
		if ( keep ) {
			if( !KWallet::Wallet::isOpen( KWallet::Wallet::NetworkWallet() ) && wallet != 0 ) {
				delete wallet;
				wallet = 0;
			}
			if ( !wallet )
			{
				QWidget * actWindow = QApplication::activeWindow();
				if( actWindow == 0 )
					actWindow = MAIN_VIEW;
				wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(), actWindow->winId() );
			}
			if ( wallet ) {
				bool ok = true;
				if ( !wallet->hasFolder( KWallet::Wallet::PasswordFolder() ) )
					ok = wallet->createFolder( KWallet::Wallet::PasswordFolder() );
				if ( ok ) {
					wallet->setFolder( KWallet::Wallet::PasswordFolder() );
					QMap<QString,QString> map;
					map.insert( "login", "archive" );
					map.insert( "password", password );
					wallet->writeMap( key, map );
				}
			}
		}
		return password;
	}
	
	return "";
}

bool KRarcHandler::isArchive(const KUrl& url) {
	QString protocol = url.protocol();
	if (arcProtocols.indexOf(protocol) != -1)
		return true;
	else return false;	
}

QString KRarcHandler::getType( bool &encrypted, QString fileName, QString mime, bool checkEncrypted ) {
	QString result = detectArchive( encrypted, fileName, checkEncrypted );
	if( result.isNull() )
		result = mime;
	else
		result = "-" + result;
	
	if( result.endsWith( "-7z" ) )
		result = "-7z";
	
	return result.right( 4 );
}


bool KRarcHandler::checkStatus( QString type, int exitCode ) {
	if( type == "-zip" || type == "-rar" || type == "-7z" )
		return exitCode == 0 || exitCode == 1;
	else if( type == "-ace" || type == "zip2" || type == "-lha" || type == "-rpm" || type == "cpio" ||
	         type == "-tar" || type == "tarz" || type == "-tbz" || type == "-tgz" || type == "-arj" ||
	         type == "-deb" || type == "-tlz" )
		return exitCode == 0;
	else if( type == "gzip" || type == "lzma" )
		return exitCode == 0 || exitCode == 2;
	else
		return exitCode == 0;
}

struct AutoDetectParams {
	QString type;
	int location;
	QString detectionString;
};

QString KRarcHandler::detectArchive( bool &encrypted, QString fileName, bool checkEncrypted ) {
	static AutoDetectParams autoDetectParams[] = {{"zip",  0, "PK\x03\x04"},
	                                              {"rar",  0, "Rar!\x1a" },
	                                              {"arj",  0, "\x60\xea" },
	                                              {"rpm",  0, "\xed\xab\xee\xdb"},
	                                              {"ace",  7, "**ACE**" },
	                                              {"bzip2",0, "\x42\x5a\x68\x39\x31" },
	                                              {"gzip", 0, "\x1f\x8b"}, 
	                                              {"deb",  0, "!<arch>\ndebian-binary   " },
	                                              {"7z",   0, "7z\xbc\xaf\x27\x1c" } };
	static int autoDetectElems = sizeof( autoDetectParams ) / sizeof( AutoDetectParams );
	
	encrypted = false;
	
	QFile arcFile( fileName );
	if ( arcFile.open( QIODevice::ReadOnly ) ) {
		char buffer[ 1024 ];
		long sizeMax = arcFile.read( buffer, sizeof( buffer ) );
		arcFile.close();
		
		for( int i=0; i < autoDetectElems; i++ ) {
			QString detectionString = autoDetectParams[ i ].detectionString;
			int location = autoDetectParams[ i ].location;
			
			int endPtr = detectionString.length() + location;
			if( endPtr > sizeMax )
				continue;
			
			int j=0;
			for(; j != detectionString.length(); j++ ) {
				if( detectionString[ j ] == '?' )
					continue;
				if( buffer[ location + j ] != detectionString[ j ] )
					break;
			}
			
			if( j == detectionString.length() ) {
				QString type = autoDetectParams[ i ].type;
				if( type == "bzip2" || type == "gzip" ) {
					KTar tapeArchive( fileName );
					if( tapeArchive.open( QIODevice::ReadOnly ) ) {
						tapeArchive.close();
						if( type == "bzip2" )
							type = "tbz";
						else
							type = "tgz";
					}
				}
				else if( type == "zip" )
					encrypted = (buffer[6] & 1);
				else if( type == "arj" ) {
					if( sizeMax > 4 ) {
						long headerSize = ((unsigned char *)buffer)[ 2 ] + 256*((unsigned char *)buffer)[ 3 ];
						long fileHeader = headerSize + 10;
						if( fileHeader + 9 < sizeMax && buffer[ fileHeader ] == (char)0x60 && buffer[ fileHeader + 1 ] == (char)0xea )
							encrypted = (buffer[ fileHeader + 8 ] & 1 );
					}
				}
				else if( type == "rar" ) {
					if( sizeMax > 13 && buffer[ 9 ] == (char)0x73 ) {
						if( buffer[ 10 ] & 0x80 ) { // the header is encrypted?
							encrypted = true;
						} else {
							long offset = 7;
							long mainHeaderSize = ((unsigned char *)buffer)[ offset+5 ] + 256*((unsigned char *)buffer)[ offset+6 ];
							offset += mainHeaderSize;
							while( offset + 10 < sizeMax ) {
								long headerSize = ((unsigned char *)buffer)[ offset+5 ] + 256*((unsigned char *)buffer)[ offset+6 ];
								bool isDir = (buffer[ offset+7 ] == '\0' ) && (buffer[ offset+8 ] == '\0' ) &&
								             (buffer[ offset+9 ] == '\0' ) && (buffer[ offset+10 ] == '\0' );
							             
								if( buffer[ offset + 2 ] != (char)0x74 )
									break;
								if( !isDir ) {
									encrypted = ( buffer[ offset + 3 ] & 4 ) != 0;
									break;
								}
								offset += headerSize;
							}
						}
					}
				}
				else if( type == "ace" ) {
						long offset = 0;
						long mainHeaderSize = ((unsigned char *)buffer)[ offset+2 ] + 256*((unsigned char *)buffer)[ offset+3 ] + 4;
						offset += mainHeaderSize;
						while( offset + 10 < sizeMax ) {
							long headerSize = ((unsigned char *)buffer)[ offset+2 ] + 256*((unsigned char *)buffer)[ offset+3 ] + 4;
							bool isDir = (buffer[ offset+11 ] == '\0' ) && (buffer[ offset+12 ] == '\0' ) &&
							             (buffer[ offset+13 ] == '\0' ) && (buffer[ offset+14 ] == '\0' );
							             
							if( buffer[ offset + 4 ] != (char)0x01 )
								break;
							if( !isDir ) {
								encrypted = ( buffer[ offset + 6 ] & 64 ) != 0;
								break;
							}
							offset += headerSize;
						}
				}
				else if( type == "7z" ) {
					if( checkEncrypted ) { // encryption check is expensive
					                       // check only if it's necessary
						Kr7zEncryptionChecker proc;
						// TODO encorporate all this in Kr7zEncryptionChecker
						proc << KrServices::fullPathName( "7z" ) << "-y" << "t";
						proc << fileName;
						proc.start();
						proc.waitForFinished();
						encrypted = proc.isEncrypted();
					}
				}
				return type;
			}
		}
		
		if( sizeMax >= 512 ) {
			/* checking if it's a tar file */
			unsigned checksum = 32*8;
			char chksum[ 9 ];
			for( int i=0; i != 512; i++ )
				checksum += ((unsigned char *)buffer)[ i ];
			for( int i=148; i != 156; i++ )
				checksum -= ((unsigned char *)buffer)[ i ];
			sprintf( chksum, "0%o", checksum );
			if( !memcmp( buffer + 148, chksum, strlen( chksum ) ) ) {
				int k = strlen( chksum );
				for(; k < 8; k++ )
					if( buffer[148+k] != 0 && buffer[148+k] != 32 )
						break;
				if( k==8 )
					return "tar";
			}
		}
	}
	
	if( fileName.endsWith( ".tar.lzma" ) || fileName.endsWith( ".tlz" ) )
		return "tlz";
	if( fileName.endsWith( ".lzma" ) )
		return "lzma";
	
	return QString();
}

#include "krarchandler.moc"

