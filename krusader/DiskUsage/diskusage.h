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
#include <qdict.h>
#include <qptrlist.h>
#include <qptrdict.h>
#include <kurl.h>
#include <ksqueezedtextlabel.h>
#include <qwidgetstack.h>
#include <qscrollview.h>

#define VIEW_LINES      0
#define VIEW_DETAILED   1
#define VIEW_FILELIGHT  2
#define VIEW_LOADER     3

typedef QDict<void> Properties;

class DUListView;
class DULines;
class DUFilelight;
class KPopupMenu;
class LoaderWidget;

class DiskUsage : public QWidgetStack
{
  Q_OBJECT
  
public:
  DiskUsage( QString confGroup, QWidget *parent = 0, char *name = 0);
  ~DiskUsage();
  
  bool       load( KURL dirName );
  
  void       setView( int view );
  int        getActiveView() { return activeView; }
  
  Directory* getDirectory( QString path );
  File *     getFile( QString path );
  
  QString    getConfigGroup() { return configGroup; }
  
  void *     getProperty( File *, QString );
  void       addProperty( File *, QString, void * );
  void       removeProperty( File *, QString );
  
  void       exclude( File *file, bool calcPercents = true );
  void       includeAll();
  
  void       del( File *file, bool calcPercents = true );
  
  QString    getToolTip( File * );
  
  void       rightClickMenu( File *, KPopupMenu * = 0, QString = QString::null );
  
  void       changeDirectory( Directory *dir );
  
  Directory* getCurrentDir();
  File*      getCurrentFile();
  
  QPixmap    getIcon( QString mime );
  
  KURL       getBaseURL() { return baseURL; }
  
public slots:  
  void       dirUp();
  void       clear();
  
signals:
  void       enteringDirectory( Directory * );
  void       clearing();
  void       changed( File * );
  void       deleted( File * );
  void       status( QString );
  void       viewChanged( int );
  void       loadFinished( bool );
  void       newSearch();
  
protected:
  QDict< Directory > contentMap;
  QPtrDict<Properties> propertyMap;
    
  Directory* currentDirectory;
  KIO::filesize_t currentSize;
 
  virtual void keyPressEvent( QKeyEvent * );
  
  void       calculateSizes( Directory *dir = 0, bool emitSig = false );
  void       calculatePercents( bool emitSig = false, Directory *dir = 0  );
  void       include( Directory *dir );
  void       createStatus();
  void       executeAction( int, File * = 0 );
  
  KURL       baseURL;             //< the base URL of loading

  DUListView                *listView;
  DULines                   *lineView;
  DUFilelight               *filelightView;
  LoaderWidget              *loaderView;
  
  Directory *root;
  
  int        activeView;
  
  QString    configGroup;
};


class LoaderWidget : public QScrollView
{
  Q_OBJECT
  
public:
  LoaderWidget( QWidget *parent = 0, const char *name = 0 );
  
  void init();
  void setCurrentURL( KURL url );
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
