/***************************************************************************
                          krvfshandler.cpp  -  description
                             -------------------
    begin                : Fri Dec 5 2003
    copyright            : (C) 2003 by Shie Erlich & Rafi Yanai
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "krvfshandler.h"
#include "normal_vfs.h"
#include "ftp_vfs.h"
#include "virt_vfs.h"
#include "../krservices.h"

#include <QtCore/QDir>

#include <kdebug.h>


KrVfsHandler::KrVfsHandler(){
}
KrVfsHandler::~KrVfsHandler(){
}

vfs::VFS_TYPE KrVfsHandler::getVfsType(const KUrl& url){
  QString protocol = url.protocol();

  if( ( protocol == "krarc" || protocol == "tar" || protocol == "zip" ) &&
      QDir(KrServices::getPath(url, KUrl::RemoveTrailingSlash)).exists() )
    return vfs::VFS_NORMAL;
  
	if( url.isLocalFile() ){
		return vfs::VFS_NORMAL;
	}
  else{
		if(url.protocol() == "virt") return vfs::VFS_VIRT;
		else return vfs::VFS_FTP;
	}
	return vfs::VFS_ERROR;
}

vfs* KrVfsHandler::getVfs(const KUrl& url,QObject* parent,vfs* oldVfs){
	vfs::VFS_TYPE newType,oldType = vfs::VFS_ERROR;

	if(oldVfs) oldType = oldVfs->vfs_getType();
	newType = getVfsType(url);
	

	vfs* newVfs = oldVfs;

	if( oldType != newType ){
		switch( newType ){
		  case (vfs::VFS_NORMAL) : newVfs = new normal_vfs(parent); break;
		  case (vfs::VFS_FTP   ) : newVfs = new ftp_vfs(parent)   ; break;
		  case (vfs::VFS_VIRT  ) : newVfs = new virt_vfs(parent)  ; break;
      case (vfs::VFS_ERROR ) : newVfs = 0                     ; break;
		}
  }

	return newVfs;
}
