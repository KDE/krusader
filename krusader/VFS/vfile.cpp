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
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
// Qt includes
#include <qdatetime.h>
// Krusader includes
#include "vfile.h"
#include "krpermhandler.h"

#include <kdebug.h>

vfile::vfile(QString name,	                  // useful construtor
						unsigned long size,	
						QString perm,
						time_t mtime,
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
	vfile_time_t=mtime;
	vfile_symLink=symLink;
	vfile_mimeType=mime;
	vfile_symDest=symDest;
	vfile_mode=mode;
  if (vfile_isDir())
    vfile_size = 0;
}

vfile::vfile(QString name,	                  // useful construtor
						unsigned long size,	
						QString perm,
						time_t mtime,
						bool symLink,
						QString	owner,
						QString group,
						QString mime,
						QString symDest,
						mode_t mode){
		vfile_name=name;
		vfile_size=size;
    vfile_owner=owner;
    vfile_group=group;
		vfile_ownerId=KRpermHandler::user2uid(owner) ;
		vfile_groupId=KRpermHandler::group2gid(group);
		vfile_perm=perm;
		vfile_time_t=mtime;
		vfile_symLink=symLink;
		vfile_mimeType=mime;
		vfile_symDest=symDest;
		vfile_mode=mode;
    if (vfile_isDir())
    vfile_size = 0;
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

QString vfile::vfile_getOwner(){
  if( vfile_owner.isEmpty() )
    vfile_owner=KRpermHandler::uid2user(vfile_getUid());
  return vfile_owner;
}

QString vfile::vfile_getGroup(){
  if( vfile_group.isEmpty() )
    vfile_group=KRpermHandler::uid2user(vfile_getGid());
  return vfile_group;
}

KIO::UDSEntry vfile::vfile_getEntry(){
	KIO::UDSEntry entry;
	KIO::UDSAtom atom;

	atom.m_uds = KIO::UDS_NAME;
	atom.m_str = vfile_getName();
	entry.append(atom);

	atom.m_uds = KIO::UDS_SIZE;
	atom.m_long = vfile_getSize();
	entry.append(atom);

 	atom.m_uds = KIO::UDS_MODIFICATION_TIME;
	atom.m_long = vfile_getTime_t();
	entry.append(atom);

	atom.m_uds = KIO::UDS_USER;
	atom.m_str = vfile_getOwner();
	entry.append(atom);

	atom.m_uds = KIO::UDS_GROUP;
	atom.m_str = vfile_getOwner(); 
	entry.append(atom);

 	atom.m_uds = KIO::UDS_MIME_TYPE;
	atom.m_str = vfile_getMime();
	entry.append(atom);

	atom.m_uds = KIO::UDS_FILE_TYPE;
	atom.m_long = vfile_getMode() & S_IFMT;
	entry.append(atom);

	atom.m_uds = KIO::UDS_ACCESS;
	atom.m_long = vfile_getMode() & 07777; // keep permissions only
	entry.append( atom );

 	atom.m_uds = KIO::UDS_MIME_TYPE;
	atom.m_str = vfile_getMime();
	entry.append(atom);

  if( vfile_isSymLink() ){
		atom.m_uds = KIO::UDS_LINK_DEST;
		atom.m_str = vfile_getSymDest();
		entry.append(atom);
  }

	return entry;
}
