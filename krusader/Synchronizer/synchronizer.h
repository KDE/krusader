/***************************************************************************
                        synchronizer.h  -  description
                             -------------------
    copyright            : (C) 2003 by Csaba Karai
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

#include "../VFS/vfs.h"
#include <qobject.h>
#include <qptrvector.h>
#include <qprogressdialog.h>

typedef enum 
{
  TT_EQUALS        = 0,   // the files are equals     -> do nothing
  TT_DIFFERS       = 1,   // the files are differents -> don't know what to do
  TT_COPY_TO_LEFT  = 2,   // the right file is newer  -> copy from right to left
  TT_COPY_TO_RIGHT = 3,   // the left file is newer   -> copy from left to right
  TT_DELETE        = 4    // the left file is single  -> delete it
} TaskType;

class SynchronizerFileItem
{
  private:
    QString               m_name;         // the file name
    QString               m_directory;    // the relative directory path from the base
    bool                  m_marked;       // flag, indicates to show the file
    bool                  m_existsLeft;   // flag, the file exists in the left directory
    bool                  m_existsRight;  // flag, the file exists in the right directory
    KIO::filesize_t       m_leftSize;     // the file size at the left directory
    KIO::filesize_t       m_rightSize;    // the file size at the right directory
    time_t                m_leftDate;     // the file date at the left directory
    time_t                m_rightDate;    // the file date at the left directory
    TaskType              m_task;         // the task with the file
    bool                  m_isDir;        // flag, indicates that the file is a directory
    SynchronizerFileItem *m_parent;       // pointer to the parent directory item or 0
    void                 *m_userData;     // user data
    bool                  m_overWrite;    // overwrite flag
    QString               m_destination;  // the destination URL at rename
    bool                  m_temporary;    // flag indicates temporary directory
    TaskType              m_originalTask; // the original task type
    
  public:
    SynchronizerFileItem(QString nam, QString dir, bool mark, bool exL,
                       bool exR, KIO::filesize_t leftSize, KIO::filesize_t rightSize,
                       time_t leftDate, time_t rightDate, TaskType tsk, bool isDir,
                       bool tmp, SynchronizerFileItem *parent ) :
                       m_name( nam ), m_directory( dir ), m_marked( mark ),
                       m_existsLeft( exL ), m_existsRight( exR ), m_leftSize( leftSize ),
                       m_rightSize( rightSize ), m_leftDate( leftDate ),
                       m_rightDate( rightDate ),m_task( tsk ), m_isDir( isDir ),
                       m_parent(parent), m_userData( 0 ), m_overWrite( false ),
                       m_destination( QString::null ), m_temporary( tmp ),
                       m_originalTask( tsk ) {}

    inline bool                   isMarked()              {return m_marked;}
    inline void                   setMarked( bool flag )  {m_marked = flag;}
    inline QString                name()                  {return m_name;}
    inline QString                directory()             {return m_directory;}
    inline bool                   existsInLeft()          {return m_existsLeft;}
    inline bool                   existsInRight()         {return m_existsRight;}
    inline bool                   overWrite()             {return m_overWrite;}
    inline KIO::filesize_t        leftSize()              {return m_leftSize;}
    inline KIO::filesize_t        rightSize()             {return m_rightSize;}
    inline time_t                 leftDate()              {return m_leftDate;}
    inline time_t                 rightDate()             {return m_rightDate;}
    inline TaskType               task()                  {return m_task;}
    inline bool                   isDir()                 {return m_isDir;}
    inline SynchronizerFileItem * parent()                {return m_parent;}
    inline void *                 userData()              {return m_userData;}
    inline void                   setUserData( void *ud)  {m_userData = ud;}
    inline void                   setOverWrite()          {m_overWrite = true;}
    inline QString                destination()           {return m_destination;}
    inline void                   setDestination(QString d) {m_destination = d;}
    inline bool                   isTemporary()           {return m_temporary;}
    inline void                   setPermanent()          {m_temporary = false;}
    inline TaskType               originalTask()          {return m_originalTask;}
    inline void                   restoreOriginalTask()   {m_task = m_originalTask;}
    inline void                   setTask( TaskType t )   {m_task = t;}
};

class Synchronizer : public QObject
{
  Q_OBJECT

  private:
    int     displayUpdateCount;   // the display is refreshed after every x-th change
  
  public:
    Synchronizer();
    int     compare( QString leftURL, QString rightURL, QString filter, bool subDirs, bool symLinks,
                     bool igDate, bool asymm, bool cmpByCnt, bool autoSc );
    void    stop() {stopped = true;}
    void    setMarkFlags( bool left, bool equal, bool differs, bool right, bool dup, bool sing, bool del );
    int     refresh( bool nostatus=false );
    bool    totalSizes( int *, KIO::filesize_t *, int *, KIO::filesize_t *, int *, KIO::filesize_t * );
    void    synchronize( bool leftCopyEnabled, bool rightCopyEnabled, bool deleteEnabled, bool overWrite );
    void    pause();
    void    resume();

    void    exclude( SynchronizerFileItem * );
    void    restore( SynchronizerFileItem * );
    void    reverseDirection( SynchronizerFileItem * );
    void    copyToLeft( SynchronizerFileItem * );
    void    copyToRight( SynchronizerFileItem * );
    void    deleteLeft( SynchronizerFileItem * );
    
    KURL    fromPathOrURL( QString url );
    QString leftBaseDirectory();
    QString rightBaseDirectory();
    static QString getTaskTypeName( TaskType taskType );

    SynchronizerFileItem *getItemAt( unsigned ndx ) {return resultList.at(ndx);}

  signals:
    void    comparedFileData( SynchronizerFileItem * );
    void    markChanged( SynchronizerFileItem * );
    void    synchronizationFinished();
    void    processedSizes( int, KIO::filesize_t, int, KIO::filesize_t, int, KIO::filesize_t );
    void    pauseAccepted();
    void    statusInfo( QString );

  public slots:
    void    slotTaskFinished(KIO::Job*);
    void    slotProcessedSize( KIO::Job * , KIO::filesize_t );
    void    slotDataReceived(KIO::Job *job, const QByteArray &data);
    void    slotFinished(KIO::Job *job);
    void    putWaitWindow();
    void    comparePercent(KIO::Job *, unsigned long);
    
  private:
    vfs *   getDirectory( QString urlIn );
    
    void    compareDirectory( SynchronizerFileItem *,QString leftURL, QString rightURL,
                              QString dir, QString addName=QString::null,
                              QString addDir=QString::null, time_t addLTime=0, time_t addRTime=0 );
    void    addSingleDirectory( SynchronizerFileItem *, QString, QString, time_t, bool );
    SynchronizerFileItem * addItem( SynchronizerFileItem *, QString, QString,
                                    bool, bool, KIO::filesize_t, KIO::filesize_t,
                                    time_t, time_t, TaskType, bool, bool);
    SynchronizerFileItem * addLeftOnlyItem( SynchronizerFileItem *, QString, QString,
                                            KIO::filesize_t, time_t, bool isDir = false, bool isTemp = false );
    SynchronizerFileItem * addRightOnlyItem( SynchronizerFileItem *, QString, QString,
                                             KIO::filesize_t, time_t, bool isDir = false, bool isTemp = false  );
    SynchronizerFileItem * addDuplicateItem( SynchronizerFileItem *, QString, QString,
                                             KIO::filesize_t, KIO::filesize_t, time_t,
                                             time_t, bool isDir = false, bool isTemp = false  );
    bool    checkName( QString name );
    bool    isMarked( TaskType task, bool dupl );
    bool    markParentDirectories( SynchronizerFileItem * );
    void    executeTask();
    bool    compareByContent( QString, QString );
    void    abortContentComparing();
    void    setPermanent( SynchronizerFileItem * );
    void    operate( SynchronizerFileItem *item, void (*)(SynchronizerFileItem *) );

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
    bool                              autoScroll;     // automatic update of the directory
    QPtrList<SynchronizerFileItem>    resultList;     // the found files
    QString                           leftBaseDir;    // the left-side base directory
    QString                           rightBaseDir;   // the right-side base directory
    QString                           fileFilter;     // the file selection filter
    bool                              stopped;        // 'Stop' button was pressed

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

    int                               leftCopyNr;     // the file number copied to left
    int                               rightCopyNr;    // the file number copied to right
    int                               deleteNr;       // the number of the deleted files
    KIO::filesize_t                   leftCopySize;   // the total size copied to left
    KIO::filesize_t                   rightCopySize;  // the total size copied to right
    KIO::filesize_t                   deleteSize;     // the size of the deleted files

    int                               scannedDirs;    // the number of scanned directories
    int                               fileCount;      // the number of counted files
    
    SynchronizerFileItem *            currentTask;    // the current task to process

  private:
    KURL                              leftURL;        // the currently processed URL (left)
    KURL                              rightURL;       // the currently processed URL (right)

    bool                              compareFinished;// flag indicates, that comparation is finished
    bool                              compareResult;  // the result of the comparation
    bool                              statusLineChanged; // true if the status line was changed
    bool                              errorPrinted;   // flag indicates error
    QProgressDialog                  *waitWindow;     // the wait window
    KIO::TransferJob                 *leftReadJob;    // compare left read job
    KIO::TransferJob                 *rightReadJob;   // compare right read job
    QByteArray                        compareArray;   // the array for comparing
    QTimer                           *timer;          // timer to show the process dialog at compare by content
};

#endif /* __SYNCHRONIZER_H__ */
