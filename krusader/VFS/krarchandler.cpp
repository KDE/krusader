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
#include <klineeditdlg.h>
// Krusader includes
#include "krarchandler.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../krservices.h"

QStringList KRarcHandler::supportedPackers(){
  QStringList packers;

  // we will simply try to find the packers here..
  if( KrServices::cmdExist("tar")   ) packers.append("tar");
  if( KrServices::cmdExist("gzip")  ) packers.append("gzip");
  if( KrServices::cmdExist("bzip2") ) packers.append("bzip2");
  if( KrServices::cmdExist("unzip") ) packers.append("unzip");
  if( KrServices::cmdExist("zip")   ) packers.append("zip");
  if( KrServices::cmdExist("rpm")   ) packers.append("rpm");
  if( KrServices::cmdExist("cpio")  ) packers.append("cpio");
  if( KrServices::cmdExist("unrar") ) packers.append("unrar");
  if( KrServices::cmdExist("rar")   ) packers.append("rar");
  if( KrServices::cmdExist("unarj") ) packers.append("unarj");
  if( KrServices::cmdExist("unace") ) packers.append("unace");

  // kdDebug() << "Supported Packers:" << endl;
  //QStringList::Iterator it;
  //for( it = packers.begin(); it != packers.end(); ++it )
  // kdDebug() << *it << endl;

  return packers;
}

bool KRarcHandler::arcSupported(QString type){
   // lst will contain the supported unpacker list...
  krConfig->setGroup("Archives");
  QStringList lst=krConfig->readListEntry("Supported Packers");

  if      (type == "-zip" && lst.contains("unzip") )
    return true;
  else if (type == "-tar" && lst.contains("tar") )
    return true;
  else if (type == "-tbz" && lst.contains("tar") )
    return true;
  else if (type == "-tgz" && lst.contains("tar") )
    return true;
  else if (type == "tarz" && lst.contains("tar") )
    return true;
  else if( type == "gzip" && lst.contains("gzip") )
    return true;
  else if( type == "zip2" && lst.contains("bzip2") )
    return true;
  else if( type == "-rar" && lst.contains("unrar") )
    return true;
  else if( type == "-ace" && lst.contains("unace") )
    return true;
  else if( type == "-arj" && lst.contains("unarj") )
    return true;
  else if( type == "-rpm" && lst.contains("cpio") )
    return true;
	else if( type == "cpio" && lst.contains("cpio") )
    return true;
  // not supported : (
	return false;
}

bool KRarcHandler::arcHandled(QString type){
  // first check if supported
  if( !arcSupported(type) ) return false;
  krConfig->setGroup("Archives");
	if((type == "-tgz" && krConfig->readBoolEntry("Do GZip" ,_DoGZip )  ) ||
     (type == "tarz" && krConfig->readBoolEntry("Do GZip" ,_DoGZip )  ) ||
	   (type == "-tar" && krConfig->readBoolEntry("Do Tar"  ,_DoTar  )  ) ||
	   (type == "-tbz" && krConfig->readBoolEntry("Do BZip2",_DoBZip2)  ) ||
	   (type == "gzip" && krConfig->readBoolEntry("Do GZip" ,_DoGZip )  ) ||
	   (type == "zip2" && krConfig->readBoolEntry("Do BZip2",_DoBZip2)  ) ||
	   (type == "-zip" && krConfig->readBoolEntry("Do UnZip",_DoUnZip)  ) ||
	   (type == "-rar" && krConfig->readBoolEntry("Do UnRar",_DoUnRar)  ) ||
	   (type == "-arj" && krConfig->readBoolEntry("Do UnArj",_DoUnarj)  ) ||
	   (type == "-ace" && krConfig->readBoolEntry("Do UnAce",_DoUnAce)  ) ||
		 (type == "cpio" && krConfig->readBoolEntry("Do RPM"  ,_DoRPM  )  ) ||
	   (type == "-rpm" && krConfig->readBoolEntry("Do RPM"  ,_DoRPM  )  ) )
	  return true;
  else
    return false;
}


