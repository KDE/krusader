/***************************************************************************
                      synchronizertask.h  -  description
                             -------------------
    copyright            : (C) 2006 + by Csaba Karai
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

#ifndef SYNCHRONIZERTASK_H
#define SYNCHRONIZERTASK_H

#include <QtCore/QObject>

#include <kio/job.h>

class Synchronizer;
class SynchronizerDirList;
class SynchronizerFileItem;
class QTimer;
class QFile;

#define ST_STATE_NEW      0
#define ST_STATE_PENDING  1
#define ST_STATE_STATUS   2
#define ST_STATE_READY    3
#define ST_STATE_ERROR    4

class SynchronizerTask : public QObject {
  Q_OBJECT

public:
  SynchronizerTask() : QObject(), m_state( ST_STATE_NEW ), m_statusMessage( QString() ) {}
  virtual ~SynchronizerTask() {};

  inline int start( QWidget *parentWidget ) { this->parentWidget = parentWidget; start(); return state(); }

  inline int  state() { return m_state; }

  void setStatusMessage( const QString & statMsg ) { 
    if( m_state == ST_STATE_PENDING || m_state == ST_STATE_STATUS )
      m_state = ST_STATE_STATUS;
      m_statusMessage = statMsg;
  }

  QString status() {
    if( m_state == ST_STATE_STATUS ) {
      m_state = ST_STATE_PENDING;
      return m_statusMessage; 
    }
    return QString();
  }

protected:
  virtual void start() {};
  int m_state;
  QString m_statusMessage;
  QWidget *parentWidget;
};


class CompareTask : public SynchronizerTask {
  Q_OBJECT

public:
  CompareTask( SynchronizerFileItem *parentIn, const QString &leftURL,
                const QString &rightURL, const QString &leftDir,
                const QString &rightDir, bool ignoreHidden );
  CompareTask( SynchronizerFileItem *parentIn, const QString &urlIn,
                const QString &dirIn, bool isLeftIn, bool ignoreHidden );
  virtual ~CompareTask();

  inline bool isDuplicate()                      { return m_duplicate; }
  inline bool isLeft()                           { return !m_duplicate && m_isLeft; }
  inline const QString & leftURL()               { return m_url; }
  inline const QString & rightURL()              { return m_otherUrl; }
  inline const QString & leftDir()               { return m_dir; }
  inline const QString & rightDir()              { return m_otherDir; }
  inline const QString & url()                   { return m_url; }
  inline const QString & dir()                   { return m_dir; }
  inline SynchronizerFileItem * parent()         { return m_parent; }
  inline SynchronizerDirList * leftDirList()     { return m_dirList; }
  inline SynchronizerDirList * rightDirList()    { return m_otherDirList; }
  inline SynchronizerDirList * dirList()         { return m_dirList; }

protected slots:
  virtual void start();
  void slotFinished( bool result );
  void slotOtherFinished( bool result );

private:
  SynchronizerFileItem * m_parent;
  QString m_url;
  QString m_dir;
  QString m_otherUrl;
  QString m_otherDir;
  bool m_isLeft;
  bool m_duplicate;
  SynchronizerDirList * m_dirList;
  SynchronizerDirList * m_otherDirList;
  bool m_loadFinished;
  bool m_otherLoadFinished;
  bool ignoreHidden;
};


class CompareContentTask : public SynchronizerTask {
  Q_OBJECT

public:
  CompareContentTask( Synchronizer *, SynchronizerFileItem *, const KUrl &, const KUrl &, KIO::filesize_t );
  virtual ~CompareContentTask();

public slots:
  void    slotDataReceived(KIO::Job *job, const QByteArray &data);
  void    slotFinished(KJob *job);
  void    sendStatusMessage();

protected:
  virtual void start();

protected slots:
  void    localFileCompareCycle();

private:
  void    abortContentComparing();

  KUrl                   leftURL;        // the currently processed URL (left)
  KUrl                   rightURL;       // the currently processed URL (right)
  KIO::filesize_t        size;           // the size of the compared files

  bool                   errorPrinted;   // flag indicates error
  KIO::TransferJob      *leftReadJob;    // compare left read job
  KIO::TransferJob      *rightReadJob;   // compare right read job
  QByteArray             compareArray;   // the array for comparing
  int                    owner;          // the owner of the compare array
  SynchronizerFileItem  *item;           // the item for content compare
  QTimer                *timer;          // timer to show the process dialog at compare by content

  QFile                 *leftFile;       // the left side local file
  QFile                 *rightFile;      // the right side local file

  KIO::filesize_t        received;       // the received size
  Synchronizer          *sync;
};

#endif /* __SYNCHRONIZER_TASK_H__ */
