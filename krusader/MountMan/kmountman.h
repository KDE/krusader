/***************************************************************************
                               kmountman.h
                            -------------------
   begin                : Thu May 4 2000
   copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
   e-mail               : krusader@users.sourceforge.net
   web site             : http://krusader.sourceforge.net
---------------------------------------------------------------------------
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
#ifndef KMOUNTMAN_H
#define KMOUNTMAN_H

// QT includes
#include <qobject.h>
#include <qstring.h>
#include <qframe.h>
#include <qlist.h> 
// KDE includes
#include <kprocess.h>
#include <kjanuswidget.h>
#include <kio/jobclasses.h>
#include <ktempfile.h> 
// krusader includes
#include <stdlib.h>
#include <math.h>

namespace MountMan {

  // forward definitions
  class fsData;

  class KMountMan : public QObject {
      Q_OBJECT
      friend class KMountManGUI;

    public:
      enum mntStatus {DOESNT_EXIST, NOT_MOUNTED, MOUNTED};

      inline bool operational() {
        return Operational;
      } // check this 1st
      inline bool ready() {
        return Ready;
      } // poll on this until true
      void mainWindow();                               // opens up the GUI
      void mount( QString mntPoint );           // this is probably what you need for mount
      void unmount( QString mntPoint );         // this is probably what you need for unmount
      mntStatus getStatus( QString mntPoint );  // return the status of a mntPoint (if any)
      void autoMount( QString path );           // just call it before refreshing into a dir
      static void eject( QString mntPoint );
      bool ejectable( QString path );
      QString convertSize( unsigned long size );
      //////////////////////////// service functions /////////////////////////////////
      static QString nextWord( QString &s );
      static QString nextWord( QString &s, char c );
      KMountMan();
      ~KMountMan();

    protected slots:
      void parseDfData( QString filename );  // parse a FULL list of filesystems
      void forceUpdate();
      void collectOutput( KProcess *p, char *buffer, int buflen );
      void finishUpdateFilesystems();
      void killMountMan();                  // called when df never finished (error)

    signals:
      void updated();

    protected:
      void mount( fsData *p );              // you don't want to call this one !
      void unmount( fsData *p );            // you don't want to call this one !
      void toggleMount( QString device );   // you don't want to call this one !
      QString followLink( QString source ); // internal to mountman/gui
      bool createFilesystems();           // potential filesystems from /etc/fstab
      void updateFilesystems();           // actually mounted systems using DF -T -P
      QString getDevice( QString mntPoint ); // mntPoint ==> device
      fsData* location( QString name );     // device   ==> mntPoint
      // using /etc/mtab
      QString pointFromMtab( QString device ); // returns mountPoint for a device
      QString devFromMtab( QString mntPoint ); // returns device for a mountPoint

    private:
      void clearOutput();
      inline QString getOutput() {
        return QString( ( !outputBuffer ?
                          QString::null : *outputBuffer ) );
      }

    private:
      QStringList mountPoints;  // all mountPoints excluding SUPERMOUNTED ones
      QList<fsData> filesystems;
      int noOfFilesystems;
      bool Ready;         // true if MountMan finished all preprocessing
      bool Operational;   // if false, something went terribly wrong on startup
      fsData *localDf;    // used for moving around information
      QString *outputBuffer; // all processes output their data here
      KTempFile *tempFile;
      KShellProcess dfProc;
      KMountManGUI *mountManGui;
  };

  // collects statistics about a path, create a label with the results and emit a singal
  class statsCollector : public QObject {
      Q_OBJECT
    public:
      statsCollector( QString, QObject * ); // this is what you'll need to use

    protected:
      void getData( QString path, fsData * );     // returns info about the whereabouts of path
      void parseDf( QString filename, fsData * ); // parse a single call for DF

    signals:
      void gotStats( QString ); // emitted when we have the stats for a path
  };

  // Data container for a single-filesystem data
  // maximum size supported is 2GB of 1kb blocks == 2048GB, enough.
  /////////////////////////////////////////////////////////////////
  class fsData {
    public:
      fsData() : Name( 0 ), Type( 0 ), MntPoint( 0 ), TotalBlks( 0 ),
      FreeBlks( 0 ), Mounted( false ) {}

      // get information
      inline QString name() {
        return Name;
      }
      inline QString shortName() {
        return Name.right( Name.length() - Name.find( "/", 1 ) - 1 );
      }
      inline QString type() {
        return Type;
      }
      inline QString mntPoint() {
        return MntPoint;
      }
      inline long totalBlks() {
        return TotalBlks;
      }
      inline long freeBlks() {
        return FreeBlks;
      }
      inline long long totalBytes() {
        return TotalBlks * 1024;
      }
      inline long long freeBytes() {
        return FreeBlks * 1024;
      }
      //////////////////// insert a good round function here /////////////////
      int usedPerct() {
        if ( TotalBlks == 0 )
          return 0;
        float res = ( ( float ) (TotalBlks-FreeBlks) ) / ( ( float ) TotalBlks ) * 100;
        if ( ( res - ( int ) res ) > 0.5 )
          return ( int ) res + 1;
        else
          return ( int ) res;
      }
      inline bool mounted() {
        return Mounted;
      }

      // set information
      inline void setName( QString n_ ) {
        Name = n_;
      }
      inline void setType( QString t_ ) {
        Type = t_;
      }
      inline void setMntPoint( QString m_ ) {
        MntPoint = m_;
      }
      inline void setTotalBlks( long t_ ) {
        TotalBlks = t_;
      }
      inline void setFreeBlks( long f_ ) {
        FreeBlks = f_;
      }
      inline void setMounted( bool m_ ) {
        Mounted = m_;
      }

    private:
      QString Name;       // i.e: /dev/cdrom
      QString Type;       // i.e: iso9600
      QString MntPoint;   // i.e: /mnt/cdrom
      long TotalBlks;  // measured in 1024bytes per block
      long FreeBlks;
      bool Mounted;    // true if filesystem is mounted

      // additional attributes of a filesystem, parsed from fstab
    public:
      bool supermount;    // is the filesystem supermounted ?
      QString options;    // additional fstab options

  };

};

#endif
