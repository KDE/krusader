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
#include "../VFS/krquery.h"
#include <qobject.h>
#include <qptrvector.h>
#include <qprogressdialog.h>
#include <qcolor.h>

typedef enum 
{
  TT_EQUALS        = 0,   // the files are equals     -> do nothing
  TT_DIFFERS       = 1,   // the files are differents -> don't know what to do
  TT_COPY_TO_LEFT  = 2,   // the right file is newer  -> copy from right to left
  TT_COPY_TO_RIGHT = 3,   // the left file is newer   -> copy from left to right
  TT_DELETE        = 4,   // the left file is single  -> delete it
  TT_MAX           = 5    // the maximum number of task types
} TaskType;

#define DECLARE_COLOR_NAME_ARRAY  QString COLOR_NAMES[] = { "Equals", "Differs", "LeftCopy", "RightCopy", "Delete" }
#define DECLARE_BACKGROUND_DFLTS  QColor BCKG_DFLTS[] = { QColor(), QColor(), QColor(), QColor(), Qt::red }
#define DECLARE_FOREGROUND_DFLTS  QColor FORE_DFLTS[] = { Qt::black, Qt::red,  Qt::blue, Qt::darkGreen, Qt::white }

#define SWAP( A, B, TYPE )      {TYPE TMP = A; A = B; B = TMP;}
#define REVERSE_TASK( A, asym ) {switch( A )                                           \
                                 {                                                     \
                                 case TT_COPY_TO_LEFT:                                 \
                                   if( asym )                                          \
                                     A = !m_existsRight ? TT_DELETE : TT_COPY_TO_LEFT; \
                                   else                                                \
                                     A = TT_COPY_TO_RIGHT;                             \
                                   break;                                              \
                                 case TT_COPY_TO_RIGHT:                                \
                                 case TT_DELETE:                                       \
                                   A = TT_COPY_TO_LEFT;                                \
                                 default:                                              \
                                   break;                                              \
                                 }};

class SynchronizerFileItem
{
  private:
    QString               m_leftName;     // the left file name
    QString               m_rightName;    // the right file name
    QString               m_leftDirectory;// the left relative directory path from the base
    QString               m_rightDirectory;// the left relative directory path from the base
    bool                  m_marked;       // flag, indicates to show the file
    bool                  m_existsLeft;   // flag, the file exists in the left directory
    bool                  m_existsRight;  // flag, the file exists in the right directory
    KIO::filesize_t       m_leftSize;     // the file size at the left directory
    KIO::filesize_t       m_rightSize;    // the file size at the right directory
    time_t                m_leftDate;     // the file date at the left directory
    time_t                m_rightDate;    // the file date at the left directory
    QString               m_leftLink;     // the left file's symbolic link destination
    QString               m_rightLink;    // the right file's symbolic link destination
    QString               m_leftOwner;    // the left file's owner
    QString               m_rightOwner;   // the right file's owner
    QString               m_leftGroup;    // the left file's group
    QString               m_rightGroup;   // the right file's group
    TaskType              m_task;         // the task with the file
    bool                  m_isDir;        // flag, indicates that the file is a directory
    SynchronizerFileItem *m_parent;       // pointer to the parent directory item or 0
    void                 *m_userData;     // user data
    bool                  m_overWrite;    // overwrite flag
    QString               m_destination;  // the destination URL at rename
    bool                  m_temporary;    // flag indicates temporary directory
    TaskType              m_originalTask; // the original task type
    
  public:
    SynchronizerFileItem(QString leftNam, QString rightNam, QString leftDir, QString rightDir, bool mark, 
                       bool exL, bool exR, KIO::filesize_t leftSize, KIO::filesize_t rightSize,
                       time_t leftDate, time_t rightDate, QString leftLink, QString rightLink, 
                       QString leftOwner, QString rightOwner, QString leftGroup, QString rightGroup, 
                       TaskType tsk, bool isDir, bool tmp, SynchronizerFileItem *parent ) :
                       m_leftName( leftNam ), m_rightName( rightNam ), m_leftDirectory( leftDir ),  m_rightDirectory( rightDir ),
                       m_marked( mark ),  m_existsLeft( exL ), m_existsRight( exR ), m_leftSize( leftSize ),
                       m_rightSize( rightSize ), m_leftDate( leftDate ), m_rightDate( rightDate ),
                       m_leftLink( leftLink ), m_rightLink( rightLink ), m_leftOwner( leftOwner ),
                       m_rightOwner( rightOwner ), m_leftGroup( leftGroup ), m_rightGroup( rightGroup ),
                       m_task( tsk ), m_isDir( isDir ), m_parent(parent), m_userData( 0 ), m_overWrite( false ),
                       m_destination( QString::null ), m_temporary( tmp ),
                       m_originalTask( tsk ) {}

