/***************************************************************************
                        kmountman.cpp
                     -------------------
copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
e-mail               : krusader@users.sourceforge.net
web site             : http://krusader.sourceforge.net
---------------------------------------------------------------------------

A 

db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                             S o u r c e    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#include <sys/param.h>
#include <time.h>
#include "kmountman.h" 
// QT incldues
#include <qcstring.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qtimer.h>
#include <qdir.h> 
// KDE includes
#include <kmessagebox.h>
#include <kprocess.h>
#include <kprocctrl.h>
#include <kio/job.h>
#include <klocale.h>
#include <kpopupmenu.h>

#if KDE_IS_VERSION(3,2,0)
#include <kmountpoint.h>
#endif

// Krusader includes
#include "../resources.h"
#include "../krusader.h"
#include "../Dialogs/krdialogs.h"
#include "../krservices.h"
#include "kmountmangui.h"
#include <unistd.h>

#ifdef _OS_SOLARIS_
#define FSTAB "/etc/vfstab"
#else
#define FSTAB "/etc/fstab"
#endif

#define  DF_WAIT_TIME             60

bool dfStartFailed = false;

using namespace MountMan;

KMountMan::KMountMan() : QObject(), Ready( false ), Operational( false ),
outputBuffer( 0 ), tempFile( 0 ), mountManGui( 0 ), mtab( "" ) {
#if KDE_IS_VERSION(3,2,0)
   _actions = 0L;
#endif /* KDE 3.2 */

   filesystems.setAutoDelete( true );
   localDf = new fsData();				 // will be used to move around information
   forceUpdate();
}

KMountMan::~KMountMan() {}

// find the next word delimited by 'c'
QString KMountMan::nextWord( QString &s, char c ) {
   s = s.stripWhiteSpace();
   int j = s.find( c, 0 );
   if ( j == -1 ) {    // if the delimiter wasn't found, return the entire
      QString tmp = s; // input, clearing the original string, to indicate
      s = "";         // that the whole input was taken
      return tmp;
   }
   QString temp = s.left( j ); // find the leftmost word.
   s.remove( 0, j );
   return temp;
}

void KMountMan::mainWindow() {
   mountManGui = new KMountManGUI();
   delete mountManGui;   /* as KMountManGUI is modal, we can now delete it */
   mountManGui = 0; /* for sanity */
}

// this version find the next word, delimeted by anything from
// comma, space, tab or newline, in this order
QString KMountMan::nextWord( QString &s ) {
   s = s.stripWhiteSpace();
   int j;
   j = s.find( ',', 0 );
   if ( j == -1 )
      j = s.find( ' ', 0 );
   if ( j == -1 )
      j = s.find( '\t', 0 );
   if ( j == -1 )
      j = s.find( '\n', 0 );
   // one of the above must have made j>0, if something went wrong,
   // we return the entire input, clearing the original string
   if ( j == -1 ) {
      QString tmp = s;
      s = "";
      return tmp;
   }
   QString temp = s.left( j ); // find the leftmost word.
   s.remove( 0, j );

   return temp;
}

QString KMountMan::followLink( QString source ) {
   // ugly hack - disable link following because of devfs
   return source;

   QFileInfo temp( source );
   // if the file doesn't exist and it contains //, it's probably a samba share
   // <patch> thanks to Cristi Dumitrescu
   if ( !temp.exists() && temp.filePath().contains( "//" ) > 0 )
      return source;
   while ( temp.isSymLink() )
      temp.setFile( temp.readLink() );
   return temp.fileName();
}

