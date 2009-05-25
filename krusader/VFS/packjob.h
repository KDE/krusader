/***************************************************************************
                          packjob.h  -  description
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

#ifndef __PACK_JOB_H__
#define __PACK_JOB_H__

#include "abstractthreadedjob.h"
#include <qmap.h>

class PackThread;
class TestArchiveThread;
class UnpackThread;

class PackJob : public AbstractThreadedJob
{
Q_OBJECT

private:
  PackJob( const KUrl &srcUrl, const KUrl &destUrl, const QStringList & fileNames, const QString &type, const QMap<QString, QString> &packProps );

public:
  static PackJob * createPacker( const KUrl &srcUrl, const KUrl &destUrl, const QStringList & fileNames, const QString &type, const QMap<QString, QString> &packProps );
};


class PackThread : public AbstractJobThread
{
Q_OBJECT

public:
  PackThread( const KUrl &srcUrl, const KUrl &destUrl, const QStringList & fileNames, const QString &type, const QMap<QString, QString> &packProps );
  virtual ~PackThread() {}

protected slots:
  virtual void slotStart();

private:
  KUrl                   _sourceUrl;
  KUrl                   _destUrl;
  QStringList            _fileNames;
  QString                _type;
  QMap<QString, QString> _packProperties;
};


class TestArchiveJob : public AbstractThreadedJob
{
Q_OBJECT

private:
  TestArchiveJob( const KUrl &srcUrl, const QStringList & fileNames );

public:
  static TestArchiveJob * testArchives( const KUrl &srcUrl, const QStringList & fileNames );
};


class TestArchiveThread : public AbstractJobThread
{
Q_OBJECT

public:
  TestArchiveThread( const KUrl &srcUrl, const QStringList & fileNames );
  virtual ~TestArchiveThread() {}

protected slots:
  virtual void slotStart();

private:
  KUrl                   _sourceUrl;
  QStringList            _fileNames;
};



class UnpackJob : public AbstractThreadedJob
{
Q_OBJECT

private:
  UnpackJob( const KUrl &srcUrl, const KUrl &destUrl, const QStringList & fileNames );

public:
  static UnpackJob * createUnpacker( const KUrl &srcUrl, const KUrl &destUrl, const QStringList & fileNames );
};


class UnpackThread : public AbstractJobThread
{
Q_OBJECT

public:
  UnpackThread( const KUrl &srcUrl, const KUrl &destUrl, const QStringList & fileNames );
  virtual ~UnpackThread() {}

protected slots:
  virtual void slotStart();

private:
  KUrl                   _sourceUrl;
  KUrl                   _destUrl;
  QStringList            _fileNames;
};

#endif // __PACK_JOB_H__

