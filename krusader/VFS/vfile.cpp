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
// KDE includes
#include <kmimetype.h>
#include <kdeversion.h>
// Krusader includes
#include "vfile.h"
#include "krpermhandler.h"
#include "normal_vfs.h"

#include <kdebug.h>

vfile::vfile(const QString& name,	                  // useful construtor
             const KIO::filesize_t size,
             const QString& perm,
             const time_t mtime,
             const bool symLink,
             const uid_t	owner,
             const gid_t group,
             const QString& mime,
             const QString& symDest,
             const mode_t mode,
             const int rwx)
{
	vfile_name=name;
	vfile_size=size;
	vfile_owner=QString();
	vfile_ownerId=owner;  
	vfile_group=QString();
	vfile_groupId=group;
	vfile_userName=QString();
	vfile_perm=perm;
	vfile_time_t=mtime;
	vfile_symLink=symLink;
	vfile_mimeType=mime;
	vfile_symDest=symDest;
	vfile_mode=mode;
	vfile_isdir = ( perm[ 0 ] == 'd' );
	if (vfile_isDir() && !vfile_symLink )
		vfile_size = 0;
	vfile_rwx = rwx;
	vfile_acl_loaded = false;
}

vfile::vfile(const QString& name,	                  // useful construtor
             const KIO::filesize_t size,	
             const QString& perm,
             const time_t mtime,
             const bool symLink,
             const QString& owner,
             const QString& group,
             const QString& userName,
             const QString& mime,
             const QString& symDest,
             const mode_t mode,
             const int rwx,
             const QString& aclString,
             const QString& aclDfltString ){
	vfile_name=name;
	vfile_size=size;
	vfile_owner=owner;
	vfile_group=group;
	vfile_userName=userName;
	vfile_ownerId=KRpermHandler::user2uid(owner) ;
	vfile_groupId=KRpermHandler::group2gid(group);
	vfile_perm=perm;
	vfile_time_t=mtime;
	vfile_symLink=symLink;
	vfile_mimeType=mime;
	vfile_symDest=symDest;
	vfile_mode=mode;
	vfile_isdir = ( perm[ 0 ] == 'd' );
	if ( vfile_isDir() && !vfile_symLink )
		vfile_size = 0;
	vfile_acl = aclString;
	vfile_def_acl = aclDfltString;
	vfile_has_acl = !aclString.isNull() || !aclDfltString.isNull();
	vfile_acl_loaded = true;
	vfile_rwx = rwx;
}

char vfile::vfile_isReadable() const {
	if( vfile_rwx == PERM_ALL )
		return ALLOWED_PERM;
	else if( vfile_userName.isNull() )
		return KRpermHandler::readable(vfile_perm,vfile_groupId,vfile_ownerId,vfile_rwx);
	else
		return KRpermHandler::ftpReadable(vfile_owner, vfile_userName, vfile_perm);
}

char vfile::vfile_isWriteable() const {
	if( vfile_rwx == PERM_ALL )
		return ALLOWED_PERM;
	else if( vfile_userName.isNull() )
		return KRpermHandler::writeable(vfile_perm,vfile_groupId,vfile_ownerId,vfile_rwx);
	else
		return KRpermHandler::ftpWriteable(vfile_owner, vfile_userName, vfile_perm);
}

char vfile::vfile_isExecutable() const {
	if( vfile_rwx == PERM_ALL )
	{
		if(( vfile_mode & 0111 ) || vfile_isdir )
			return ALLOWED_PERM;
		else
			return NO_PERM;
	}
	else if( vfile_userName.isNull() )
		return KRpermHandler::executable(vfile_perm,vfile_groupId,vfile_ownerId,vfile_rwx);
	else
		return KRpermHandler::ftpExecutable(vfile_owner, vfile_userName, vfile_perm);
}

const QString& vfile::vfile_getMime(bool fast){
	if( vfile_mimeType == QString() ){ // mimetype == "" is OK so don't check mimetype.empty() !
		vfile_mimeType = KMimeType::findByURL( vfile_getUrl(),vfile_getMode(),vfile_getUrl().isLocalFile(),fast)->name();
		if( vfile_mimeType.contains("directory") ) vfile_perm[0] = 'd', vfile_isdir = true;
	}
	return vfile_mimeType;
}

const QString& vfile::vfile_getOwner(){
	if( vfile_owner.isEmpty() )
		vfile_owner=KRpermHandler::uid2user(vfile_getUid());
	return vfile_owner;
}

const QString& vfile::vfile_getGroup(){
	if( vfile_group.isEmpty() )
		vfile_group=KRpermHandler::gid2group(vfile_getGid());
	return vfile_group;
}

const QString& vfile::vfile_getACL(){
	if( !vfile_acl_loaded )
		vfile_loadACL();
	return vfile_acl;
}

const QString& vfile::vfile_getDefaultACL(){
	if( !vfile_acl_loaded )
		vfile_loadACL();
	return vfile_def_acl;
}