bool KMountMan::createFilesystems() {
   QString temp[ 5 ][ 100 ];  // a temporary array which allows parsing of upto 100 fs
   QString dumb;          // my very-own dumb pipe: nothing goes out of it :-)
   QString s;
   int i = 0, j = 0;

   noOfFilesystems = 0;
   // if |filesystems|>0, than we are re-creating, delete the old list
   if ( filesystems.count() > 0 )
      while ( filesystems.removeLast() )
         ;
   // open the /etc/fstab file...
   QFile fstab( FSTAB );
   if ( !fstab.open( IO_ReadOnly ) ) {
      kdWarning() << "Mt.Man: Unable to read " << QString( FSTAB ) << " !!! Shutting down. (sorry)" << endl;
      return false;
   }
   // and read it into the temporary array
   QTextStream t( &fstab );
   //kdWarning() << "debug: createFilesystems" << endl;
   while ( !fstab.atEnd() ) {
      s = t.readLine();
      s = s.simplifyWhiteSpace(); // remove TABs
      if ( s == QString::null || s == "" )
         continue;  // skip empty lines in fstab
      // temp[0]==name, temp[1]==type, temp[2]==mount point, temp[3]==options
      // temp[4] is reserved for special cases, right now, only for supermount
      //kdWarning() << "debug: " << s << endl;
      bool remark = false;
      ( temp[ 0 ][ i ] ) = nextWord( s, ' ' );
#if defined(_OS_SOLARIS_)

      nextWord( s, ' ' );
#endif

      ( temp[ 2 ][ i ] ) = nextWord( s, ' ' );
      ( temp[ 1 ][ i ] ) = nextWord( s, ' ' );
#if !defined(_OS_SOLARIS_)

      ( temp[ 3 ][ i ] ) = nextWord( s, ' ' );
#endif
      // now, we check if a remark was inserted in the line
      for ( int cnt = 0; cnt < 3; ++cnt )
         if ( temp[ cnt ][ i ] == "#" || temp[ cnt ][ i ].left( 1 ) == "#" ) {
            //kdDebug() << "MountMan: found a remark in fstab, skipping the line." << endl;
            remark = true;
            break;
         }
      // if the filesystem is supermount, then the whole fstab is not like
      // we expected, so alterations must be made... mucho work...
      if ( temp[ 1 ][ i ] == "supermount" ) {
         temp[ 4 ][ i ] = "supermount"; // mark the filesystem
         // temp[3][i] holds the entire 'options' string
         QString t = ( temp[ 3 ][ i ] ).mid( ( temp[ 3 ][ i ] ).find( "fs=", 0 ) + 3 );
         temp[ 1 ][ i ] = nextWord( t );
         t = ( temp[ 3 ][ i ] ).mid( ( temp[ 3 ][ i ] ).find( "dev=", 0 ) + 4 );
         temp[ 0 ][ i ] = nextWord( t );
         // now we have to cut temp[3][i] down to remove supermount info from it
         int i1 = ( temp[ 3 ][ i ] ).find( "fs=", 0 ) + 3;
         int i2 = ( temp[ 3 ][ i ] ).find( "dev=", 0 ) + 4;
         // find where the supermount info ends and the other options begin
         int i3 = ( temp[ 3 ][ i ] ).find( ',', ( i1 > i2 ? i1 : i2 ) );
         t = ( temp[ 3 ][ i ] ).mid( i3 + 1 );    // got the entire line ... all we need to do
         ( temp[ 3 ][ i ] ) = nextWord( t, ' ' ); // is remove the '0 0' or '0 1' at the end
         ( temp[ 3 ][ i ] ).simplifyWhiteSpace(); // make sure an empty string is empty!
      }
      if ( !remark )
         ++i; // count this as a filesystem, only if it's not a remark
   }
   --i;
   fstab.close();  // finished with it
   for ( j = 0; j <= i; ++j ) {
      if ( temp[ 0 ][ j ] == "" || temp[ 0 ][ j ] == "tmpfs" || temp[ 0 ][ j ] == "none" || temp[ 0 ][ j ] == "proc" ||
#if defined(BSD)
            temp[ 0 ][ j ] == "swap" || temp[ 1 ][ j ] == "procfs" || temp[ 1 ][ j ] == "/dev/pts" ||         // FreeBSD: procfs instead of proc
#else
            temp[ 0 ][ j ] == "swap" || temp[ 1 ][ j ] == "proc" || temp[ 1 ][ j ] == "/dev/pts" ||
#endif
            temp[ 1 ][ j ] == "swap" || temp[ 4 ][ j ] == "supermount" )
         continue;
      ++noOfFilesystems;
   }
   // now, create the main list
   QString forDebugOnly = "";
   i = 0;
   j = 0;
   while ( i < noOfFilesystems ) {
      if ( temp[ 0 ][ j ] == "" || temp[ 1 ][ j ] == "proc" || temp[ 1 ][ j ] == "/dev/pts" || temp[ 0 ][ j ] == "tmpfs" ||
            temp[ 1 ][ j ] == "swap" || temp[ 0 ][ j ] == "none" || temp[ 0 ][ j ] == "proc" ||
            temp[ 0 ][ j ] == "swap" || temp[ 4 ][ j ] == "supermount" )
         ++j;
      else {
         fsData* system = new fsData();
         system->setName( temp[ 0 ][ j ] );
         system->setType( temp[ 1 ][ j ] );
         system->setMntPoint( temp[ 2 ][ j ] );
         // remove trailing spaces (since mtab removes them)
         if ( system->mntPoint() [ system->mntPoint().length() - 1 ] == '/' && system->mntPoint() != "/" )         // also skip root!
            system->setMntPoint( system->mntPoint().left( system->mntPoint().length() - 1 ) );
         system->supermount = ( temp[ 4 ][ j ] == "supermount" ? true : false );
         system->options = temp[ 3 ][ j ];
         filesystems.append( system );
         forDebugOnly = forDebugOnly + "[" + system->name() + "] on [" + system->mntPoint() +
                        "] (" + temp[ 3 ][ j ] + ")" + '\n';
         ++i;
         ++j;
         // if not supermounted, we add the mount point to our list
         if ( !system->supermount )
            mountPoints += system->mntPoint();
      }
   }
   kdDebug() << "Mt.Man: found the following:\n" << forDebugOnly << "Trying DF..." << endl;

#if defined(BSD) || defined(_OS_SOLARIS_)
   // FreeBSD problem: df does not retrive fs type.
   // Workaround: execute mount -p and merge result.

   KShellProcess proc;
   proc << KrServices::fullPathName( "mount" ) + " -p";

   // connect all outputs to collectOutput, to be displayed later
   connect( &proc, SIGNAL( receivedStdout( KProcess*, char*, int ) ),
            this, SLOT( collectOutput( KProcess*, char*, int ) ) );
   // launch
   clearOutput();
   if ( !proc.start( KProcess::Block, KProcess::Stdout ) ) {
      kdDebug() << "Unable to execute 'mount -p' !!!" << endl;
      return true;
   }

   QString str = getOutput();
   QTextStream t2( str, IO_ReadOnly );
   //kdWarning() << "debug: mount -p" << endl;
   while ( !t2.atEnd() ) {
      s = t2.readLine();
      s = s.simplifyWhiteSpace(); // remove TABs
      if ( s == QString::null || s == "" )
         continue;  // skip empty lines in fstab
      // temp[0]==name, temp[1]==type, temp[2]==mount point, temp[3]==options
      // temp[4] is reserved for special cases, right now, only for supermount
      //kdWarning() << "debug: " << s << endl;
      QString temp0 = nextWord( s, ' ' );
#ifdef _OS_SOLARIS_
      QString temp4 = nextWord( s, ' ' ); // skip '-' column
#endif

      QString temp2 = nextWord( s, ' ' );
      QString temp1 = nextWord( s, ' ' );
      QString temp3 = nextWord( s, ' ' );
      if ( temp0 == "" || temp2 == "/proc" || temp2 == "/dev/pts" ||
            temp2 == "swap" || temp0 == "none" || temp0 == "procfs" ||
            temp0 == "swap" || location( temp0 ) )
         continue;
      else {
         fsData* system = new fsData();
         system->setName( temp0 );
         system->setType( temp1 );
         system->setMntPoint( temp2 );
         system->supermount = false;
#ifndef _OS_SOLARIS_

         system->options = temp3; // unknown column on solaris
#endif

         filesystems.append( system );
         ++noOfFilesystems;
         kdWarning() << "Mt.Man: filesystem [" << temp0 << "] found by mount -p is unlisted in " << QString( FSTAB ) << endl;
      }
   }
#endif
   return true;
}

