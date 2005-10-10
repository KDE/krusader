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
#include <qtextstream.h> 
// KDE includes
#include <kprocess.h>
#include <ktempfile.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kinputdialog.h> 
#include <qfile.h>
// Krusader includes
#include "krarchandler.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../krservices.h"

static QStringList arcProtocols = QStringList::split(";", "tar;bzip;bzip2;gzip;krarc;zip");

QStringList KRarcHandler::supportedPackers() {
  QStringList packers;

  // we will simply try to find the packers here..
  if ( KrServices::cmdExist( "tar" ) ) packers.append( "tar" );
  if ( KrServices::cmdExist( "gzip" ) ) packers.append( "gzip" );
  if ( KrServices::cmdExist( "bzip2" ) ) packers.append( "bzip2" );
  if ( KrServices::cmdExist( "unzip" ) ) packers.append( "unzip" );
  if ( KrServices::cmdExist( "zip" ) ) packers.append( "zip" );
  if ( KrServices::cmdExist( "rpm" ) ) packers.append( "rpm" );
  if ( KrServices::cmdExist( "lha" ) ) packers.append( "lha" );
  if ( KrServices::cmdExist( "cpio" ) ) packers.append( "cpio" );
  if ( KrServices::cmdExist( "unrar" ) ) packers.append( "unrar" );
  if ( KrServices::cmdExist( "rar" ) ) packers.append( "rar" );
  if ( KrServices::cmdExist( "arj" ) ) packers.append( "arj" );
  if ( KrServices::cmdExist( "unarj" ) ) packers.append( "unarj" );
  if ( KrServices::cmdExist( "unace" ) ) packers.append( "unace" );

  // kdDebug() << "Supported Packers:" << endl;
  //QStringList::Iterator it;
  //for( it = packers.begin(); it != packers.end(); ++it )
  // kdDebug() << *it << endl;

  return packers;
  }

bool KRarcHandler::arcSupported( QString type ) {
  // lst will contain the supported unpacker list...
  krConfig->setGroup( "Archives" );
  QStringList lst = krConfig->readListEntry( "Supported Packers" );

  removeAliases( type );
  
  if ( type == "-zip" && lst.contains( "unzip" ) )
    return true;
  else if ( type == "-tar" && lst.contains( "tar" ) )
    return true;
  else if ( type == "-tbz" && lst.contains( "tar" ) )
    return true;
  else if ( type == "-tgz" && lst.contains( "tar" ) )
    return true;
  else if ( type == "tarz" && lst.contains( "tar" ) )
    return true;
  else if ( type == "gzip" && lst.contains( "gzip" ) )
    return true;
  else if ( type == "zip2" && lst.contains( "bzip2" ) )
    return true;
  else if ( type == "-lha" && lst.contains( "lha" ) )
    return true;
  else if ( type == "-rar" && ( lst.contains( "unrar" ) || lst.contains( "rar" ) ) )
    return true;
  else if ( type == "-ace" && lst.contains( "unace" ) )
    return true;
  else if ( type == "-arj" && ( lst.contains( "unarj" ) || lst.contains( "arj" ) ) )
    return true;
  else if ( type == "-rpm" && lst.contains( "cpio" ) )
    return true;
  else if ( type == "cpio" && lst.contains( "cpio" ) )
    return true;
  // not supported : (
  return false;
  }

bool KRarcHandler::arcHandled( QString type ) {
  // first check if supported
  if ( !arcSupported( type ) ) return false;

  removeAliases( type );
  
  krConfig->setGroup( "Archives" );
  if ( ( type == "-tgz" && krConfig->readBoolEntry( "Do GZip" , _DoGZip ) ) ||
       ( type == "tarz" && krConfig->readBoolEntry( "Do GZip" , _DoGZip ) ) ||
       ( type == "-tar" && krConfig->readBoolEntry( "Do Tar" , _DoTar ) ) ||
       ( type == "-tbz" && krConfig->readBoolEntry( "Do BZip2", _DoBZip2 ) ) ||
       ( type == "gzip" && krConfig->readBoolEntry( "Do GZip" , _DoGZip ) ) ||
       ( type == "zip2" && krConfig->readBoolEntry( "Do BZip2", _DoBZip2 ) ) ||
       ( type == "-zip" && krConfig->readBoolEntry( "Do UnZip", _DoUnZip ) ) ||
       ( type == "-lha" && krConfig->readBoolEntry( "Do Lha", _DoUnZip ) ) ||
       ( type == "-rar" && krConfig->readBoolEntry( "Do UnRar", _DoUnRar ) ) ||
       ( type == "-arj" && krConfig->readBoolEntry( "Do UnArj", _DoUnarj ) ) ||
       ( type == "-ace" && krConfig->readBoolEntry( "Do UnAce", _DoUnAce ) ) ||
       ( type == "cpio" && krConfig->readBoolEntry( "Do RPM" , _DoRPM ) ) ||
       ( type == "-rpm" && krConfig->readBoolEntry( "Do RPM" , _DoRPM ) ) )
    return true;
  else
    return false;
  }


