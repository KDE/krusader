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

KrVfsHandler::KrVfsHandler(){
}
KrVfsHandler::~KrVfsHandler(){
}

vfs::VFS_TYPE KrVfsHandler::getVfsType(const KURL& url){
	if(url.isLocalFile() ) return vfs::NORMAL;
  else{
		if(url.protocol() == "virt") return vfs::VIRT;
		else return vfs::FTP;
	}
	return vfs::ERROR;
}

vfs* KrVfsHandler::getVfs(const KURL& url,vfs* oldVfs){
	vfs::VFS_TYPE newType,oldType = vfs::ERROR;
	if(oldVfs) oldType = oldVfs->vfs_getType();


	return 0;
}