// run DF process and when it finishes, catch output with "parseDfData"
///////////////////////////////////////////////////////////////////////
void KMountMan::updateFilesystems() {
   getMtab();  // here we get the current state of mtab for watching it later
   // create the "df -P -T" process

   if ( dfStartFailed ) {
      Operational = Ready = false; // stop mountman
      return ;
   }

   tempFile = new KTempFile();
   tempFile->setAutoDelete( true );
   dfProc.clearArguments();
   dfProc << KrServices::fullPathName( "df" );
#if defined(BSD)

   dfProc << ">" << tempFile->name(); // FreeBSD: df instead of df -T -P
#elif defined(_OS_SOLARIS_)

   dfProc << "-k" << ">" << tempFile->name(); // Solaris: df -k instead of df -T -P
#else

   dfProc << "-T" << "-P" << ">" << tempFile->name();
#endif

   connect( &dfProc, SIGNAL( processExited( KProcess * ) ), this,
            SLOT( finishUpdateFilesystems() ) );
   dfProc.start( KProcess::NotifyOnExit );
   // if 'df' doesn't return in DF_WAIT_TIME seconds, stop mountman
   QTimer::singleShot( DF_WAIT_TIME * 1000, this, SLOT( killMountMan() ) );
}

// if df didn't return, stop mountman
void KMountMan::killMountMan() {
   // if Operational and !Ready, than df didn't return (yet)
   if ( Operational && !Ready ) {
      dfProc.kill( SIGKILL );    // kill the process
      Operational = Ready = false; // stop mountman
      dfStartFailed = true;   // problems at starting df
   }
}

