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
#include <kprocess.h>
#include <kurl.h>
#include <kwallet.h>
#include <unistd.h> // for setsid, see Kr7zEncryptionChecker::setupChildProcess
#include <signal.h> // for kill

class KRarcObserver : public QObject
{
  Q_OBJECT
public:
  virtual ~KRarcObserver() {}

  virtual void processEvents() = 0;
  virtual void subJobStarted( const QString & jobTitle, int count ) = 0;
  virtual void subJobStopped() = 0;
  virtual bool wasCancelled() = 0;
  virtual void error( const QString & error ) = 0;
  virtual void detailedError( const QString & error, const QString & details ) = 0;

public slots:
  virtual void incrementProgress( int ) = 0;
};

class KRarcHandler: public QObject {
  Q_OBJECT
public:
  static KRarcObserver *defaultObserver;

  // return the number of files in the archive
  static long arcFileCount(QString archive, QString type, QString password, KRarcObserver *observer = defaultObserver);
  // unpack an archive to destination directory
  static bool unpack(QString archive, QString type, QString password, QString dest, KRarcObserver *observer = defaultObserver );
  // pack an archive to destination directory
  static bool pack(QStringList fileNames, QString type, QString dest, long count, QMap<QString,QString> extraProps, KRarcObserver *observer = defaultObserver );
  // test an archive
  static bool test(QString archive, QString type, QString password, long count = 0L, KRarcObserver *observer = defaultObserver );
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

/**
 * A Process which emits how manny lines it is writing to stdout or atderr.
 */
class KrLinecountingProcess : public KProcess {
	Q_OBJECT
public:
	KrLinecountingProcess() : KProcess() {
	setOutputChannelMode(KProcess::SeparateChannels); // without this output redirection has no effect!
		connect(this, SIGNAL(readyReadStandardError()), SLOT(receivedError()) );
		connect(this, SIGNAL(readyReadStandardOutput()), SLOT(receivedOutput()) );
	}

	QString getErrorMsg() {
		if( errorData.trimmed().isEmpty() )
			return QString::fromLocal8Bit(outputData);
		else
			return QString::fromLocal8Bit(errorData);
	}
	
public slots:
	void receivedError() {
		QByteArray newData(this->readAllStandardError());
		emit newErrorLines(newData.count('\n'));
		errorData += newData;
		if( errorData.length() > 500 )
			errorData = errorData.right( 500 );
		receivedOutput(newData);
	}
	
	void receivedOutput(QByteArray newData = QByteArray()) {
		if (newData.isEmpty())
			newData = this->readAllStandardOutput();
		emit newOutputLines(newData.count('\n'));
		outputData += newData;
		if( outputData.length() > 500 )
			outputData = outputData.right( 500 );
	}

signals:
	void newOutputLines(int count);
	void newErrorLines(int count);

private:
	QByteArray errorData;
	QByteArray outputData;
};

class Kr7zEncryptionChecker : public KProcess {
	Q_OBJECT
	
public:
	Kr7zEncryptionChecker() : KProcess(), encrypted( false ), lastData() {
		setOutputChannelMode(KProcess::SeparateChannels); // without this output redirection has no effect!
		connect(this, SIGNAL(readyReadStandardOutput()), SLOT(receivedOutput()) );
	}

protected:
  virtual void setupChildProcess() {
    // This function is called after the fork but for the exec. We create a process group
    // to work around a broken wrapper script of 7z. Without this only the wrapper is killed.
    setsid(); // make this process leader of a new process group
  }

public slots:
	void receivedOutput() {
		QString data =  QString::fromLocal8Bit(this->readAllStandardOutput());
		
		QString checkable = lastData + data;
		
		QStringList lines = checkable.split( '\n' );
		lastData = lines[ lines.count() - 1 ];
		for( int i=0; i != lines.count(); i++ ) {
			QString line = lines[ i ].trimmed().toLower();
			int ndx = line.indexOf( "testing" );
			if( ndx >=0 )
				line.truncate( ndx );
			if( line.isEmpty() )
				continue;
			
			if( line.contains( "password" ) && line.contains( "enter" ) ) {
				encrypted = true;
				::kill(- pid(), SIGKILL); // kill the whole process group by giving the negativ PID
			}
		}
	}

	bool isEncrypted() { return encrypted; }
private:
	bool encrypted;
	QString lastData;
};

#endif
