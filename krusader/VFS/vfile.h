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
#include <kio/udsentry.h>
#include <kmimetype.h>

#define PERM_ALL          -2

/**
 * The Virtual File class handles all the details of maintaining a single
 * file component within the virtual file system (vfs). a vfile object
 * contains the nessecery details about a file and member functions which
 *  allow the object to give out the needed details about the file.
 */
class vfile : public QObject{
  Q_OBJECT
  
public:
	vfile(){}

  /**
	 * Use this constructor when you know the following files properties: \n
	 * file name, file size, file permissions,is the file a link,owner uid & group uid.
	 */
	vfile(const QString& name,
	      const KIO::filesize_t size,
	      const QString& perm,
	      const time_t mtime,
	      const bool symLink,
	      const uid_t	owner,
	      const gid_t group,
	      const QString& mime,
	      const QString& symDest,
	      const mode_t  mode,
	      const int rwx = -1 );
	
	vfile(const QString& name,	
	      const KIO::filesize_t size,	
	      const QString& perm,
	      const time_t mtime,
	      const bool symLink,
	      const QString& owner,
	      const QString& group,
	      const QString& userName,
	      const QString& mime,
	      const QString& symDest,
	      const mode_t  mode,
	      const int rwx = -1,
	      const QString& aclString = QString(),
	      const QString& aclDfltString = QString() );
	
	bool        operator==(const vfile& vf) const;
	vfile&      operator= (const vfile& vf); 
	inline bool operator!=(const vfile& vf){ return !((*this)==vf); }
	
	// following functions give-out file details
	inline const QString&   vfile_getName()    const { return vfile_name;           }
	inline KIO::filesize_t  vfile_getSize()    const { return vfile_size;           }
	inline const QString&   vfile_getPerm()    const { return vfile_perm;           }
	inline bool             vfile_isDir()      const { return vfile_isdir;          }
	inline bool             vfile_isSymLink()  const { return vfile_symLink;        }
	inline const QString&   vfile_getSymDest() const { return vfile_symDest;        }
	inline mode_t           vfile_getMode()    const { return vfile_mode;           }
	inline uid_t            vfile_getUid()     const { return vfile_ownerId;        }
	inline gid_t            vfile_getGid()     const { return vfile_groupId;        }
	inline time_t           vfile_getTime_t()  const { return vfile_time_t;         }
	inline const KUrl&      vfile_getUrl()     const { return vfile_url;            }
  
	const QString&          vfile_getMime(bool fast=false);
	const QString&          vfile_getOwner();
	const QString&          vfile_getGroup();
	const QString&          vfile_getACL();
	const QString&          vfile_getDefaultACL();
	const KIO::UDSEntry     vfile_getEntry(); //< return the UDSEntry from the vfile
	char                    vfile_isReadable()   const;
	char                    vfile_isWriteable()  const;
	char                    vfile_isExecutable() const;
	/**
	 * Set the file size.
	 * used ONLY when calculating a directory's space, needs to change the
	 * displayed size of the viewitem and thus the vfile. For INTERNAL USE !
	 */
	inline void             vfile_setSize(KIO::filesize_t size) {vfile_size = size;}
	inline void             vfile_setUrl(const KUrl& url)       {vfile_url = url;  }

	inline void             vfile_setIcon(const QString& icn)   {vfile_icon = icn; }
	inline QString          vfile_getIcon();

	virtual ~vfile(){}

private:
	void                    vfile_loadACL();

protected:
	// the file information list
	QString          vfile_name;     //< file name
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
	KUrl             vfile_url;      //< file URL - empty by default
	QString          vfile_icon;     //< the name of the icon file
	bool             vfile_isdir;    //< flag, if it's a directory
	int              vfile_rwx;      //< flag, showing read, write, execute properties
	bool             vfile_acl_loaded;//<flag, indicates that ACL permissions already loaded
	bool             vfile_has_acl;  //< flag, indicates ACL permissions
	QString          vfile_acl;      //< ACL permission string
	QString          vfile_def_acl;  //< ACL default string
};

	
QString vfile::vfile_getIcon(){
	if( vfile_icon.isEmpty() ){
		QString mime = this->vfile_getMime();
      if ( mime == "Broken Link !" )
         vfile_icon = "file_broken";
      else {
         vfile_icon = KMimeType::mimeType( mime ) ->iconName();
      }		
	}
	return vfile_icon;
}

#endif