// gets notified when df is finished.
void KMountMan::finishUpdateFilesystems() {
   // call parseDfData to work on the data
   parseDfData( tempFile->name() );
   disconnect( &dfProc, 0, 0, 0 );
   delete tempFile;
   tempFile = 0;
   emit updated();
}

fsData* KMountMan::location( QString name ) {
   fsData * it;
   for ( it = filesystems.first() ; ( it != 0 ) ; it = filesystems.next() ) {
      if ( followLink( it->name() ) == followLink( name ) )
         break;
#if defined(BSD) || defined(_OS_SOLARIS_)

      if ( name.left( 2 ) == "//" && !strcasecmp( followLink( it->name() ).local8Bit(), followLink( name ).local8Bit() ) )
         break; // FreeBSD: ignore case due to smbfs mounts
#endif

   }
   return it;
}

/* we cannot use collectOutput as other processes may connected to it */
void KMountMan::collectMtab( KProcess*, char *buffer, int buflen ) {
   // add new buffer to mtab
   for ( int i = 0; i < buflen; ++i )
      mtab += buffer[ i ];
}

QString KMountMan::getMtab() {
   KShellProcess proc;
   proc << KrServices::fullPathName( "mount" );

   // connect all outputs by collectMtab ( collectOutput can not be used because of the watcher )
   connect( &proc, SIGNAL( receivedStdout( KProcess*, char*, int ) ),
            this, SLOT( collectMtab( KProcess*, char*, int ) ) );
   // launch
   mtab = "";
   if ( !proc.start( KProcess::Block, KProcess::Stdout ) ) {
      kdDebug() << "Unable to execute 'mount' !!!" << endl;
      return "";
   }

   return mtab;
}

bool KMountMan::checkMtabChanged() {
   QString LastMtab = mtab;
   return getMtab() != LastMtab;
}

QString KMountMan::devFromMtab( QString mntPoint ) {
   QString mtab = getMtab();

   // and read it into the temporary array
   QTextStream t( &mtab, IO_ReadOnly );
   while ( !t.atEnd() ) {
      QString dev, point;
      QString s = t.readLine().simplifyWhiteSpace();
      dev = nextWord( s, ' ' );   /* device */
      nextWord( s, ' ' );         /* on */
      point = nextWord( s, ' ' ); /* mountpoint */
      if ( point == mntPoint ) {
         return dev;
      }
   }
   return QString::null;
}

QString KMountMan::pointFromMtab( QString device ) {
   QString mtab = getMtab();

   // and read it into the temporary array
   QTextStream t( &mtab, IO_ReadOnly );
   while ( !t.atEnd() ) {
      QString dev, mntPoint;
      QString s = t.readLine().simplifyWhiteSpace();
      dev = nextWord( s, ' ' );   /* device */
      nextWord( s, ' ' );         /* on */
      mntPoint = nextWord( s, ' ' ); /* mountpoint */
      if ( dev == device ) {
         return mntPoint;
      }
   }
   return QString::null;
}

KMountMan::mntStatus KMountMan::getStatus( QString mntPoint ) {
   // parse the mount table and search for mntPoint
   bool mounted = ( devFromMtab( mntPoint ) != QString::null );
   if ( mounted )
      return KMountMan::MOUNTED;
   if ( mountPoints.findIndex( mntPoint ) == -1 )          // no such mountPoint is /etc/fstab
      return KMountMan::DOESNT_EXIST;
   else
      return KMountMan::NOT_MOUNTED;
}

