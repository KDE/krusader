/***************************************************************************   
                       		vfile.cpp
                   		-------------------
    copyright            : (C) 2000 by Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------

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
// System includes
#include <unistd.h>
// Krusader includes
#include "vfile.h"
#include "krpermhandler.h"

vfile::vfile(QString name,	                  // useful construtor
						unsigned long size,	
						QString perm,
						QString	dateTime,
						bool symLink,
						uid_t	owner,
						gid_t group,
						QString mime,
						QString symDest,
						mode_t mode){
	vfile_name=name;
	vfile_size=size;
	vfile_ownerId=owner;
	vfile_groupId=group;
	vfile_perm=perm;
	vfile_dateTime=dateTime;
	vfile_symLink=symLink;
	vfile_mimeType=mime;
	vfile_symDest=symDest;
	vfile_mode=mode;
}

vfile::vfile(QString name,	                  // useful construtor
						unsigned long size,	
						QString perm,
						QString	dateTime,
						bool symLink,
						QString	owner,
						QString group,
						QString mime,
						QString symDest,
						mode_t mode){
		vfile_name=name;
		vfile_size=size;
		if (owner == QString::null) vfile_ownerId = geteuid() ;
		else vfile_ownerId= KRpermHandler::user2uid(owner) ;
		vfile_groupId=			KRpermHandler::group2gid(group);
		vfile_perm=perm;
		vfile_dateTime=dateTime;
		vfile_symLink=symLink;
		vfile_mimeType=mime;
		vfile_symDest=symDest;
		vfile_mode=mode;
}

char vfile::vfile_isReadable(){
	return KRpermHandler::readable(vfile_perm,vfile_groupId,vfile_ownerId);
}

char vfile::vfile_isWriteable(){
	return KRpermHandler::writeable(vfile_perm,vfile_groupId,vfile_ownerId);
}

char vfile::vfile_isExecutable(){
	return KRpermHandler::executable(vfile_perm,vfile_groupId,vfile_ownerId);
}
