/***************************************************************************
                                 krsearchmod.cpp
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

#include "krsearchmod.h"
#include "krquery.h"
#include "../krusader.h"
#include "../resources.h"
#include "../VFS/arc_vfs.h"
#include "../VFS/vfile.h"
#include "../VFS/krpermhandler.h"
#include "../VFS/krarchandler.h"

#include <klocale.h>
#include <qdir.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <klargefile.h>

#include <kmimetype.h>

KRSearchMod::KRSearchMod(const KRQuery* q) {
	stopSearch = false; /// ===> added
	query = new KRQuery(*q);
	query->normalize();
}

KRSearchMod::~KRSearchMod(){
	delete query;
}

void KRSearchMod::start() {
	// search every dir that needs to be searched
	for(unsigned int i=0; i<query->whereToSearch.count(); ++i)
		scanDir( *(query->whereToSearch.at(i)) );

  emit finished();
}

void KRSearchMod::stop() {
  stopSearch = true;
}

bool KRSearchMod::checkPerm(QString perm){
	QString q_perm=query->perm;
	for( int i=0; i<9; ++i )
		if( q_perm[i] != '?' && q_perm[i] != perm[i+1]) return false;
	return true;
}

bool KRSearchMod::checkType(QString mime){
	QString type = query->type;
	if( type == mime ) return true;
	if( type == i18n("Archives") )
		return KRarcHandler::arcSupported(mime.right(4));
	if( type == i18n("Directories") ) return mime.contains("directory");
	if( type == i18n("Image Files") ) return mime.contains("image/");
	if( type == i18n("Text Files")  ) return mime.contains("text/");
	if( type == i18n("Video Files") ) return mime.contains("video/");
	if( type == i18n("Audio Files") ) return mime.contains("audio/");
	if( type == i18n("Custom") ) return query->customType.contains(mime);
	return false;
}

bool KRSearchMod::fileMatch(const QString name){
	unsigned int len;
	for(unsigned int i=0; i<query->matches.count(); ++i){
	  QRegExp(*query->matches.at(i),query->matchesCaseSensitive,true).match(name,0,(int*)&len);
    if( len==name.length()) return true;
	}
  return false;
}

void KRSearchMod::scanDir( QString dir){
	if( stopSearch ) return;
	if( dir.right(1)!="/" && !dir.isEmpty() ) dir=dir+"/";
	if( query->whereNotToSearch.contains(dir) ) return;
	if( scanedDirs.contains(dir) ) return ; // don't scan dirs twice
	scanedDirs.append( dir );
	// let the gui know where we are
	emit searching(dir);
	qApp->processEvents();
	
  DIR* d = opendir(dir.local8Bit());
  if(!d) return;

	struct dirent* dirEnt;
  QString name;
	unsigned long size;
  time_t mtime;
  KURL url;
	
	while( (dirEnt=readdir(d)) != NULL ){
    QString mime = QString::null;
    name = dirEnt->d_name;
 		// we dont scan the ".",".." enteries
		if (name=="." || name == "..") continue;
		
    url.setPath(dir+name);
		KDE_struct_stat stat_p;
	  KDE_lstat(dir.local8Bit()+name.local8Bit(),&stat_p);
 	  if( query->recurse ){
				if( S_ISLNK(stat_p.st_mode) && query->followLinks ){
            scanDir( QDir(dir+"/"+name).canonicalPath() );
				}
				else if( S_ISDIR(stat_p.st_mode) ) scanDir(dir+name);
	  }
		if( query->inArchive ){
			mime = KMimeType::findByURL(url,stat_p.st_mode,true,false)->name();
			QString type = mime.right(4);
      if( mime.contains("-rar") ) type="-rar";
    	
			if( KRarcHandler::arcSupported(type) ){
      	scanArchive(dir+name,type);
			}
		}
    // see if the name matches
    if( !fileMatch(name) ) continue;
    // check that the size fit
    size = stat_p.st_size;
		if( query->minSize && size < query->minSize ) continue;
    if( query->maxSize && size > query->maxSize ) continue;
    // check the time frame
    mtime = stat_p.st_mtime;
		if( query->olderThen && mtime > query->olderThen ) continue;
    if( query->newerThen && mtime < query->newerThen ) continue;
    // check the type
    if( !query->type.isEmpty() ) {
      if ( mime.isEmpty() )
				mime = KMimeType::findByURL(url,stat_p.st_mode,true,false)->name();
		  if ( !checkType(mime) ) continue;
		}
		// check owner name
		if( !query->owner.isEmpty() &&
         stat_p.st_uid != KRpermHandler::user2uid(query->owner) ) continue;
    // check group name
		if( !query->group.isEmpty() &&
         stat_p.st_gid != KRpermHandler::group2gid(query->group) ) continue;
		// check permission
		if( !query->perm.isEmpty() &&
         !checkPerm(KRpermHandler::mode2QString(stat_p.st_mode)) ) continue;
		// check if it contains the text (avoid the /dev diretory).
    // grep code from KFind ( the copyright goes with our thanx to the unknown author )
    if ( !query->contain.isEmpty() && !dir.startsWith("/dev") ){
			QFile qf(dir+name);

			qf.open(IO_ReadOnly);
			QTextStream text(&qf);
      text.setEncoding(QTextStream::Locale);
      QString line;
			while(!stopSearch && !text.atEnd() ){
        line = text.readLine();
        if (line.isNull() ) break;
        if (line.find(query->contain, 0, query->containCaseSensetive) != -1)
         	  break;
         	//qApp->processEvents(); // is that needed ?
      }
      if (line.isNull() || text.atEnd() ) continue;
    }

    // if we got here - we got a winner
    results.append(dir+name);
		//  kdWarning() << "Found: " << (dir+name).local8Bit() << endl;
    emit found(name, dir, (KIO::filesize_t)stat_p.st_size, stat_p.st_mtime,KRpermHandler::mode2QString(stat_p.st_mode) );
    qApp->processEvents();
  }
	// clean up
	closedir(d);
}

void KRSearchMod::scanArchive( QString archive, QString type ){
	if( stopSearch ) return;
	// ace and rar archives are currently not suported
	if (type == "-arj" || type == "-ace" ) return;
	
	vfs *v = new arc_vfs(archive,type,0,true);
   	
	if ( v->vfs_error() ){
			kdWarning() << "Failed to open vfs: " << archive.local8Bit() << endl;
			delete v;
			return;
	}
  emit searching(archive);
  qApp->processEvents();
	scanVfsDir(v,"\\",archive);
	delete v;
}

void KRSearchMod::scanVfsDir( vfs* v, QString dir, QString archive ){
	v->vfs_refresh(dir);
	for( vfile* vf=v->vfs_getFirstFile(); vf != 0 ; vf=v->vfs_getNextFile() ){
		QString name =  vf->vfile_getName();
		if ( vf->vfile_isDir() ){
			scanVfsDir(v,dir+"/"+name,archive);
			v->vfs_refresh(dir);
		}
    // see if the name matches
    if( !fileMatch(name) ) continue;
    // check that the size fit
    KIO::filesize_t size = vf->vfile_getSize();
		if( query->minSize && size < query->minSize ) continue;
    if( query->maxSize && size > query->maxSize ) continue;
		// check the time frame
    time_t mtime = vf->vfile_getTime_t();
		if( query->olderThen && mtime > query->olderThen ) continue;
    if( query->newerThen && mtime < query->newerThen ) continue;
    // check owner name
		if( !query->owner.isEmpty() &&
				vf->vfile_getUid() != KRpermHandler::user2uid(query->owner) ) continue;
    // check group name
		if( !query->group.isEmpty() &&
    		vf->vfile_getGid() != KRpermHandler::group2gid(query->group) ) continue;
		//check permission
		if( !query->perm.isEmpty() && !checkPerm(vf->vfile_getPerm()) ) continue;

		// if we got here - we got a winner
    results.append(dir+name);
		emit found(name, archive+dir, size, vf->vfile_getTime_t(),vf->vfile_getPerm());
    qApp->processEvents();
	}
}

#include "krsearchmod.moc"