long KRarcHandler::arcFileCount( QString archive, QString type ) {
  // first check if supported
  if ( !arcSupported( type ) ) return false;

  // bzip an gzip archive contains only one file
  if ( type == "zip2" || type == "gzip" ) return 1L;

  // set the right lister to do the job
  QString lister;

  removeAliases( type );
  
  if ( type == "-zip" ) lister = KrServices::fullPathName( "unzip" ) + " -ZTs";
  else if ( type == "-tar" ) lister = KrServices::fullPathName( "tar" ) + " -tvf";
  else if ( type == "-tgz" ) lister = KrServices::fullPathName( "tar" ) + " -tvzf";
  else if ( type == "tarz" ) lister = KrServices::fullPathName( "tar" ) + " -tvzf";
  else if ( type == "-tbz" ) lister = KrServices::fullPathName( "tar" ) + " -tjvf";
  else if ( type == "-lha" ) lister = KrServices::fullPathName( "lha" ) + " l";
  else if ( type == "-rar" ) lister = KrServices::fullPathName( KrServices::cmdExist( "rar" ) ? "rar" : "unrar" ) + " l";
  else if ( type == "-ace" ) lister = KrServices::fullPathName( "unace" ) + " l";
  else if ( type == "-arj" ) lister = KrServices::fullPathName( KrServices::cmdExist( "arj" ) ? "arj" : "unarj" ) + " l";
  else return 0L;

  // tell the user to wait
  krApp->startWaiting( i18n( "Counting files in archive" ) );

  // count the number of files in the archive
  long count = 1;
  KTempFile tmpFile( /*"tmp"*/ QString::null, "krusader-unpack" ); // commented out as it created files in the current dir!
  KShellProcess list;
  list << lister << convertName( archive ) << ">" << tmpFile.name() ;
  if( type == "-ace" && QFile( "/dev/ptmx" ).exists() )  // Don't remove, unace crashes if missing!!!
    list<< "<" << "/dev/ptmx";
  list.start( KProcess::Block );
  QTextStream *stream = tmpFile.textStream();
  while ( stream && stream->readLine() != QString::null ) ++count;
  tmpFile.unlink();

  //make sure you call stopWait after this function return...
  //krApp->stopWait();

  return count;
  }

void KRarcHandler::removeAliases( QString &type ) {
  // jar files are handled as zips
  if( type == "-jar" )
    type = "-zip";
  }
  
bool KRarcHandler::unpack( QString archive, QString type, QString dest ) {
  krConfig->setGroup( "Archives" );
  if ( krConfig->readBoolEntry( "Test Before Unpack", _TestBeforeUnpack ) ) {
    // test first - or be sorry later...
    if ( !test( archive, type, 0 ) ) {
      KMessageBox::error( krApp, i18n( "Failed to unpack" ) + " \"" + archive + "\" !" );
      return false;
      }
    }

  // count the files in the archive
  long count = arcFileCount( archive, type );
  if ( count == 0 ) return false; // not supported
  if ( count == 1 ) count = 0 ;

  // choose the right packer for the job
  QString packer;

  removeAliases( type );
  
  // set the right packer to do the job
  if ( type == "-zip" ) packer = KrServices::fullPathName( "unzip" ) + " -o" ;
  else if ( type == "-tar" ) packer = KrServices::fullPathName( "tar" ) + " -xvf";
  else if ( type == "-tgz" ) packer = KrServices::fullPathName( "tar" ) + " -xvzf";
  else if ( type == "tarz" ) packer = KrServices::fullPathName( "tar" ) + " -xvzf";
  else if ( type == "-tbz" ) packer = KrServices::fullPathName( "tar" ) + " -xjvf";
  else if ( type == "gzip" ) packer = KrServices::fullPathName( "gzip" ) + " -cd";
  else if ( type == "zip2" ) packer = KrServices::fullPathName( "bzip2" ) + " -cdk";
  else if ( type == "-lha" ) packer = KrServices::fullPathName( "lha" ) + " xf";
  else if ( type == "-rar" ) packer = KrServices::fullPathName( KrServices::cmdExist( "rar" ) ? "rar" : "unrar" ) + " -y x";
  else if ( type == "-ace" ) packer = KrServices::fullPathName( "unace" ) + " x";
  else if ( type == "-arj" ) packer = KrServices::cmdExist( "arj" ) ?
                                      KrServices::fullPathName( "arj" ) + " -y x" :
                                      KrServices::fullPathName( "unarj" ) + " x";
  else return false;

  // unpack the files
  KShellProcess proc;
  proc << packer << " " + convertName( archive );
  if( type == "zip2" || type=="gzip" ){
    QString arcname = archive.mid(archive.findRev("/")+1);
    if( arcname.contains(".") ) arcname = arcname.left(arcname.findRev("."));
    proc << ">" << convertName( dest+"/"+arcname );
  }
  if( type == "-ace" && QFile( "/dev/ptmx" ).exists() ) // Don't remove, unace crashes if missing!!!
    proc << "<" << "/dev/ptmx";
  
  QString save = getcwd( 0, 0 );
  chdir( dest.local8Bit() );

  // tell the user to wait
  krApp->startWaiting( i18n( "Unpacking File(s)" ), count );
  if ( count != 0 )
    connect( &proc, SIGNAL( receivedStdout( KProcess*, char*, int ) ),
             krApp, SLOT( incProgress( KProcess*, char*, int ) ) );

  // start the unpacking process
  proc.start( KProcess::NotifyOnExit, KProcess::AllOutput );
  while ( proc.isRunning() ) {
    usleep( 1000 );
    qApp->processEvents();
    }
  ; // busy wait - need to find something better...
  krApp->stopWait();
  chdir( save.local8Bit() );

  // check the return value
  if ( !proc.normalExit() ) {
    KMessageBox::error( krApp, i18n( "Failed to unpack" ) + " \"" + archive + "\" !" );
    return false;
    }
  return true; // SUCCESS
  }