    inline bool                   isMarked()              {return m_marked;}
    inline void                   setMarked( bool flag )  {m_marked = flag;}
    inline QString                leftName()              {return m_leftName;}
    inline QString                rightName()             {return m_rightName;}
    inline QString                leftDirectory()         {return m_leftDirectory;}
    inline QString                rightDirectory()        {return m_rightDirectory;}
    inline bool                   existsInLeft()          {return m_existsLeft;}
    inline bool                   existsInRight()         {return m_existsRight;}
    inline bool                   overWrite()             {return m_overWrite;}
    inline KIO::filesize_t        leftSize()              {return m_leftSize;}
    inline KIO::filesize_t        rightSize()             {return m_rightSize;}
    inline time_t                 leftDate()              {return m_leftDate;}
    inline time_t                 rightDate()             {return m_rightDate;}
    inline QString                leftLink()              {return m_leftLink;}
    inline QString                rightLink()             {return m_rightLink;}
    inline QString                leftOwner()             {return m_leftOwner;}
    inline QString                rightOwner()            {return m_rightOwner;}
    inline QString                leftGroup()             {return m_leftGroup;}
    inline QString                rightGroup()            {return m_rightGroup;}
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
    inline void                   swap( bool asym=false ) {SWAP( m_existsLeft, m_existsRight, bool );
                                                           SWAP( m_leftName, m_rightName, QString );
                                                           SWAP( m_leftDirectory, m_rightDirectory, QString );
                                                           SWAP( m_leftSize, m_rightSize, KIO::filesize_t );
                                                           SWAP( m_leftDate, m_rightDate, time_t );
                                                           SWAP( m_leftLink, m_rightLink, QString );
                                                           SWAP( m_leftOwner, m_rightOwner, QString );
                                                           SWAP( m_leftGroup, m_rightGroup, QString );
                                                           REVERSE_TASK( m_originalTask, asym );
                                                           REVERSE_TASK( m_task, asym );}
};

class Synchronizer : public QObject
{
  Q_OBJECT

  private:
    int     displayUpdateCount;   // the display is refreshed after every x-th change
  
  public:
    Synchronizer();
    int     compare( QString leftURL, QString rightURL, KRQuery *query, bool subDirs, bool symLinks,
                     bool igDate, bool asymm, bool cmpByCnt, bool igCase, bool autoSc, QStringList &selFiles );
    void    stop() {stopped = true;}
    void    setMarkFlags( bool left, bool equal, bool differs, bool right, bool dup, bool sing, bool del );
    int     refresh( bool nostatus=false );
    bool    totalSizes( int *, KIO::filesize_t *, int *, KIO::filesize_t *, int *, KIO::filesize_t * );
    void    synchronize( bool leftCopyEnabled, bool rightCopyEnabled, bool deleteEnabled, bool overWrite );
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
    bool    isDir( const vfile * file );
    QString readLink( const vfile * file );
    vfs *   getDirectory( QString urlIn );
    vfile * searchFile( vfs *vfs, QString file );
    
    void    compareDirectory( SynchronizerFileItem *,QString leftURL, QString rightURL,
                              QString leftDir, QString rightDir, QString addLeftName=QString::null,
                              QString addRightName=QString::null, QString addLeftDir=QString::null, 
                              QString addRightDir=QString::null, time_t addLTime=0, time_t addRTime=0,
                              QString leftLink=QString::null, QString rightLink=QString::null, 
                              QString leftOwner=QString::null, QString rightOwner=QString::null, 
                              QString leftGroup=QString::null, QString rightGroup=QString::null, 
                              bool isTemp = false );
    void    addSingleDirectory( SynchronizerFileItem *, QString, QString, time_t, QString, 
                                QString, QString, bool, bool );
    SynchronizerFileItem * addItem( SynchronizerFileItem *, QString, QString, QString, QString,
                                    bool, bool, KIO::filesize_t, KIO::filesize_t, time_t, time_t,
                                    QString, QString, QString, QString, QString, QString, TaskType, bool, bool);
    SynchronizerFileItem * addLeftOnlyItem( SynchronizerFileItem *, QString, QString,
                                            KIO::filesize_t, time_t, QString, QString, QString,
                                            bool isDir = false, bool isTemp = false );
    SynchronizerFileItem * addRightOnlyItem( SynchronizerFileItem *, QString, QString,
                                             KIO::filesize_t, time_t, QString,  QString, QString,
                                             bool isDir = false, bool isTemp = false  );
    SynchronizerFileItem * addDuplicateItem( SynchronizerFileItem *, QString, QString, QString, QString,
                                             KIO::filesize_t, KIO::filesize_t, time_t,
                                             time_t, QString, QString, QString, QString, QString, 
                                             QString, bool isDir = false, bool isTemp = false  );
    bool    isMarked( TaskType task, bool dupl );
    bool    markParentDirectories( SynchronizerFileItem * );
    void    executeTask();
    bool    compareByContent( QString, QString, QString, QString );
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
    bool                              ignoreCase;     // case insensitive synchronization for Windows fs
    bool                              autoScroll;     // automatic update of the directory
    QPtrList<SynchronizerFileItem>    resultList;     // the found files
    QString                           leftBaseDir;    // the left-side base directory
    QString                           rightBaseDir;   // the right-side base directory
    QStringList                       excludedPaths;  // list of the excluded paths
    KRQuery                           *query;         // the filter used for the query
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
    QStringList                       selectedFiles;  // the selected files to compare
    QWidget                          *parentWidget;   // the parent widget
};

#endif /* __SYNCHRONIZER_H__ */