QString KMountMan::getDevice( QString mntPoint ) {
   fsData * it;
   for ( it = filesystems.first() ; ( it != 0 ) ; it = filesystems.next() )
      if ( it->mntPoint() == mntPoint )
         return it->name();
   // if we got here, the mntPoint doesn't exist in the database
   return QString::null;
}

// parseDfData assumes that DF always knows better than FSTAB about currently
// mounted filesystems.
/////////////////////////////////////////////////////////////////////////////
void KMountMan::parseDfData( QString filename ) {
   QString temp;
   QFile f( filename );
   if ( !f.open( IO_ReadOnly ) || dfStartFailed ) { // error reading temp file
      Operational = false;
      Ready = false;
      return ;                   // make mt.man non-operational
   }
   QTextStream t( &f );        // use a text stream
   QString s, s2;
   s = t.readLine();  // read the 1st line - it's trash for us
   // now that we have a QString containing all the output of DF, let's get to work.
   int countFilesystems = 0; // sucessfully found filesystems
   //kdWarning() << "debug: parseDfData" << endl;
   while ( !t.eof() ) {
      bool newFS = false;
      s2 = s = t.readLine();  // this is the important one!
      //kdWarning() << "debug: " << s << endl;
      temp = nextWord( s, ' ' );
      // avoid adding unwanted filesystems to the list
      if ( temp == "tmpfs" )
         continue;
#if defined(BSD)

      if ( temp == "procfs" )
         continue; // FreeBSD: ignore procfs too
#elif defined(_OS_SOLARIS_)

      if ( temp == "proc" || temp == "swap" )
         continue; // Solaris: ignore procfs too
#endif

      temp = followLink( temp );  // make sure DF gives us the true device and not a link
      fsData* loc = location( temp ); // where is the filesystem located in our list?
      if ( loc == 0 ) {
         kdWarning() << "Mt.Man: filesystem [" << temp << "] found by DF is unlisted in " << QString( FSTAB ) << endl;
         loc = new fsData();
         loc->supermount = false;
         filesystems.append( loc );
         if ( temp.contains( "//" ) > 0 ||            // if it contains '//', it's a smb share
               temp.contains( ":" ) > 0 ||            // if it contains ':' , it's an nfs share
               temp.startsWith( "/" ) )              // if it is a fullpath device name
            loc->setName( temp );      // <patch> thanks to Cristi Dumitrescu
         else
            loc->setName( "/dev/" + temp );
         newFS = true;
      }
#if !defined(BSD) && !defined(_OS_SOLARIS_)
      temp = nextWord( s, ' ' );   // catch the TYPE
      // is it supermounted ?
      if ( temp == "supermount" )
         loc->supermount = true;
      loc->setType( temp );
      if ( loc->type() != temp ) {
         kdWarning() << "Mt.Man: according to DF, filesystem [" << loc->name() <<
         "] has a different type from what's stated in " << QString( FSTAB ) << endl;
         loc->setType( temp );  // DF knows best
      }
#endif
      temp = nextWord( s, ' ' );
      loc->setTotalBlks( temp.toLong() );
      temp = nextWord( s, ' ' );
      temp = nextWord( s, ' ' );   // get rid of the next 2 words
      loc->setFreeBlks( temp.toLong() );
      temp = nextWord( s, ' ' );
      temp = nextWord( s, '\n' );  // read the "mounted on" thing
      if ( loc->mntPoint() != temp ) {
         if ( !newFS )
            kdWarning() << "Mt.Man: according to DF, filesystem [" << loc->name() <<
            "] is mounted on " << temp << " and not on " << loc->mntPoint() << endl;
         loc->setMntPoint( temp );  // DF knows best
      }
      loc->setMounted( true );
      ++countFilesystems;
   }

   Operational = Ready = true; // we are finished here
   if ( countFilesystems > 0 )
      kdDebug() << "Mt.Man: Alive and kicking. " << endl;
   else {
      kdWarning() << "Mt.Man: failed running DF. Shutting down (sorry)." << endl;
      Operational = Ready = false;
   }
}