bool KRarcHandler::test( QString archive, QString type, long count, QString password ) {
  // choose the right packer for the job
  QString packer;

  removeAliases( type );

  // set the right packer to do the job
  if ( type == "-zip" ) packer = KrServices::fullPathName( "unzip" ) + " -t";
  else if ( type == "-tar" ) packer = KrServices::fullPathName( "tar" ) + " -tvf";
  else if ( type == "-tgz" ) packer = KrServices::fullPathName( "tar" ) + " -tvzf";
  else if ( type == "tarz" ) packer = KrServices::fullPathName( "tar" ) + " -tvzf";
  else if ( type == "-tbz" ) packer = KrServices::fullPathName( "tar" ) + " -tjvf";
  else if ( type == "gzip" ) packer = KrServices::fullPathName( "gzip" ) + " -tv";
  else if ( type == "zip2" ) packer = KrServices::fullPathName( "bzip2" ) + " -tv";
  else if ( type == "-rar" ) packer = KrServices::fullPathName( KrServices::cmdExist( "rar" ) ? "rar" : "unrar" ) + " t";
  else if ( type == "-ace" ) packer = KrServices::fullPathName( "unace" ) + " t";
  else if ( type == "-lha" ) packer = KrServices::fullPathName( "lha" ) + " t";
  else if ( type == "-arj" ) packer = KrServices::fullPathName( KrServices::cmdExist( "arj" ) ? "arj" : "unarj" ) + " t";
  else if ( type == "cpio" ) packer = KrServices::fullPathName( "cpio" ) + " --only-verify-crc -tvF" ;
  else return false;

  if ( !password.isEmpty() ) {
    if ( type == "-zip" ) packer = KrServices::fullPathName( "unzip" ) + " -P " + password + " -t ";
    }

  // unpack the files
  KShellProcess proc;
  proc << packer << convertName( archive );

  if( type == "-ace" && QFile( "/dev/ptmx" ).exists() ) // Don't remove, unace crashes if missing!!!
    proc << "<" << "/dev/ptmx";
  
  // tell the user to wait
  krApp->startWaiting( i18n( "Testing Archive" ), count );
  if ( count != 0 ) connect( &proc, SIGNAL( receivedStdout( KProcess*, char*, int ) ),
                               krApp, SLOT( incProgress( KProcess*, char*, int ) ) );

  // start the unpacking process
  proc.start( KProcess::NotifyOnExit, KProcess::AllOutput );
  while ( proc.isRunning() ) {
    usleep( 1000 );
    qApp->processEvents();
    }
  ; // busy wait - need to find something better...
  krApp->stopWait();

  // check the return value
  if ( !proc.normalExit() || proc.exitStatus() != 0 )
    return false;

  return true; // SUCCESS
  }

