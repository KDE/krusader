/***************************************************************************
                          vfile.h
                      -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
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
#ifndef VFILE_H
#define VFILE_H

// QT includes
#include <qstring.h>
#include <qobject.h>
// System includes
#include <sys/types.h>
// KDE includes
#include <kio/global.h>

/**
 * The Virtual File class handles all the details of maintaining a single
 * file component within the virtual file system (vfs). a vfile object
 * contains the nessecery details about a file and member functions which
 *  allow the object to give out the needed details about the file.
 */
class vfile : public QObject{
  Q_OBJECT
  
public:
  /**
	 * Use this constructor when you know the following files properties: \n
	 * file name, file size, file permissions,is the file a link,owner uid & group uid.
	 */
	vfile(QString name,	
				KIO::filesize_t size,
			  QString perm,
				time_t mtime,
				bool symLink,
				uid_t	owner,
				gid_t group,
				QString mime,
				QString symDest,
				mode_t  mode);
	
	vfile(QString name,	
				KIO::filesize_t size,	
			  QString perm,
				time_t mtime,
				bool symLink,
				QString	owner,
				QString group,
				QString userName,
				QString mime,
				QString symDest,
				mode_t  mode);
	
	// following functions give-out file details
	inline QString  			 	vfile_getName() const { return vfile_name;  		}
	inline KIO::filesize_t  vfile_getSize() 	 	{ return vfile_size;  		}
	inline QString					vfile_getPerm()			{ return vfile_perm;			}
	inline bool							vfile_isDir()				{ return (vfile_perm[0]=='d');}
	inline bool							vfile_isSymLink()		{ return vfile_symLink;   }
	inline QString					vfile_getMime()			{ return vfile_mimeType;	}
	inline QString					vfile_getSymDest()	{ return vfile_symDest;		}
	inline mode_t						vfile_getMode()			{ return vfile_mode;			}
	inline uid_t						vfile_getUid()			{ return vfile_ownerId;		}
  inline gid_t						vfile_getGid()			{ return vfile_groupId;		}
  inline time_t           vfile_getTime_t()   { return vfile_time_t;    }
  inline KURL             vfile_getUrl()      { return vfile_url;       }
  
	QString						      vfile_getOwner();
  QString						      vfile_getGroup();
  char			              vfile_isReadable();
  char 			              vfile_isWriteable();
  char			              vfile_isExecutable();
	KIO::UDSEntry           vfile_getEntry(); //< return the UDSEntry from the vfile
  /**
	 * Set the file size.
	 * used ONLY when calculating a directory's space, needs to change the
	 * displayed size of the viewitem and thus the vfile. For INTERNAL USE !
	 */
	inline void             vfile_setSize(KIO::filesize_t size) {vfile_size = size;}
	inline void             vfile_setUrl(const KURL& url){ vfile_url = url; }
  virtual ~vfile(){}

protected:
  // the file information list
  QString 	       vfile_name;     //< file name
  KIO::filesize_t  vfile_size;     //< file size
  mode_t           vfile_mode;     //< file mode
	uid_t            vfile_ownerId;  //< file owner id
	gid_t            vfile_groupId;  //< file group id
  QString          vfile_owner;    //< file owner name
  QString          vfile_group;    //< file group name
  QString          vfile_userName; //< the current username
	QString          vfile_perm;     //< file permissions string
  time_t           vfile_time_t;   //< file modification in time_t format
	bool             vfile_symLink;  //< true if the file is a symlink
	QString          vfile_mimeType; //< file mimetype
	QString          vfile_symDest;  //< if it's a sym link - its detination
	KURL             vfile_url;      //< file URL - empty by default
};

#endif
