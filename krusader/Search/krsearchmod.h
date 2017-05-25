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
#include <QDateTime>
#include <QObject>
#include <QStack>
#include <QStringList>
#include <QUrl>

#include <KIO/Global>

class KRQuery;
class FileItem;
class FileSystem;
class DefaultFileSystem;
class VirtualFileSystem;

/**
 * Search for files based on a search query.
 *
 * Subdirectories are included if query->isRecursive() is true.
 */
class KRSearchMod : public QObject
{
    Q_OBJECT
public:
    explicit KRSearchMod(const KRQuery *query);
    ~KRSearchMod();

    void start();
    void stop();

private:
    void scanUrl(const QUrl &url);
    void scanDirectory(const QUrl &url);
    FileSystem *getFileSystem(const QUrl &url);

signals:
    void searching(const QString &url);
    void found(const FileItem &file, const QString &foundText);
    // NOTE: unused
    void error(const QUrl &url);
    void finished();

private slots:
    void slotProcessEvents(bool &stopped);

private:
    KRQuery *m_query;
    DefaultFileSystem *m_defaultFileSystem;
    VirtualFileSystem *m_virtualFileSystem;

    bool m_stopSearch;

    QStack<QUrl> m_scannedUrls;
    QStack<QUrl> m_unScannedUrls;
    QTime m_timer;
};

#endif
