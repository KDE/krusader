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
#include "temp_vfs.h"
#include "ftp_vfs.h"

#include <qdir.h>

#include <kdebug.h>


KrVfsHandler::KrVfsHandler(){
}
KrVfsHandler::~KrVfsHandler(){
}

vfs::VFS_TYPE KrVfsHandler::getVfsType(const KURL& url){
  QString protocol = url.protocol();

  if( ( protocol == "krarc" || protocol == "tar" || protocol == "zip" ) &&
      QDir(url.path(-1)).exists() )
    return vfs::NORMAL;
  
	if( url.isLocalFile() ){
		return vfs::NORMAL;
	}
  else{
		if(url.protocol() == "virt") return vfs::VIRT;
		else return vfs::FTP;
	}
	return vfs::ERROR;
}

vfs* KrVfsHandler::getVfs(const KURL& url,QObject* parent,vfs* oldVfs){
	vfs::VFS_TYPE newType,oldType = vfs::ERROR;

	if(oldVfs) oldType = oldVfs->vfs_getType();
	newType = getVfsType(url);
	

	vfs* newVfs = oldVfs;

	if( oldType != newType ){
		switch( newType ){
		  case (vfs::NORMAL) : newVfs = new normal_vfs(parent); break;
		  case (vfs::FTP   ) : newVfs = new ftp_vfs(parent)   ; break;
//		  case (vfs::TEMP  ) : newVfs = new temp_vfs(parent)  ; break;
      case (vfs::ERROR ) : newVfs = 0                     ; break;
		}
  }

	return newVfs;
}
