/***************************************************************************
                          virt_vfs.h  -  description
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

#ifndef VIRT_VFS_H
#define VIRT_VFS_H

#include <vfs.h>

/**
  *@author Shie Erlich & Rafi Yanai
  */

class virt_vfs : public vfs  {
public: 
	virt_vfs(QObject* panel, bool quiet=false);
	~virt_vfs();
};

#endif
