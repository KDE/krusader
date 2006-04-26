/***************************************************************************
                        synchronizer.h  -  description
                             -------------------
    copyright            : (C) 2003 + by Csaba Karai
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
 
#ifndef __SYNCHRONIZER_H__
#define __SYNCHRONIZER_H__

#include "synchronizertask.h"
#include "synchronizerfileitem.h"
#include <qobject.h>
#include <qptrvector.h>
#include <qmap.h>
#include <qcolor.h>
#include <kio/job.h>

class KRQuery;
class vfile;

#define DECLARE_COLOR_NAME_ARRAY  QString COLOR_NAMES[] = { "Equals", "Differs", "LeftCopy", "RightCopy", "Delete", \
                                                            "Equals", "Equals",  "Equals",   "Equals",    "Equals" }
#define DECLARE_BACKGROUND_DFLTS  QColor BCKG_DFLTS[] = { QColor(), QColor(), QColor(), QColor(), Qt::red, \
                                                          QColor(), QColor(), QColor(), QColor(), QColor() }
#define DECLARE_FOREGROUND_DFLTS  QColor FORE_DFLTS[] = { Qt::black, Qt::red,  Qt::blue, Qt::darkGreen, Qt::white, \
                                                          Qt::black, Qt::black, Qt::black, Qt::black, Qt::black }

class Synchronizer : public QObject
{
  Q_OBJECT

  private:
    int     displayUpdateCount;   // the display is refreshed after every x-th change
  
  public:
    Synchronizer();
    int     compare( QString leftURL, QString rightURL, KRQuery *query, bool subDirs, bool symLinks,
                     bool igDate, bool asymm, bool cmpByCnt, bool igCase, bool autoSc, QStringList &selFiles,
                     int equThres, int timeOffs, int parThreads );
    void    stop() {stopped = true;}
    void    setMarkFlags( bool left, bool equal, bool differs, bool right, bool dup, bool sing, bool del );
    int     refresh( bool nostatus=false );
    bool    totalSizes( int *, KIO::filesize_t *, int *, KIO::filesize_t *, int *, KIO::filesize_t * );
    void    synchronize( QWidget *,bool leftCopyEnabled, bool rightCopyEnabled, bool deleteEnabled,
                         bool overWrite, int parThreads );
    void    synchronizeWithKGet();
    void    setScrolling( bool scroll );
    void    pause();
    void    resume();
    void    swapSides();
    void    reset();

    void    exclude( SynchronizerFileItem * );
    void    restore( SynchronizerFileItem * );
    void    reverseDirection( SynchronizerFileItem * );
    void    copyToLeft( SynchronizerFileItem * );
    void    copyToRight( SynchronizerFileItem * );
    void    deleteLeft( SynchronizerFileItem * );
    
    QString leftBaseDirectory();
    QString rightBaseDirectory();
    static QString getTaskTypeName( TaskType taskType );

    SynchronizerFileItem *getItemAt( unsigned ndx ) {return resultList.at(ndx);}

    void     setParentWidget( QWidget * widget )    {parentWidget = widget;}
    void     compareContentResult( SynchronizerFileItem * item, bool result );

  signals:
    void    comparedFileData( SynchronizerFileItem * );
    void    markChanged( SynchronizerFileItem *, bool );
    void    synchronizationFinished();
    void    processedSizes( int, KIO::filesize_t, int, KIO::filesize_t, int, KIO::filesize_t );
    void    pauseAccepted();
    void    statusInfo( QString );

  public slots:
    void    slotTaskFinished(KIO::Job*);
    void    slotProcessedSize( KIO::Job * , KIO::filesize_t );
    
  private:
    bool                  isDir( const vfile * file );
    QString               readLink( const vfile * file );
    
    void    compareDirectory( SynchronizerFileItem *,SynchronizerDirList *, SynchronizerDirList *,
                              const QString &leftDir, const QString &rightDir );
    void    addSingleDirectory( SynchronizerFileItem *, SynchronizerDirList *, const QString &, bool );
    SynchronizerFileItem * addItem( SynchronizerFileItem *, const QString &, const QString &,
                                    const QString &, const QString &, bool, bool, KIO::filesize_t,
                                    KIO::filesize_t, time_t, time_t, const QString &, const QString &,
                                    const QString &, const QString &, const QString &, const QString &,
                                    TaskType, bool, bool);
    SynchronizerFileItem * addLeftOnlyItem( SynchronizerFileItem *, const QString &, const QString &,
                                            KIO::filesize_t, time_t, const QString &, const QString &,
                                            const QString &, bool isDir = false, bool isTemp = false );
    SynchronizerFileItem * addRightOnlyItem( SynchronizerFileItem *, const QString &, const QString &,
                                             KIO::filesize_t, time_t, const QString &,  const QString &,
                                             const QString &, bool isDir = false, bool isTemp = false  );
    SynchronizerFileItem * addDuplicateItem( SynchronizerFileItem *, const QString &, const QString &,
                                             const QString &, const QString &, KIO::filesize_t,
                                             KIO::filesize_t, time_t, time_t, const QString &,
                                             const QString &, const QString &, const QString &,
                                             const QString &, const QString &, bool isDir = false,
                                             bool isTemp = false  );
    bool    isMarked( TaskType task, bool dupl );
    bool    markParentDirectories( SynchronizerFileItem * );
    void    synchronizeLoop();
    SynchronizerFileItem * getNextTask();
    void    executeTask( SynchronizerFileItem * task );
    void    setPermanent( SynchronizerFileItem * );
    void    operate( SynchronizerFileItem *item, void (*)(SynchronizerFileItem *) );
    void    compareLoop();

    static void excludeOperation( SynchronizerFileItem *item );
    static void restoreOperation( SynchronizerFileItem *item );
    static void reverseDirectionOperation( SynchronizerFileItem *item );
    static void copyToLeftOperation( SynchronizerFileItem *item );
    static void copyToRightOperation( SynchronizerFileItem *item );
    static void deleteLeftOperation( SynchronizerFileItem *item );
    
  protected:
    bool                              recurseSubDirs; // walk through subdirectories also
    bool                              followSymLinks; // follow the symbolic links
    bool                              ignoreDate;     // don't use date info at comparing
    bool                              asymmetric;     // asymmetric directory update
    bool                              cmpByContent;   // compare the files by content
    bool                              ignoreCase;     // case insensitive synchronization for Windows fs
    bool                              autoScroll;     // automatic update of the directory
    QPtrList<SynchronizerFileItem>    resultList;     // the found files
    QPtrList<SynchronizerFileItem>    temporaryList;  // temporary files
    QString                           leftBaseDir;    // the left-side base directory
    QString                           rightBaseDir;   // the right-side base directory
    QStringList                       excludedPaths;  // list of the excluded paths
    KRQuery                           *query;         // the filter used for the query
    bool                              stopped;        // 'Stop' button was pressed

    int                               equalsThreshold;// threshold to treat files equal
    int                               timeOffset;     // time offset between the left and right sides

    bool                              markEquals;     // show the equal files
    bool                              markDiffers;    // show the different files
    bool                              markCopyToLeft; // show the files to copy from right to left
    bool                              markCopyToRight;// show the files to copy from left to right
    bool                              markDeletable;  // show the files to be deleted
    bool                              markDuplicates; // show the duplicated items
    bool                              markSingles;    // show the single items

    bool                              leftCopyEnabled;// copy to left is enabled at synchronize
    bool                              rightCopyEnabled;// copy to right is enabled at synchronize
    bool                              deleteEnabled;  // delete is enabled at synchronize
    bool                              overWrite;      // overwrite or query each modification
    bool                              autoSkip;       // automatic skipping
    bool                              paused;         // pause flag
    bool                              disableNewTasks;// at mkdir the further task creation is disabled

    int                               leftCopyNr;     // the file number copied to left
    int                               rightCopyNr;    // the file number copied to right
    int                               deleteNr;       // the number of the deleted files
    int                               parallelThreads;// the number of the parallel procesing threads
    KIO::filesize_t                   leftCopySize;   // the total size copied to left
    KIO::filesize_t                   rightCopySize;  // the total size copied to right
    KIO::filesize_t                   deleteSize;     // the size of the deleted files

    int                               comparedDirs;   // the number of the compared directories
    int                               fileCount;      // the number of counted files

  private:
    QPtrList<SynchronizerTask>        stack;          // stack for comparing
    QMap<KIO::Job *,SynchronizerFileItem *> jobMap;   // job maps
    QMap<KIO::Job *,KIO::filesize_t>  receivedMap;    // the received file size
    SynchronizerFileItem             *lastTask;       // reference to the last stack
    int                               inTaskFinished; // counter of quasy 'threads' in slotTaskFinished

    QStringList                       selectedFiles;  // the selected files to compare
    QWidget                          *parentWidget;   // the parent widget
    QWidget                          *syncDlgWidget;  // the synchronizer dialog widget
};

#endif /* __SYNCHRONIZER_H__ */
