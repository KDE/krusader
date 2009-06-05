/***************************************************************************
                        threadedjob.h  -  description
                             -------------------
    copyright            : (C) 2009 + by Csaba Karai
    e-mail               : krusader@users.sourceforge.net
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

#ifndef ABSTRACTTHREADEDJOB_H
#define ABSTRACTTHREADEDJOB_H

#include <QThread>
#include <QEvent>
#include <QMutex>
#include <QWaitCondition>
#include <QStack>
#include <QVariant>
#include <QList>
#include <QEventLoop>
#include <QTime>

#include <kurl.h>
#include <kio/jobclasses.h>

class AbstractJobThread;
class KTempDir;
class UserEvent;
class KRarcObserver;
class KTemporaryFile;

class AbstractThreadedJob : public KIO::Job
{
friend class AbstractJobThread;

Q_OBJECT

protected:
  AbstractThreadedJob();

  void addEventResponse( QList<QVariant> * obj );
  QList<QVariant> * getEventResponse( UserEvent * event );
  void  sendEvent( UserEvent * event );

  virtual ~AbstractThreadedJob();
  virtual bool event(QEvent *);
  virtual void start( AbstractJobThread* );

protected slots:
  void slotDownloadResult( KJob* );
  void slotProcessedAmount( KJob *, KJob::Unit, qulonglong );
  void slotTotalAmount( KJob *, KJob::Unit, qulonglong );
  void slotSpeed( KJob *, unsigned long );
  void slotDescription( KJob *job, const QString &title, const QPair<QString, QString> &field1,
                        const QPair<QString, QString> &field2 );

public:
  QMutex                    _locker;
  QWaitCondition            _waiter;
  QStack<QList<QVariant> *> _stack;
  QString                   _title;
  qulonglong                _maxProgressValue;
  qulonglong                _currentProgress;
  QTime                     _time;
  bool                      _exiting;

private:
  AbstractJobThread *       _jobThread;
};



class AbstractJobThread : public QThread
{
friend class AbstractThreadedJob;
friend class AbstractJobObserver;
Q_OBJECT

public:
  AbstractJobThread();
  virtual ~AbstractJobThread();

  void abort();
  KRarcObserver * observer();

protected slots:
  virtual void slotStart() = 0;

protected:
  virtual void run();
  void setJob( AbstractThreadedJob * job ) { _job = job; }

  KUrl::List remoteUrls( const KUrl &baseUrl, const QStringList & files );
  KUrl downloadIfRemote( const KUrl &baseUrl, const QStringList & files );
  void calcSpaceLocal( const KUrl &baseUrl, const QStringList & files, KIO::filesize_t &totalSize, 
                       unsigned long &totalDirs, unsigned long &totalFiles );

  void sendError( int errorCode, QString message );
  void sendInfo( QString message, QString a1 = QString(), QString a2 = QString(), QString a3 = QString(), QString a4 = QString() );
  void sendReset( QString message, QString a1 = QString(""), QString a2 = QString(""), QString a3 = QString(""), QString a4 = QString("") );
  void sendSuccess();
  void sendMessage( const QString &message );
  void sendMaxProgressValue( qulonglong value );
  void sendAddProgress( qulonglong value, const QString &progress = QString() );

  void setProgressTitle( const QString &title ) { _progressTitle = title; }

  QString tempFileIfRemote( const KUrl &kurl, const QString &type );
  QString tempDirIfRemote( const KUrl &kurl );
  bool uploadTempFiles();

  bool isExited() { return _exited; }
  void terminate();

  QString getPassword( const QString &path );

  AbstractThreadedJob *_job;
  QEventLoop          *_loop;

  KTempDir            *_downloadTempDir;
  KRarcObserver       *_observer;

  KTemporaryFile      *_tempFile;
  QString              _tempFileName;
  KUrl                 _tempFileTarget;
  KTempDir            *_tempDir;
  QString              _tempDirName;
  KUrl                 _tempDirTarget;

  bool                 _exited;

  QString              _progressTitle;
};

enum PossibleCommands
{
  CMD_ERROR                =  1,
  CMD_INFO                 =  2,
  CMD_RESET                =  3,
  CMD_DOWNLOAD_FILES       =  4,
  CMD_UPLOAD_FILES         =  5,
  CMD_SUCCESS              =  6,
  CMD_MAXPROGRESSVALUE     =  7,
  CMD_ADD_PROGRESS         =  8,
  CMD_GET_PASSWORD         =  9,
  CMD_MESSAGE              = 10
};

class UserEvent : public QEvent
{
public:
  UserEvent( int command, const QList<QVariant> &args ) : QEvent( QEvent::User ), _command( command ), _args( args ) {}

  inline int                     command() { return _command; }
  inline const QList<QVariant> & args()    { return _args; }

protected:
  int             _command;
  QList<QVariant> _args;
};

#endif // __ABSTRACTTHREADED_JOB_H__