bool KRarcHandler::pack( QStringList fileNames, QString type, QString dest, long count ) {
  // set the right packer to do the job
  QString packer;
  removeAliases( type );
  
  if ( type == "zip" ) { packer = KrServices::fullPathName( "zip" ) + " -ry"; type = "-zip"; } 
  else if ( type == "tar" ) { packer = KrServices::fullPathName( "tar" ) + " -cvf"; type = "-tar"; } 
  else if ( type == "tar.gz" ) { packer = KrServices::fullPathName( "tar" ) + " -cvzf"; type = "-tgz"; } 
  else if ( type == "tar.bz2" ) { packer = KrServices::fullPathName( "tar" ) + " -cvjf"; type = "-tbz"; } 
  else if ( type == "rar" ) { packer = KrServices::fullPathName( "rar" ) + " -r a"; type = "-rar"; } 
  else if ( type == "lha" ) { packer = KrServices::fullPathName( "lha" ) + " a"; type = "-lha"; } 
  else if ( type == "arj" ) { packer = KrServices::fullPathName( "arj" ) + " -r a"; type = "-arj"; } 
  else return false;

  // prepare to pack
  KShellProcess proc;
  proc << packer << convertName( dest );

  for ( QStringList::Iterator file = fileNames.begin(); file != fileNames.end(); ++file ) {
    proc << convertName( *file );
    }

  // tell the user to wait
  krApp->startWaiting( i18n( "Packing File(s)" ), count );
  if ( count != 0 )
    connect( &proc, SIGNAL( receivedStdout( KProcess*, char*, int ) ),
             krApp, SLOT( incProgress( KProcess*, char*, int ) ) );

  // start the packing process
  proc.start( KProcess::NotifyOnExit, KProcess::AllOutput );
  while ( proc.isRunning() ) {
    usleep( 1000 );
    qApp->processEvents();
    }
  ; // busy wait - need to find something better...
  krApp->stopWait();

  // check the return value
  if ( !proc.normalExit() || proc.exitStatus() != 0 ) {
    KMessageBox::error( krApp, i18n( "Failed to pack: " ) + dest, i18n( "Error" ) );
    return false;
    }

  krConfig->setGroup( "Archives" );
  if ( krConfig->readBoolEntry( "Test Archives", _TestArchives ) &&
       !test( dest, type, count ) ) {
    KMessageBox::error( krApp, i18n( "Failed to pack: " ) + dest, i18n( "Error" ) );
    return false;
    }
  return true; // SUCCESS
  }

QString KRarcHandler::getPassword( QString archive, QString type ) {
  removeAliases( type );
  
  if ( type != "-zip" ) return QString::null;

  KRarcHandler handler;
  handler.inSet = 0;

  KShellProcess proc;
  proc << KrServices::fullPathName( "unzip" ) + " -t" << archive;
  connect( &proc, SIGNAL( receivedStdout( KProcess*, char*, int ) ),
           &handler, SLOT( setPassword( KProcess*, char*, int ) ) );

  proc.start( KProcess::NotifyOnExit, KProcess::AllOutput );
  while ( proc.isRunning() ) {
    usleep( 1000 );
    qApp->processEvents();
    }
  ; // busy wait - need to find something better...

  return handler.password;
  }

void KRarcHandler::setPassword( KProcess * proc, char *buffer, int ) {
  //while ( inSet != 1 );
  if ( inSet == 2 ) return ;
  inSet = 1;

  password = password + buffer;
  if ( !password.contains( '\n' ) ) {
    inSet = 0;
    return ;
    } else password = password.mid( password.find( '\n' ) + 1 );
  if ( password.length() < 10 ) {
    inSet = 0;
    return ;
    }

  proc->kill( SIGKILL );
  inSet = 2;

  if ( password.lower().contains( "password" ) ) {
    bool ok;
    password = KInputDialog::getText(i18n("Password Needed"),
                 i18n("This archive is encrypted, please supply the password:"),
                 "", &ok, krApp );
    if ( !ok ) password = "123"; // no way someone will use this pass
    } else password = QString::null;
  }

bool KRarcHandler::isArchive(const KURL& url) {
	QString protocol = url.protocol();
	if (arcProtocols.find(protocol) != arcProtocols.end())
		return true;
	else return false;	
}


QString KRarcHandler::convertName( QString name ) {
  if( !name.contains( '\'' ) )
    return "'" + name + "'";
  if( !name.contains( '"' ) && !name.contains( '$' ) )
    return "\"" + name + "\"";
  return escape( name );
}

QString KRarcHandler::escape( QString name ) {
  const QString evilstuff = "\\\"'`()[]{}!?;$&<>| ";		// stuff that should get escaped
     
    for ( unsigned int i = 0; i < evilstuff.length(); ++i )
        name.replace( evilstuff[ i ], ('\\' + evilstuff[ i ]) );

  return name;
}

  
#include "krarchandler.moc"

