/***************************************************************************
	  	                        vfs.cpp
  	                    -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
  ------------------------------------------------------------------------
   the vfs class is an extendable class which by itself does (almost)
   nothing. other VFSs like the normal_vfs inherits from this class and
   make it possible to use a consistent API for all types of VFSs.

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

#include "vfs.h"
#include <kapplication.h>
#include <time.h>
#include "../krusader.h"

vfs::vfs(QObject* panel, bool quiet): quietMode(quiet),vfileIterator(0){
		if ( panel ){
	 		connect(this,SIGNAL(startUpdate()),panel,SLOT(slotStartUpdate()));
		}
		else quietMode = true;
}

KIO::filesize_t vfs::vfs_totalSize(){
	KIO::filesize_t temp=0;
	class vfile* vf=vfs_getFirstFile();
		
	while (vf!=0){
		if ( (vf->vfile_getName() != ".") && ( vf->vfile_getName() != "..")  
		     && !(vf->vfile_isDir()) )
				temp+=vf->vfile_getSize();
		vf=vfs_getNextFile();
	}
	return temp;                                                     
}

bool vfs::vfs_refresh(KIO::Job* job){
  if(job && job->error()){
		job->showErrorDialog(krApp);
	}
	return vfs_refresh(vfs_origin);
}

KURL vfs::fromPathOrURL( const QString &originIn )
{
  QString password, loginName, origin = originIn;
  bool bugfix = false;
  
  if ( originIn.contains( ":/" ) && !originIn.startsWith( "/" ) )
  {
    // breakdown the url;
    /* FIXME: untill KDE fixes the bug we have to check for
       passwords and users with @ in them... */
    bugfix = origin.find("@") != origin.findRev("@");
    if(bugfix){
      if(origin.find(":") != origin.findRev(":", origin.findRev("@") )){
        int passStart = origin.find( ":",origin.find(":")+1 )+1;
        int passLen = origin.findRev("@")-passStart;
        password = origin.mid(passStart,passLen);
        origin = origin.remove(passStart-1,passLen+1);
      }
      if(origin.find("@") != origin.findRev("@")){
        int usrStart = origin.find( "/" )+1;
        if(origin.at(usrStart) == '/') ++usrStart;
        int usrLen = origin.findRev("@")-usrStart;
        loginName = origin.mid(usrStart,usrLen);
        origin = origin.remove(usrStart,usrLen+1);
      }
    }
  }
  KURL url = KURL::fromPathOrURL( origin );
  if(loginName.isEmpty()) loginName = url.user();
  if(password.isEmpty())  password  = url.pass();
  if(bugfix){
    url.setPass(password);
    url.setUser(loginName);
  }

  return url;
}

void vfs::setVfsFilesP(QDict<vfile>* dict){
	vfs_filesP=dict;
	if( vfileIterator ) delete vfileIterator;
	vfileIterator = new QDictIterator<vfile>(*dict);
}

#include "vfs.moc"
