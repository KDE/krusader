/***************************************************************************
                          diskusage.h  -  description
                             -------------------
    copyright            : (C) 2004 by Csaba Karai
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

#ifndef __DISK_USAGE_H__
#define __DISK_USAGE_H__

#include "../VFS/vfs.h"
#include "filelightParts/fileTree.h"

#include <qdialog.h>
#include <qlabel.h>
#include <q3dict.h>
#include <q3ptrlist.h>
#include <q3ptrdict.h>
#include <q3valuestack.h>
#include <q3ptrstack.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QPixmap>
#include <QKeyEvent>
#include <QEvent>
#include <kurl.h>
#include <ksqueezedtextlabel.h>
#include <q3widgetstack.h>
#include <q3scrollview.h>
#include <qtimer.h>

#define VIEW_LINES      0
#define VIEW_DETAILED   1
#define VIEW_FILELIGHT  2
#define VIEW_LOADER     3

typedef Q3Dict<void> Properties;

class DUListView;
class DULines;
class DUFilelight;
class KMenu;
class LoaderWidget;

class DiskUsage : public Q3WidgetStack
{
  Q_OBJECT
  
public:
  DiskUsage( QString confGroup, QWidget *parent = 0);
  ~DiskUsage();
  
  void       load( KUrl dirName );
  void       close();
  void       stopLoad();
  bool       isLoading()     { return loading; }
  
  void       setView( int view );
  int        getActiveView() { return activeView; }
  
  Directory* getDirectory( QString path );
  File *     getFile( QString path );
  
  QString    getConfigGroup() { return configGroup; }
  
  void *     getProperty( File *, QString );
  void       addProperty( File *, QString, void * );
  void       removeProperty( File *, QString );
  
  int        exclude( File *file, bool calcPercents = true, int depth = 0 );
  void       includeAll();
  
  int        del( File *file, bool calcPercents = true, int depth = 0 );
  
  QString    getToolTip( File * );
  
  void       rightClickMenu( File *, KMenu * = 0, QString = QString() );
  
  void       changeDirectory( Directory *dir );
  
  Directory* getCurrentDir();
  File*      getCurrentFile();
  
  QPixmap    getIcon( QString mime );
  
  KUrl       getBaseURL() { return baseURL; }
  
public slots:  
  void       dirUp();
  void       clear();
  
signals:
  void       enteringDirectory( Directory * );
  void       clearing();
  void       changed( File * );
  void       changeFinished();
  void       deleted( File * );
  void       deleteFinished();
  void       status( QString );
  void       viewChanged( int );
  void       loadFinished( bool );
  void       newSearch();

protected slots:
  void       slotLoadDirectory();

protected:
  Q3Dict< Directory > contentMap;
  Q3PtrDict<Properties> propertyMap;
    
  Directory* currentDirectory;
  KIO::filesize_t currentSize;
 
  virtual void keyPressEvent( QKeyEvent * );
  virtual bool event( QEvent * );
  
  int        calculateSizes( Directory *dir = 0, bool emitSig = false, int depth = 0 );
  int        calculatePercents( bool emitSig = false, Directory *dir = 0 , int depth = 0 );
  int        include( Directory *dir, int depth = 0 );
  void       createStatus();
  void       executeAction( int, File * = 0 );
  
  KUrl       baseURL;             //< the base URL of loading

  DUListView                *listView;
  DULines                   *lineView;
  DUFilelight               *filelightView;
  LoaderWidget              *loaderView;
  
  Directory *root;
  
  int        activeView;
  
  QString    configGroup;
  
  bool       first;
  bool       loading;
  bool       abortLoading;
  bool       clearAfterAbort;
  bool       deleting;

  Q3ValueStack<QString> directoryStack;
  Q3PtrStack<Directory> parentStack;

  vfs       * searchVfs;
  vfile     * currentVfile;
  Directory * currentParent;
  QString     dirToCheck;
  
  int   fileNum; 
  int   dirNum;
  int   viewBeforeLoad;

  QTimer loadingTimer;
};


class LoaderWidget : public Q3ScrollView
{
  Q_OBJECT
  
public:
  LoaderWidget( QWidget *parent = 0 );
  
  void init();
  void setCurrentURL( KUrl url );
  void setValues( int fileNum, int dirNum, KIO::filesize_t total );  
  bool wasCancelled()  { return cancelled; }
  
public slots:
  void slotCancelled();
  
protected:
  virtual void resizeEvent ( QResizeEvent *e );
  
  QLabel *totalSize;
  QLabel *files;
  QLabel *directories;
  
  KSqueezedTextLabel *searchedDirectory;
  QWidget *widget;
    
  bool   cancelled;
};

#endif /* __DISK_USAGE_GUI_H__ */
