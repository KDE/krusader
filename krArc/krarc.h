/***************************************************************************
                                   krarc.h
                             -------------------
    begin                : Sat Jun 14 14:42:49 IDT 2003
    copyright            : (C) 2003 by Rafi Yanai & Shie Erlich
    email                : yanai@users.sf.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __krarc_h__
#define __krarc_h__

#include <qstring.h>
#include <q3cstring.h>
#include <q3dict.h>
#include <qfile.h>
#include <sys/types.h>

#include <kurl.h>
#include <kio/global.h>
#include <kio/slavebase.h>
#include <k3process.h>

class K3Process;
class KFileItem;
class Q3CString;

class kio_krarcProtocol : public QObject, public KIO::SlaveBase {
Q_OBJECT
public:
	kio_krarcProtocol(const Q3CString &pool_socket, const Q3CString &app_socket);
	virtual ~kio_krarcProtocol();
	virtual void stat( const KUrl & url );
	virtual void get(const KUrl& url);
	virtual void put(const KUrl& url,int permissions,bool overwrite,bool resume);
	virtual void mkdir(const KUrl& url,int permissions);
	virtual void listDir(const KUrl& url);
	virtual void del(KUrl const & url, bool isFile);
	virtual void copy (const KUrl &src, const KUrl &dest, int permissions, bool overwrite);

public slots:
	void receivedData(K3Process* proc,char* buf,int len);
	void checkOutputForPassword( K3Process*,char*,int );

protected:
	virtual bool   initDirDict(const KUrl& url,bool forced = false);
	virtual bool   initArcParameters();
	QString detectArchive( bool &encrypted, QString fileName );
	virtual void parseLine(int lineNo, QString line, QFile* temp);
	virtual bool setArcFile(const KUrl& url);
	virtual QString getPassword();
	virtual void invalidatePassword();

	// archive specific commands
	QString cmd;     ///< the archiver name.
	QString listCmd; ///< list files. 
	QString getCmd;  ///< unpack files command.
	QString delCmd;  ///< delete files command.
	QString putCmd;  ///< add file command.
	QString copyCmd; ///< copy to file command.

private:
	void get(const KUrl& url, int tries);
	/** checks if the exit code is OK. */
	bool checkStatus( int exitCode );
	/** service function for parseLine. */
	QString nextWord(QString &s,char d=' ');
	/** translate permittion string to mode_t. */
	mode_t parsePermString(QString perm);
	/** return the name of the directory inside the archive. */
	QString findArcDirectory(const KUrl& url);
	/** find the UDSEntry of a file in a directory. */
	KIO::UDSEntry* findFileEntry(const KUrl& url);
	/** add a new directory (file list container). */
	KIO::UDSEntryList* addNewDir(QString path);
	QString fullPathName( QString name );
	QString convertFileName( QString name );
	static QString convertName( QString name );
	static QString escape( QString name );
	
	Q3Dict<KIO::UDSEntryList> dirDict; //< the directoris data structure.
	bool encrypted;                   //< tells whether the archive is encrypted
	bool archiveChanged;              //< true if the archive was changed.
	bool archiveChanging;             //< true if the archive is currently changing.
	bool newArchiveURL;               //< true if new archive was entered for the protocol
	KIO::filesize_t decompressedLen;  //< the number of the decompressed bytes
	KFileItem* arcFile;               //< the archive file item.
	QString arcPath;                  //< the archive location
	QString arcTempDir;               //< the currently used temp directory.
	QString arcType;                  //< the archive type.
	bool extArcReady;                 //< Used for RPM & DEB files.
	QString password;                 //< Password for the archives
	KConfig *krConfig;                //< The configuration file for krusader
	KConfigGroup confGrp;             //< the 'Dependencies' config group
	
	QString lastData;
	QString encryptedArchPath;
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
		errorMsg += QString::fromLocal8Bit( buf, len );
		if( errorMsg.length() > 500 )
			errorMsg = errorMsg.right( 500 );
		receivedOutputMsg( 0, buf, len );
	}
	
	void receivedOutputMsg(K3Process*, char *buf, int len) {
		outputMsg += QString::fromLocal8Bit( buf, len );
		if( outputMsg.length() > 500 )
			outputMsg = outputMsg.right( 500 );
	}
	
private:
	QString errorMsg;
	QString outputMsg;
};

#endif
