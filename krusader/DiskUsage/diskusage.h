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

#include <qdialog.h>
#include <qlabel.h>
#include <qdict.h>
#include <qptrlist.h>
#include <qptrdict.h>
#include <kurl.h>
#include <ksqueezedtextlabel.h>

typedef QDict<void> Properties;

class DiskUsageDialog : public QDialog
{
  Q_OBJECT
  
public:
  DiskUsageDialog( QWidget *parent = 0, const char *name = 0 );
  
  void   setCurrentURL( KURL url );
  void   setValues( int fileNum, int dirNum, KIO::filesize_t total );
  
  bool   wasCancelled()  { return cancelled; }
  
public slots:
  void    slotCancelled();
  virtual void reject();
  
protected:
  QLabel *totalSize;
  QLabel *files;
  QLabel *directories;
  
  KSqueezedTextLabel *searchedDirectory;
  
  bool   cancelled;
};

class DiskUsageItem
{
private:
  QString         m_name;     //< file name
  QString         m_directory;//< the directory of the file
  KIO::filesize_t m_size;     //< size with subdirectories
  KIO::filesize_t m_ownSize;  //< size without subdirectories
  mode_t          m_mode;     //< file mode
  QString         m_owner;    //< file owner name
  QString         m_group;    //< file group name
  QString         m_perm;     //< file permissions string
  time_t          m_time;     //< file modification in time_t format
  bool            m_symLink;  //< true if the file is a symlink
  QString         m_mimeType; //< file mimetype
  bool            m_excluded; //< flag if the file is excluded from du
  int             m_percent;  //< percent flag
  
public:
  DiskUsageItem( QString nameIn, QString dir, KIO::filesize_t sizeIn, mode_t modeIn,  QString ownerIn,
                 QString groupIn, QString permIn, time_t timeIn, bool symLinkIn, QString mimeTypeIn )
  : m_name( nameIn ), m_directory( dir ), m_size( sizeIn ), m_ownSize( sizeIn ), m_mode( modeIn ), 
    m_owner( ownerIn ), m_group( groupIn ), m_perm( permIn ), m_time( timeIn ), m_symLink( symLinkIn ), 
    m_mimeType( mimeTypeIn ), m_excluded( false ), m_percent( -1 ) {}
      
  inline QString          name()                {return m_name;}
  inline QString          directory()           {return m_directory;}
  inline KIO::filesize_t  size()                {return m_size;}
  inline KIO::filesize_t  ownSize()             {return m_ownSize;}
  inline mode_t           mode()                {return m_mode;}
  inline QString          owner()               {return m_owner;}
  inline QString          group()               {return m_group;}
  inline QString          perm()                {return m_perm;}
  inline time_t           time()                {return m_time;}
  inline QString          mime()                {return m_mimeType;}
  inline bool             isSymLink()           {return m_symLink;}
  inline bool             isDir()               {return m_perm[0]=='d';}
  inline bool             isExcluded()          {return m_excluded;}
  inline void             exclude( bool flag )  {m_excluded = flag;}
  inline int              intPercent()          {return m_percent;}
  inline QString          percent()             {if( m_percent < 0 )
                                                   return "INV";
                                                 char buf[ 25 ];  
                                                 sprintf( buf, "%d.%02d%%", m_percent / 100, m_percent % 100 );
                                                 return QString( buf );}
  inline void             setPercent( int p )   {m_percent = p;}  
  
  inline void setSizes( KIO::filesize_t totalSize, KIO::filesize_t ownSize )
  {
    m_ownSize = ownSize;
    m_size = totalSize;
  }

};

class DiskUsage : public QObject
{
  Q_OBJECT
  
public:
  DiskUsage();
  
  bool     load( KURL dirName, QWidget *parent );
  void     clear();
  QPtrList<DiskUsageItem> *getDirectory( QString dir );
  DiskUsageItem           *getItem( QString path );
  
  void *   getProperty( DiskUsageItem *, QString );
  void     addProperty( DiskUsageItem *, QString, void * );
  void     removeProperty( DiskUsageItem *, QString );
  
  void     exclude( QString dir, QString name );
  void     includeAll();
  
  QString  getToolTip( DiskUsageItem * );
  
  void     rightClickMenu( DiskUsageItem * );
  
  void     changeDirectory( QString dir );
  QString  getCurrentDir();
  QPixmap  getIcon( QString mime );
  
signals:
  void     directoryChanged( QString );
  void     clearing();
  void     changed( DiskUsageItem * );
  void     status( QString );
  
protected:
  QDict< QPtrList<DiskUsageItem> > contentMap;
  QPtrDict<Properties> propertyMap;
  
  QString  currentDirectory;
  
  void     calculateSizes();
  void     calculatePercents( bool emitSig = false );
  void     calculatePercentsDir( QString dir, KIO::filesize_t currentSize, bool emitSig );
  void     calculateDirSize( QString dir, KIO::filesize_t &total, KIO::filesize_t &own, bool emitSig = false );
  void     excludeDir( QString dir );
  void     includeDir( QString dir );
  void     createStatus();
  
  KIO::filesize_t totalSize;     //< size with subdirectories
  KIO::filesize_t ownSize;       //< size without subdirectories

  KURL      baseURL;             //< the base URL of loading
};

#endif /* __DISK_USAGE_GUI_H__ */
