/***************************************************************************
                                arc_vfs.h
                             -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
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

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ARC_VFS_H
#define ARC_VFS_H

#include <sys/types.h>
#include "vfs.h"
#include <qvaluestack.h>
#include <kprocess.h>
#include <qfile.h>
#include <qprogressdialog.h>

class arc_vfs : public vfs  {
  Q_OBJECT
  class arc_dir;
  class extFile;
public:
	arc_vfs(QString origin,QString type,QObject* panel,bool write);
 ~arc_vfs();
	
	// copy a file to the vfs (physical)
	void    vfs_addFiles(KURL::List *fileUrls,KIO::CopyJob::CopyMode mode,QObject* toNotify=0,QString dir = "");	
	// remove a file from the vfs (physical)
	void 		vfs_delFiles(QStringList *fileNames);	
	// return a path to the file
	QString vfs_getFile(QString name);
	KURL::List* vfs_getFiles(QStringList* names);
	// make dir
	void vfs_mkdir(QString name);
	// rename file
	void vfs_rename(QString fileName,QString newName);
	// calculate space
	void vfs_calcSpace(QString name ,KIO::filesize_t *totalSize,unsigned long *totalFiles,unsigned long *totalDirs, bool* stop);
	// return the working dir
	QString vfs_workingDir();
	
public slots:
	// actually reads files and stats
	bool vfs_refresh(QString origin);
  void repack();
	
protected:
  QString tmpDir;   // the temp directory tha archive is using
  QString arcFile;  // the archive file URL
  bool changed;     // true if repack changed the archive
  QList<arc_dir> dirList;
  QValueList<extFile> extFiles; // the name, time & size of files unpacked from this archive

  void processName(const QString& name,QStringList *urls,KIO::filesize_t *totalSize,unsigned long *totalFiles );
  bool getDirs();   // fill the dir list
  vfileDict* findDir(QString name);
  arc_dir* findArcDir(QString name);
  void getFilesToPack  (QStringList* filesToPack,QString dir_name = "");
	void getFilesToDelete(QStringList* filesToDelete,QString dir_name = "");
  void getExtFiles( QString dir_name="" );
  QString nextWord( QString &s, char d=' ' );
  QString changeDir(QString name);

  void parseLine(QString line,QFile* temp);

	QString prefix;
  QString cmd;      // the archiver main command
  QString listCmd;  // the file listing option
  QString delCmd;   // the delete option
  QString addCmd;   // the add files option
  QString getCmd;   // the extract files option

  // the interl progress bar variale
  int ignoreLines; // no of lines to ignore on stdout

private:
  class arc_dir{
    public:
    arc_dir(QString _name){
      name = _name;
      entries.setAutoDelete(true);
    }
    QString name;         // the name of the dir
    vfileDict entries; // the file and dir in this dir
  };

  class extFile{
    public:
    extFile(): url(""),time(0),size(0){}
    extFile(QString u): url(u),time(0),size(0){}
    extFile(QString u,time_t t,off_t s): url(u),time(t),size(s){}
    bool operator==(const extFile& ef) const{
      if( url != ef.url ) return false;
      if( size*ef.size && size!=ef.size )return false;
      if( time*ef.time && time!=ef.time )return false;
      return true;
    }
    QString url;
    time_t time;
    off_t  size;
  };



};

#endif