long KRarcHandler::arcFileCount(QString archive, QString type){
  // first check if supported
  if( !arcSupported(type) ) return false;

  // bzip an gzip archive contains only one file
  if( type == "zip2" || type == "gzip" ) return 1L;

  // set the right lister to do the job
  QString lister;

       if( type == "-zip" ) lister = "unzip -ZTs";
  else if( type == "-tar" ) lister = "tar -tvf";
  else if( type == "-tgz" ) lister = "tar -tvzf";
  else if( type == "tarz" ) lister = "tar -tvzf";
  else if( type == "-tbz" ) lister = "tar -tjvf";
  else if( type == "-rar" ) lister = "unrar l";
  else if( type == "-ace" ) lister = "unace l";
  else if( type == "-arj" ) lister = "unarj l";
  else return 0L;

  // tell the user to wait
  krApp->startWaiting(i18n("Counting files in archive"));

  // count the number of files in the archive
  long count = 1;
  KTempFile tmpFile("tmp","krusader-unpack");
  KShellProcess list;
  list << lister << + "\""+archive+"\"" << ">" << tmpFile.name() ;
  list.start(KProcess::Block);
  QTextStream *stream = tmpFile.textStream();
  while ( stream->readLine() != QString::null ) ++count;
  tmpFile.unlink();

  //make sure you call stopWait after this function return...
  //krApp->stopWait();

  return count;
}

bool KRarcHandler::unpack(QString archive, QString type, QString dest ) {
  // test first - or be sorry later...
  if ( !test(archive,type,0) ){
		KMessageBox::error(krApp,i18n("Failed to unpack")+" \""+archive+"\" !");
   	return false;
	}

	// count the files in the archive
  long count = arcFileCount(archive,type);
  if( count == 0 ) return false; // not supported
  if( count == 1 ) count = 0 ;

	// choose the right packer for the job
  QString packer;

  // set the right packer to do the job
       if( type == "-zip" ) packer = "unzip -o";
  else if( type == "-tar" ) packer = "tar -xvf";
  else if( type == "-tgz" ) packer = "tar -xvzf";
  else if( type == "tarz" ) packer = "tar -xvzf";
  else if( type == "-tbz" ) packer = "tar -xjvf";
  else if( type == "gzip" ) packer = "gzip -d";
  else if( type == "zip2" ) packer = "bzip2 -d";
  else if( type == "-rar" ) packer = "unrar x";
  else if( type == "-ace" ) packer = "unace x";
  else if( type == "-arj" ) packer = "unarj x";
  else return false;

  // unpack the files
  KShellProcess proc;
  proc << packer << +"\""+archive+"\"";
  QString save = getcwd(0,0);
  chdir(dest.local8Bit());

  // tell the user to wait
  krApp->startWaiting(i18n("Unpacking File(s)"),count);
  if ( count != 0 )
		connect(&proc,SIGNAL(receivedStdout(KProcess*,char*,int)),
                krApp, SLOT(incProgress(KProcess*,char*,int)) );

  // start the unpacking process
  proc.start(KProcess::NotifyOnExit,KProcess::AllOutput);
  while( proc.isRunning() ) qApp->processEvents(); // busy wait - need to find something better...
  krApp->stopWait();
  chdir(save.local8Bit());

  // check the return value
  if( !proc.normalExit() ){
	   KMessageBox::error(krApp,i18n("Failed to unpack")+" \""+archive+"\" !");
	   return false;
	}
	return true; // SUCCESS
}

