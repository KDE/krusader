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
#include <qcstring.h>
#include <qdict.h>
#include <qfile.h>
#include <sys/types.h>

#include <kurl.h>
#include <kio/global.h>
#include <kio/slavebase.h>

class KProcess;
class KFileItem;
class QCString;

class kio_krarcProtocol : public QObject, public KIO::SlaveBase
{
Q_OBJECT
public:
  kio_krarcProtocol(const QCString &pool_socket, const QCString &app_socket);
  virtual ~kio_krarcProtocol();
	virtual void stat( const KURL & url );
  virtual void get(const KURL& url);
	virtual void put(const KURL& url,int permissions,bool overwrite,bool resume);
	virtual void mkdir(const KURL& url,int permissions);
	virtual void listDir(const KURL& url);
	virtual void del(KURL const & url, bool isFile);

public slots:
	void receivedData(KProcess* proc,char* buf,int len);

protected:
	virtual bool initDirDict(const KURL& url,bool forced = false);
	virtual bool initArcParameters();
	virtual void parseLine(int lineNo, QString line, QFile* temp);
	virtual bool setArcFile(const QString& path);
  virtual QString getPassword();

	// archive specific commands
	QString cmd;     ///< the archiver name.
	QString listCmd; ///< list files. 
	QString getCmd;  ///< unpack files command.
	QString delCmd;  ///< delete files command.
	QString putCmd;  ///< add file commnad.

private:
	/** service function for parseLine. */
	QString nextWord(QString &s,char d=' ');
	/** translate permittion string to mode_t. */
	mode_t parsePermString(QString perm);
  /** return the name of the directory inside the archive. */
	QString findArcDirectory(const KURL& url);
	/** find the UDSEntry of a file in a directory. */
	KIO::UDSEntry* findFileEntry(const KURL& url);
  /** add a new directory (file list container). */
  KIO::UDSEntryList* addNewDir(QString path);
  QString fullPathName( QString name );
  
	QDict<KIO::UDSEntryList> dirDict; //< the directoris data structure.
  bool archiveChanged;              //< true if the archive was changed.
  bool archiveChanging;             //< true if the archive is currently changing.
	KFileItem* arcFile;               //< the archive file item.
  QString arcPath;                  //< the archive location
	QString arcTempDir;               //< the currently used temp directory.
  QString arcType;                  //< the archive type.
  bool cpioReady;                   //< Used for RPM files.
  QString password;                 //< Password for the archives
  KConfig *krConfig;                //< The configuration file for krusader
};


#endif
