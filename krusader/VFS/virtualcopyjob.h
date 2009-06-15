/***************************************************************************
                       virtualcopyjob.h  -  description
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

#ifndef VIRTUALCOPYJOB_H
#define VIRTUALCOPYJOB_H

#include <QtCore/QList>
#include <QtCore/QTimer>
#include <QtCore/QHash>
#include <qmap.h>

#include <kio/job.h>

#include "preservingcopyjob.h"

class vfs;

typedef enum {
    ST_STARTING               = 0,
    ST_CALCULATING_TOTAL_SIZE = 1,
    ST_CREATING_DIRECTORY     = 2,
    ST_COPYING                = 3
} State;

class VirtualCopyJob : public KIO::Job
{
    Q_OBJECT

public:
    VirtualCopyJob(const QStringList *names, vfs * vfs, const KUrl& dest, const KUrl& baseURL,
                   PreserveMode pmode, KIO::CopyJob::CopyMode mode, bool showProgressInfo,
                   bool autoStart = true);
    virtual ~VirtualCopyJob();

    inline bool isSkipAll()       {
        return m_skipAll;
    }
    inline void setSkipAll()      {
        m_skipAll = true;
    }
    inline bool isOverwriteAll()  {
        return m_overwriteAll;
    }
    inline void setOverwriteAll() {
        m_overwriteAll = true;
    }
    inline bool isMulti()         {
        return m_multi;
    }

protected:
    void statNextDir();
    void createNextDir();
    void copyCurrentDir();
    void directoryFinished(const QString &);

public slots:
    void slotStart();

protected slots:
    void slotReport();

    void slotKdsResult(KJob *);
    void slotStatResult(KJob *);
    void slotMkdirResult(KJob *);
    void slotCopyResult(KJob *);

    void slotCopying(KIO::Job *, const KUrl &, const KUrl &);
    void slotMoving(KIO::Job *, const KUrl &, const KUrl &);

    void slotProcessedFiles(KIO::Job *, unsigned long);
    void slotProcessedSize(KJob *, qulonglong);

signals:
    void totalFiles(KIO::Job *job, unsigned long files);
    void totalDirs(KIO::Job *job, unsigned long dirs);
    void processedFiles(KIO::Job *job, unsigned long files);
    void processedDirs(KIO::Job *job, unsigned long dirs);

private:
    bool                     m_overwriteAll;
    bool                     m_skipAll;
    bool                     m_multi;

    KIO::filesize_t          m_totalSize;
    unsigned long            m_totalFiles;
    unsigned long            m_totalSubdirs;

    qulonglong               m_processedSize;
    unsigned long            m_processedFiles;
    unsigned long            m_processedSubdirs;

    qulonglong               m_tempSize;
    unsigned long            m_tempFiles;

    QList<KUrl>              m_dirsToGetSize;

    QHash<QString, KUrl::List *> m_filesToCopy;

    QMap<QString, qulonglong> m_size;
    QMap<QString, int>        m_filenum;
    QMap<QString, int>        m_subdirs;

    KUrl                     m_baseURL;
    KUrl                     m_dest;
    PreserveMode             m_pmode;
    KIO::CopyJob::CopyMode   m_mode;
    bool                     m_showProgressInfo;

    State                    m_state;

    QTimer                   m_reportTimer;

    KUrl                     m_current;
    QString                  m_currentDir;

    QStringList              m_dirStack;
};

#endif /* __VIRTUAL_COPY_JOB_H__ */