bool KRarcHandler::test(QString archive, QString type, long count, QString password){
  // choose the right packer for the job
  QString packer;

  // set the right packer to do the job
       if( type == "-zip" ) packer = "unzip -t";
  else if( type == "-tar" ) packer = "tar -tvf";
  else if( type == "-tgz" ) packer = "tar -tvzf";
  else if( type == "tarz" ) packer = "tar -tvzf";
  else if( type == "-tbz" ) packer = "tar -tjvf";
  else if( type == "gzip" ) packer = "gzip -tv";
  else if( type == "zip2" ) packer = "bzip2 -tv";
  else if( type == "-rar" ) packer = "unrar t";
  else if( type == "-ace" ) packer = "unace t";
  else if( type == "-arj" ) packer = "unarj t";
  else if( type == "cpio" ) packer = "cpio --only-verify-crc -tvF" ;
  else return false;

	if( !password.isEmpty() ){
  	if( type == "-zip") packer = "unzip -P "+password+" -t ";
	}

  // unpack the files
  KShellProcess proc;
  proc << packer << +"\""+archive+"\"";

  // tell the user to wait
  krApp->startWaiting(i18n("Testing Archive"), count);
  if ( count != 0 ) connect(&proc,SIGNAL(receivedStdout(KProcess*,char*,int)),
                                krApp, SLOT(incProgress(KProcess*,char*,int)) );

  // start the unpacking process
  proc.start(KProcess::NotifyOnExit,KProcess::AllOutput);
  while( proc.isRunning() ) qApp->processEvents(); // busy wait - need to find something better...
  krApp->stopWait();

	// check the return value
  if( !proc.normalExit() || proc.exitStatus() != 0 )
    return false;
	
	return true; // SUCCESS
}

bool KRarcHandler::pack(QStringList fileNames, QString type, QString dest, long count ){
  // set the right packer to do the job
  QString packer;
       if( type ==  "zip"    ) { packer = "zip -ry";   type = "-zip"; }
  else if( type ==  "tar"    ) { packer = "tar -cvf";  type = "-tar"; }
  else if( type ==  "tar.gz" ) { packer = "tar -cvzf"; type = "-tgz"; }
  else if( type ==  "tar.bz2") { packer = "tar -cvjf"; type = "-tbz"; }
  else if( type ==  "rar"    ) { packer = "rar -r a";  type = "-rar"; }
  else return false;

  // prepare to pack
  KShellProcess proc;
  proc << packer << "\""+dest+"\"";

  for(QStringList::Iterator file = fileNames.begin(); file != fileNames.end(); ++file){
    proc << "\""+(*file)+"\"";
  }

  // tell the user to wait
  krApp->startWaiting(i18n("Packing File(s)"), count);
  if (count != 0 )
		connect(&proc,SIGNAL(receivedStdout(KProcess*,char*,int)),
                krApp, SLOT(incProgress(KProcess*,char*,int)) );

  // start the packing process
  proc.start(KProcess::NotifyOnExit,KProcess::AllOutput);
  while( proc.isRunning() ) qApp->processEvents(); // busy wait - need to find something better...
  krApp->stopWait();

  // check the return value
  if( !proc.normalExit() ){
		KMessageBox::error(krApp,i18n("Error"),i18n("Failed to pack: ")+dest.latin1() );
    return false;
	}

	krConfig->setGroup("Archives");
  if ( krConfig->readBoolEntry( "Test Archives",_TestArchives ) &&
       !test(dest,type,count) ){
  	KMessageBox::error(krApp,i18n("Error"),i18n("Failed to pack")+dest.latin1() );
    return false;
	}
  return true; // SUCCESS
}

QString KRarcHandler::getPassword(QString archive, QString type){
	if( type != "-zip" ) return QString::null;
	
	KRarcHandler handler;
	handler.inSet = 0;
	
	KShellProcess proc;
	proc << "unzip -t" << archive;
  connect(&proc,SIGNAL(receivedStdout(KProcess*,char*,int)),
                &handler, SLOT(setPassword(KProcess*,char*,int)) );
	
	proc.start(KProcess::NotifyOnExit,KProcess::AllOutput);
  while( proc.isRunning() ) qApp->processEvents(); // busy wait - need to find something better...

	return handler.password;
}

void KRarcHandler::setPassword(KProcess * proc,char *buffer,int){
	//while ( inSet != 1 );
  if ( inSet == 2 ) return;
	inSet = 1;

	password = password + buffer;
	if( !password.contains('\n') ){
		inSet = 0;
		return;
	}
  else password = password.mid(password.find('\n')+1 );
	if( password.length() < 10 ){
		inSet = 0;
		return;
  }
	
	proc->kill(SIGKILL);
	inSet = 2;	

	if( password.lower().contains("password") ){
		bool ok;
  	password = KLineEditDlg::getText(
         "This archive is encrypted, please supply the password:",
         "",&ok,krApp);
		if(!ok) password = "123"; // no way someone will use this pass	
	}
	else password = QString::null;
}

#include "krarchandler.moc"