void vfile::vfile_loadACL()
{
	if( vfile_url.isLocalFile() )
	{
		normal_vfs::getACL( this, vfile_acl, vfile_def_acl );
		vfile_has_acl = !vfile_acl.isNull() || !vfile_def_acl.isNull();
	}
	vfile_acl_loaded = true;
}

const KIO::UDSEntry vfile::vfile_getEntry() {
	KIO::UDSEntry entry;
	KIO::UDSAtom atom;

	atom.m_uds = KIO::UDSEntry::UDS_NAME;
	atom.m_str = vfile_getName();
	entry.append(atom);

	atom.m_uds = KIO::UDSEntry::UDS_SIZE;
	atom.m_long = vfile_getSize();
	entry.append(atom);

	atom.m_uds = KIO::UDSEntry::UDS_MODIFICATION_TIME;
	atom.m_long = vfile_getTime_t();
	entry.append(atom);

	atom.m_uds = KIO::UDSEntry::UDS_USER;
	atom.m_str = vfile_getOwner();
	entry.append(atom);

	atom.m_uds = KIO::UDSEntry::UDS_GROUP;
	atom.m_str = vfile_getGroup(); 
	entry.append(atom);

	atom.m_uds = KIO::UDSEntry::UDS_MIME_TYPE;
	atom.m_str = vfile_getMime();
	entry.append(atom);

	atom.m_uds = KIO::UDSEntry::UDS_FILE_TYPE;
	atom.m_long = vfile_getMode() & S_IFMT;
	entry.append(atom);

	atom.m_uds = KIO::UDSEntry::UDS_ACCESS;
	atom.m_long = vfile_getMode() & 07777; // keep permissions only
	entry.append( atom );

	atom.m_uds = KIO::UDSEntry::UDS_MIME_TYPE;
	atom.m_str = vfile_getMime();
	entry.append(atom);

	if( vfile_isSymLink() ){
		atom.m_uds = KIO::UDSEntry::UDS_LINK_DEST;
		atom.m_str = vfile_getSymDest();
		entry.append(atom);
	}

	if( !vfile_acl_loaded )
		vfile_loadACL();
	if( vfile_has_acl ) {
		atom.m_uds = KIO::UDSEntry::UDS_EXTENDED_ACL;
		atom.m_long = 1;
		entry.append( atom );
		
		if( !vfile_acl.isNull() )
		{
			atom.m_uds = KIO::UDSEntry::UDS_ACL_STRING;
			atom.m_str = vfile_acl;
			entry.append(atom);
		}
		
		if( !vfile_def_acl.isNull() )
		{
			atom.m_uds = KIO::UDSEntry::UDS_DEFAULT_ACL_STRING;
			atom.m_str = vfile_acl;
			entry.append(atom);
		}
	}

	return entry;
}

bool vfile::operator==(const vfile& vf) const{
	bool equal;
	
	if( !vfile_acl_loaded )
		const_cast<vfile *>( this )->vfile_loadACL();
	if( !vf.vfile_acl_loaded )
		const_cast<vfile *>( &vf )->vfile_loadACL();
	
	equal = (vfile_name     == vf.vfile_getName()   ) &&
	        (vfile_size     == vf.vfile_getSize()   ) &&
	        (vfile_perm     == vf.vfile_getPerm()   ) &&
	        (vfile_time_t   == vf.vfile_getTime_t() ) &&
	        (vfile_ownerId  == vf.vfile_getUid()    ) &&
	        (vfile_groupId  == vf.vfile_getGid()    ) &&
	        (vfile_has_acl  == vf.vfile_has_acl     ) &&
	        (!vfile_has_acl || 
	        (vfile_acl      == vf.vfile_acl         ) &&
	        (vfile_def_acl  == vf.vfile_def_acl     ) );;
	
	return equal;
}

vfile& vfile::operator= (const vfile& vf){
	vfile_name       = vf.vfile_name      ;
	vfile_size       = vf.vfile_size      ;
	vfile_mode       = vf.vfile_mode      ;
	vfile_ownerId    = vf.vfile_ownerId   ;
	vfile_groupId    = vf.vfile_groupId   ;
	vfile_owner      = vf.vfile_owner     ;
	vfile_group      = vf.vfile_group     ;
	vfile_userName   = vf.vfile_userName  ;
	vfile_perm       = vf.vfile_perm      ;
	vfile_time_t     = vf.vfile_time_t    ;
	vfile_symLink    = vf.vfile_symLink   ;
	vfile_mimeType   = vf.vfile_mimeType  ;
	vfile_symDest    = vf.vfile_symDest   ;
	vfile_url        = vf.vfile_url       ;
	vfile_isdir      = vf.vfile_isdir     ;
	vfile_has_acl    = vf.vfile_has_acl   ;
	vfile_acl        = vf.vfile_acl       ;
	vfile_def_acl    = vf.vfile_def_acl   ;
	vfile_rwx        = vf.vfile_rwx       ;
	vfile_acl_loaded = vf.vfile_acl_loaded;
	
	return (*this);
} 

#include "vfile.moc"
