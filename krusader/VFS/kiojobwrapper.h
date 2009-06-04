/***************************************************************************
                               kiojobwrapper.h
                             -------------------
    copyright            : (C) 2008+ by Csaba Karai
    email                : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
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

#ifndef KIOJOBWRAPPER_H
#define KIOJOBWRAPPER_H

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QMap>

#include <kurl.h>
#include <kio/jobclasses.h>

class QEvent;
class vfs;

enum KIOJobWrapperType {
	Stat = 1,
	DirectorySize = 2,
	Copy = 3,
	Move = 4,
	VirtualCopy = 5,
	VirtualMove = 6,
	Pack = 7,
	Unpack = 8
};

class KIOJobWrapper : public QObject {
	Q_OBJECT
	friend class KrJobStarter;
	friend class JobStartEvent;
private:
	KIOJobWrapperType         m_type;
	KUrl                      m_url;
	KUrl::List                m_urlList;
	bool                      m_showProgress;
	int                       m_pmode;
	void *                    m_userData;
	bool                      m_autoErrorHandling;
	
	QMap<QString,QString>     m_archiveProperties;
	QStringList               m_archiveFileNames;
	QString                   m_archiveType;
	KUrl                      m_archiveSourceBase;
	
	QList<const char *>       m_signals;
	QList<QPointer<QObject> > m_receivers;
	QList<const char *>       m_methods;
	
	QPointer<KIO::Job>        m_job;
	
	bool                      m_delete;
	bool                      m_started;
	bool                      m_suspended;
	
	KIOJobWrapper( KIOJobWrapperType type, const KUrl &url );
	KIOJobWrapper( KIOJobWrapperType type, const KUrl &url, void * userData );
	KIOJobWrapper( KIOJobWrapperType type, const KUrl &url, const KUrl::List &list, int pmode, bool showp );
	KIOJobWrapper( KIOJobWrapperType type, const KUrl &url, const KUrl &dest, const QStringList &names,
	               bool showp, const QString &atype, const QMap<QString,QString> &packProps );
	void createJob();
	
public:
	virtual ~KIOJobWrapper();
	
	void start();
	
	void suspend();
	void resume();
	void abort();
	
	void connectTo( const char * signal, const QObject * receiver, const char * method );
	void setAutoErrorHandlingEnabled( bool err ) { m_autoErrorHandling = err; }
	bool isStarted()             { return m_started; }
	bool isSuspended()           { return m_suspended; }
	
	KIO::Job *        job()      { return m_job; }
	KIOJobWrapperType type()     { return m_type; }
	QString           typeStr();
	KUrl              url()      { return m_url; }
	KUrl::List        urlList()  { return m_urlList; }
	QString           toolTip();
	
	static KIOJobWrapper * stat( KUrl &url );
	static KIOJobWrapper * directorySize( KUrl &url );
	static KIOJobWrapper * copy( int pmode, KUrl::List &list, KUrl &url, bool showProgress );
	static KIOJobWrapper * move( int pmode, KUrl::List &list, KUrl &url, bool showProgress );
	static KIOJobWrapper * virtualCopy( const QStringList *names, vfs * vfs, KUrl& dest,
	                                    const KUrl& baseURL, int pmode, bool showProgressInfo );
	static KIOJobWrapper * virtualMove( const QStringList *names, vfs * vfs, KUrl& dest,
	                                    const KUrl& baseURL, int pmode, bool showProgressInfo );
	static KIOJobWrapper * pack( const KUrl &srcUrl, const KUrl &destUrl, const QStringList & fileNames,
	                                    const QString &type, const QMap<QString, QString> &packProps,
	                                    bool showProgressInfo );
	static KIOJobWrapper * unpack( const KUrl &srcUrl, const KUrl &destUrl, const QStringList & fileNames,
	                                    bool showProgressInfo );
};

class KrJobStarter : public QObject {
	Q_OBJECT
	friend class KIOJobWrapper;
public:
	KrJobStarter() { m_self = this;}
protected:
	bool event( QEvent * e );
	
	static KrJobStarter * self() { return m_self; }
	static KrJobStarter * m_self;
};

#endif // __KIO_JOB_WRAPPER__
