/***************************************************************************
                                 krarchandler.h
                             -------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description: this class will supply static archive handling functions.
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KRARCHANDLER_H
#define KRARCHANDLER_H

#include <qstringlist.h>
#include <qobject.h>
#include <k3process.h>
#include <kurl.h>
#include <kwallet.h>

class KRarcHandler: public QObject {
  Q_OBJECT
public:
  // return the number of files in the archive
  static long arcFileCount(QString archive, QString type, QString password);
  // unpack an archive to destination directory
  static bool unpack(QString archive, QString type, QString password, QString dest );
  // pack an archive to destination directory
  static bool pack(QStringList fileNames, QString type, QString dest, long count, QMap<QString,QString> extraProps );
  // test an archive
  static bool test(QString archive, QString type, QString password, long count = 0L );
  // true - if the right unpacker exist in the system
  static bool arcSupported(QString type);
  // true - if supported and the user want us to handle this kind of archive
  static bool arcHandled(QString type);
  // return the a list of supported packers
  static QStringList supportedPackers();
  // true - if the url is an archive (ie: tar:/home/test/file.tar.bz2)
  static bool isArchive(const KUrl& url);
  // used to determine the type of the archive
  static QString getType( bool &encrypted, QString fileName, QString mime, bool checkEncrypted = true );
  // queries the password from the user
  static QString getPassword( QString path );
  // detects the archive type
  static QString detectArchive( bool &encrypted, QString fileName, bool checkEncrypted = true );
private:
  // checks if the returned status is correct
  static bool checkStatus( QString type, int exitCode );

  static KWallet::Wallet * wallet;
};

class KrShellProcess : public K3ShellProcess {
	Q_OBJECT
public:
	KrShellProcess() : K3ShellProcess(), errorMsg( QString() ), outputMsg( QString() ) {
		connect(this,SIGNAL(receivedStderr(K3Process*,char*,int)),
				this,SLOT(receivedErrorMsg(K3Process*,char*,int)) );
		connect(this,SIGNAL(receivedStdout(K3Process*,char*,int)),
				this,SLOT(receivedOutputMsg(K3Process*,char*,int)) );
	}
	
	QString getErrorMsg() {
		if( errorMsg.trimmed().isEmpty() )
			return outputMsg.right( 500 );
		else
			return errorMsg.right( 500 );
	}
	
public slots:
	void receivedErrorMsg(K3Process*, char *buf, int len) {
		emit newErrorLines(countLines(buf, len));
		errorMsg += QString::fromLocal8Bit( buf, len );
		if( errorMsg.length() > 500 )
			errorMsg = errorMsg.right( 500 );
		receivedOutputMsg( 0, buf, len );
	}
	
	void receivedOutputMsg(K3Process*, char *buf, int len) {
		emit newOutputLines(countLines(buf, len));
		outputMsg += QString::fromLocal8Bit( buf, len );
		if( outputMsg.length() > 500 )
			outputMsg = outputMsg.right( 500 );
	}

signals:
	void newOutputLines(int count);
	void newErrorLines(int count);

private:
	int countLines(char* buf, int len) {
		int lines = 0;
		for ( int i = 0 ; i < len; ++i )
			if ( buf[ i ] == '\n' )
				++lines;
		return lines;
	}

	QString errorMsg;
	QString outputMsg;
};

class Kr7zEncryptionChecker : public KrShellProcess {
	Q_OBJECT
	
public:
	Kr7zEncryptionChecker() : KrShellProcess(), encrypted( false ), lastData() {
		connect(this,SIGNAL(receivedStdout(K3Process*,char*,int)),
				this,SLOT(processStdout(K3Process*,char*,int)) );
	}

public slots:
	void processStdout( K3Process *proc,char *buf,int len ) {
		QByteArray d(len);
		d.setRawData(buf,len);
		QString data =  QString( d );
		d.resetRawData(buf,len);
		
		QString checkable = lastData + data;
		
		QStringList lines = QStringList::split( '\n', checkable );
		lastData = lines[ lines.count() - 1 ];
		for( int i=0; i != lines.count(); i++ ) {
			QString line = lines[ i ].trimmed().toLower();
			int ndx = line.find( "testing" );
			if( ndx >=0 )
				line.truncate( ndx );
			if( line.isEmpty() )
				continue;
			
			if( line.contains( "password" ) && line.contains( "enter" ) ) {
				encrypted = true;
				proc->kill();
			}
		}
	}

	bool isEncrypted() { return encrypted; }
private:
	bool encrypted;
	QString lastData;
};

#endif