void KMountMan::forceUpdate() {
   kdWarning() << "Mt.Man: Born, looking around to get familiar." << endl;
   mountPoints.clear();
   // first create our list from /etc/fstab
   if ( !createFilesystems() ) {  // create the potential fs from /etc/fstab
      Operational = Ready = false;
      return ;										 // if something went wrong, bail out!
   } else
      Operational = true;     // mountman is alive but not yet ready
   updateFilesystems();         // use the output of "DF -T -P" to update data
}

void KMountMan::collectOutput( KProcess *p, char *buffer, int buflen ) {
   if ( !p )
      return ; // don't collect data from unknown/undefined processes
   if ( outputBuffer == 0 )
      outputBuffer = new QString();  // create buffer if needed

   // add new buffer to the main output buffer
   for ( int i = 0; i < buflen; ++i )
      ( *outputBuffer ) += buffer[ i ];
}

void KMountMan::clearOutput() {
   if ( outputBuffer != 0 ) {
      delete outputBuffer;
      outputBuffer = 0;
   }
}

// this is the generic version of mount - it receives a mntPoint and try to
// mount it the usual way - via /etc/fstab
void KMountMan::mount( QString mntPoint ) {
   if ( mountPoints.findIndex( mntPoint ) == -1 )
      return ; // safety measure

   if ( !KrServices::cmdExist( "mount" ) ) {
      KMessageBox::error( 0,
                          i18n( "Can't start 'mount'! Check the 'Dependencies' page in konfigurator." ) );
      return ;
   }

   KProcess mountProc;
   mountProc << KrServices::fullPathName( "mount" ) << mntPoint.local8Bit();
   // connect all outputs to collectOutput, to be displayed later
   connect( &mountProc, SIGNAL( receivedStderr( KProcess*, char*, int ) ),
            this, SLOT( collectOutput( KProcess*, char*, int ) ) );
   // launch
   clearOutput();
   if ( !mountProc.start( KProcess::Block, KProcess::Stderr ) ) {
      KMessageBox::error( 0,
                          i18n( "Unable to execute 'mount' !!!\ncheck that /bin/mount or /sbin/mount are availble" ) );
      return ;
   }
   if ( mountProc.normalExit() )
      if ( mountProc.exitStatus() == 0 )
         return ; // incase of a normal exit
   // on any other case,report an error
   KMessageBox::sorry( 0, i18n( "Unable to complete the mount." ) +
                       i18n( "The error reported was:\n\n" ) + getOutput() );
}

// this version of mount is used strictly by the GUI
// don't call it explicitly !
void KMountMan::mount( fsData *p ) {
   // first, if the user isn't ROOT, he won't be able to mount it, unless
   // the option states 'user'. if so, we'll mount the mntPoint
   if ( p->options.contains( "user" ) && getuid() != 0 ) {
      mount( p->mntPoint() ); // call the alternative method
      return ;
   }

   if ( !KrServices::cmdExist( "mount" ) ) {
      KMessageBox::error( 0,
                          i18n( "Can't start 'mount'! Check the 'Dependencies' page in konfigurator." ) );
      return ;
   }

   bool ro = ( p->type() == "iso9660" );
   KProcess mountProc;
   mountProc << KrServices::fullPathName( "mount" );
   if ( ro )
      mountProc << "-r";                // read only
   mountProc << "-t" << p->type().local8Bit();  // local8Bit == normal ascii
   if ( !p->options.isEmpty() )
      mountProc << "-o" << p->options; // -o options
   mountProc << p->name().local8Bit() << p->mntPoint().local8Bit();

   // don't allow mounting 'supermount' filesystems
   if ( p->supermount ) {
      KMessageBox::information( mountManGui, i18n( "Warning: you're trying to mount a 'supermount' filesystem. Supermount filesystems are (un)mounted automatically by linux upon insert/eject. This is usually a Linux-Mandrake feature.Krusader will not allow this, as it creates unpredictable behaviour." ), i18n( "Error" ), "SupermountWarning" );
      return ;
   }
   // connect all outputs to collectOutput, to be displayed later
   connect( &mountProc, SIGNAL( receivedStderr( KProcess*, char*, int ) ),
            this, SLOT( collectOutput( KProcess*, char*, int ) ) );
   // launch
   clearOutput();
   if ( !mountProc.start( KProcess::Block, KProcess::Stderr ) ) {
      KMessageBox::error( 0,
                          i18n( "Unable to execute 'mount' !!! check that /bin/mount or /sbin/mount are availble" ) );
      return ;
   }
   if ( mountProc.normalExit() )
      if ( mountProc.exitStatus() == 0 )
         return ; // incase of a normal exit
   // on any other case,report an error
   KMessageBox::sorry( 0, i18n( "Unable to complete the mount." ) +
                       i18n( "The error reported was:\n\n" ) + getOutput() );
}

