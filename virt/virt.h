/***************************************************************************
                              virt.h
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

#ifndef _VIRT_H
#define _VIRT_H

#include <sys/types.h>
#include <q3dict.h>
#include <QByteArray>
#include <kconfig.h>
#include <kio/slavebase.h>

class VirtProtocol : public KIO::SlaveBase {
public:
	VirtProtocol( const QByteArray &pool, const QByteArray &app );
	virtual ~VirtProtocol();

	virtual void listDir ( const KUrl & url );
	virtual void stat    ( const KUrl & url );
	virtual void get    ( const KUrl & url );
	virtual void mkdir(const KUrl& url,int permissions);
	virtual void copy( const KUrl &src, const KUrl &dest, int permissions, bool overwrite );
	virtual void del    (KUrl const & url, bool isFile);

protected:
	bool lock();
	bool unlock();
	bool save();
	bool load();

	void local_entry(const KUrl& url,KIO::UDSEntry& entry);
	bool addDir(QString& path);


	static Q3Dict<KUrl::List> kioVirtDict;
	static KConfig* kio_virt_db;

	bool rewriteURL(const KUrl&, KUrl&);

};

#endif
