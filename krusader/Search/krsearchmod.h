/***************************************************************************
                                 krsearchmod.h
                             -------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
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



#ifndef KRSEARCHMOD_H
#define KRSEARCHMOD_H

// QtCore
#include <QObject>
#include <QStringList>
#include <QDateTime>
#include <QStack>
#include <QUrl>

#include <KIO/Global>

#include "../FileSystem/defaultfilesystem.h"
#include "../FileSystem/virtualfilesystem.h"


class KRQuery;
class ftp_fileSystem;

class KRSearchMod : public QObject
{
    Q_OBJECT
public:
    explicit KRSearchMod(const KRQuery *q);
    ~KRSearchMod();

    void scanURL(QUrl url);
    void start();
    void stop();

private:
    void scanLocalDir(const QUrl &url);
    void scanRemoteDir(QUrl url);

signals:
    void finished();
    void searching(const QString&);
    void found(const FileItem &file, const QString &foundText);

private slots:
    void slotProcessEvents(bool & stopped);

private:
    bool stopSearch;
    QStack<QUrl> scannedUrls;
    QStack<QUrl> unScannedUrls;
    KRQuery *query;
    QStringList results;

    DefaultFileSystem *remote_fileSystem;
    VirtualFileSystem *virtual_fileSystem;

    QTime timer;
};

#endif