// this version of unmount is generic - it doesn't care which device is mounted
// on the mntPoint, it simply unmounts it --- used with externally mounted systems
void KMountMan::unmount( QString mntPoint ) {
   fsData * tmp = new fsData();
   tmp->setMntPoint( mntPoint ); // mount-point and supermount=false is all that's
   tmp->supermount = false;     // needed to normally unmount
   unmount( tmp );
   delete tmp;                // no memory-leaks here !
}

// this version is used strictly by the GUI or by the unmount(QString) version
// you've got no need to call it explicitly
void KMountMan::unmount( fsData *p ) {
   if ( !KrServices::cmdExist( "umount" ) ) {
      KMessageBox::error( 0,
                          i18n( "Can't start 'umount'! Check the 'Dependencies' page in konfigurator." ) );
      return ;
   }

   KProcess umountProc;
   umountProc << KrServices::fullPathName( "umount" );
   umountProc << p->mntPoint().local8Bit();

   // don't allow unmounting 'supermount' filesystems
   if ( p->supermount ) {
      KMessageBox::information( mountManGui, i18n( "Warning: you're trying to unmount a 'supermount' filesystem. Supermount filesystems are (un)mounted automatically by linux upon insert/eject. This is usually a Linux-Mandrake feature. Krusader will not allow this, as it creates unpredictable behaviour." ), i18n( "Error" ), "SupermountWarning" );
      return ;
   }
   // connect outputs to collectOutput, to be displayed later
   connect( &umountProc, SIGNAL( receivedStderr( KProcess*, char*, int ) ),
            this, SLOT( collectOutput( KProcess*, char*, int ) ) );
   // launch
   krApp->startWaiting( i18n( "Unmounting device, please wait ..." ), 0, false );

   clearOutput();
   if ( !umountProc.start( KProcess::Block, KProcess::Stderr ) ) {
      KMessageBox::error( 0,
                          i18n( "Unable to execute 'umount' !!! check that /bin/umount or /sbin/umount are availble" ) );
      return ;
   } else
      while ( umountProc.isRunning() )
         qApp->processEvents();
   krApp->stopWait();
   if ( umountProc.normalExit() )
      if ( umountProc.exitStatus() == 0 )
         return ; // incase of a normal exit
   // on any other case,report an error
   KMessageBox::sorry( 0, i18n( "Unable to complete the un-mount." ) +
                       i18n( "The error reported was:\n\n" ) + getOutput() );
}

void KMountMan::toggleMount( QString device ) {
   fsData * p = location( device );
   // request mountMan to (un)mount something, if he does his job (and he does)
   // we will be notified by signal
   ( p->mounted() ? unmount( p ) : mount( p ) );
}

void KMountMan::autoMount( QString path ) {
   if ( getStatus( path ) == NOT_MOUNTED )
      mount( path );
}

void KMountMan::eject( QString mntPoint ) {
   KShellProcess proc;
   proc << KrServices::fullPathName( "eject" ) << "\"" + mntPoint + "\"";
   proc.start( KProcess::Block );
   if ( !proc.normalExit() || proc.exitStatus() != 0 )         // if we failed with eject
      KMessageBox::information( 0, i18n( "Error ejecting device ! You need to have 'eject' in your path." ), i18n( "Error" ), "CantExecuteEjectWarning" );
}


// returns true if the path is an ejectable mount point (at the moment CDROM)
bool KMountMan::ejectable( QString path ) {
#if !defined(BSD) && !defined(_OS_SOLARIS_)
   fsData * it;
   for ( it = filesystems.first() ; ( it != 0 ) ; it = filesystems.next() )
      if ( it->mntPoint() == path &&
            ( it->type() == "iso9660" || followLink( it->name() ).left( 2 ) == "cd" ) )
         return KrServices::cmdExist( "eject" );
#endif

   return false;
}

// a mountMan special version of KIO::convertSize, which deals
// with large filesystems ==> >4GB, it actually recieve size in
// a minimum block of 1024 ==> data is KB not bytes
QString KMountMan::convertSize( KIO::filesize_t size ) {
   float fsize;
   QString s;
   // Tera-byte
   if ( size >= 1073741824 ) {
      fsize = ( float ) size / ( float ) 1073741824;
      if ( fsize > 1024 )         // no name for something bigger than tera byte
         // let's call it Zega-Byte, who'll ever find out? :-)
         s = i18n( "%1 ZB" ).arg( KGlobal::locale() ->formatNumber( fsize / ( float ) 1024, 1 ) );
      else
         s = i18n( "%1 TB" ).arg( KGlobal::locale() ->formatNumber( fsize, 1 ) );
   }
   // Giga-byte
   else if ( size >= 1048576 ) {
      fsize = ( float ) size / ( float ) 1048576;
      s = i18n( "%1 GB" ).arg( KGlobal::locale() ->formatNumber( fsize, 1 ) );
   }
   // Mega-byte
   else if ( size > 1024 ) {
      fsize = ( float ) size / ( float ) 1024;
      s = i18n( "%1 MB" ).arg( KGlobal::locale() ->formatNumber( fsize, 1 ) );
   }
   // Kilo-byte
   else {
      fsize = ( float ) size;
      s = i18n( "%1 KB" ).arg( KGlobal::locale() ->formatNumber( fsize, 0 ) );
   }
   return s;
}

// populate the pop-up menu of the mountman tool-button with actions
void KMountMan::quickList() {
   if ( !Operational ) {
      KMessageBox::error( 0, i18n( "MountMan is not operational. Sorry" ) );
      return ;
   }

#if KDE_IS_VERSION(3,2,0)

   // clear the popup menu
   ( ( KToolBarPopupAction* ) krMountMan ) ->popupMenu() ->clear();

   // create lists of current and possible mount points
   KMountPoint::List current = KMountPoint::currentMountPoints();
   KMountPoint::List possible = KMountPoint::possibleMountPoints();

   // create a popupmenu, displaying mountpoints with possible actions
   // also, populate a small array with the actions
   if ( _actions )
      delete[] _actions;
   _actions = new QString[ possible.size() ];

   KMountPoint::List::iterator it;
   KMountPoint *m;
   int idx;
   for ( it = possible.begin(), idx = 0; it != possible.end(); ++it, ++idx ) {
      m = *it;
      // does the mountpoint exist in current list? if so, it can only
      // be umounted, otherwise, it can be mounted
      bool needUmount = false;
      KMountPoint::List::iterator otherIt;
      for ( otherIt = current.begin(); otherIt != current.end(); ++otherIt ) {
         if ( ( *otherIt ) ->mountPoint() == m->mountPoint() ) { // found it, in needs umount
            needUmount = true;
            break;
         }
      }
      // add the item to the menu
      _actions[ idx ] = QString( needUmount ? "_U_" : "_M_" ) + m->mountPoint();
      QString text = QString( ( needUmount ? i18n( "Unmount" ) : i18n( "Mount" ) ) ) + " " + m->mountPoint() +
                     " (" + m->mountedFrom() + ")";


      ( ( KToolBarPopupAction* ) krMountMan ) ->popupMenu() ->insertItem( text, idx );
   }
   connect( ( ( KToolBarPopupAction* ) krMountMan ) ->popupMenu(), SIGNAL( activated( int ) ),
            this, SLOT( performAction( int ) ) );

#endif /* KDE 3.2 */

}

void KMountMan::performAction( int idx ) {

#if KDE_IS_VERSION(3,2,0)

   while ( qApp->hasPendingEvents() )
      qApp->processEvents();

   if ( idx < 0 )
      return ;
   bool domount = _actions[ idx ].left( 3 ) == "_M_";
   QString mountPoint = _actions[ idx ].mid( 3 );
   if ( !domount ) { // umount
      unmount( mountPoint );
   } else { // mount
      mount( mountPoint );
   }

   // free memory
   delete[] _actions;
   _actions = 0L;
   disconnect( ( ( KToolBarPopupAction* ) krMountMan ) ->popupMenu(), SIGNAL( activated( int ) ), 0, 0 );

#endif /* KDE 3.2 */

}

