/***************************************************************************
                                 krarc.cpp
                             -------------------
    begin                : Sat Jun 14 14:42:49 IDT 2003
    copyright            : (C) 2003 by Rafi Yanai & Shie Erlich
    email                : krusader@users.sf.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qregexp.h>

#include <kfileitem.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kinstance.h>
#include <klocale.h>
#include <kurl.h>
#include <kprocess.h>
#include <ktempfile.h>
#include <klargefile.h>
#include <kstandarddirs.h>
#include <kio/job.h>

#include <iostream>
#include "krarc.h"

#define MAX_IPC_SIZE (1024*32)

#if 0
#define KRDEBUG(X...) do{   \
	QFile f("/tmp/debug");    \
	f.open(IO_WriteOnly | IO_Append);     \
	QTextStream stream( &f ); \
  stream <<__FUNCTION__<<"(" <<__LINE__<<"): "; \
  stream << X << endl;      \
	f.close();                \
} while(0);
#else
#define KRDEBUG(X...)
#endif

using namespace KIO;
extern "C" {

int kdemain( int argc, char **argv ){
	KInstance instance( "kio_krarc" );

	if (argc != 4){
		kdWarning() << "Usage: kio_krarc  protocol domain-socket1 domain-socket2" << endl;
		exit(-1);
	}
    
	kio_krarcProtocol slave(argv[2], argv[3]);
	slave.dispatchLoop();

	return 0;
}

} // extern "C" 

kio_krarcProtocol::kio_krarcProtocol(const QCString &pool_socket, const QCString &app_socket)
 : SlaveBase("kio_krarc", pool_socket, app_socket), archiveChanged(true), arcFile(0L),cpioReady(false),
   password(QString::null) {
  dirDict.setAutoDelete(true);

  arcTempDir = locateLocal("tmp",QString::null);
	QString dirName = "krArc"+QDateTime::currentDateTime().toString(Qt::ISODate);
  dirName.replace(QRegExp(":"),"_");
  QDir(arcTempDir).mkdir(dirName);
	arcTempDir = arcTempDir+dirName+"/";
}

/* ---------------------------------------------------------------------------------- */
kio_krarcProtocol::~kio_krarcProtocol(){
	// delete the temp directory
	KShellProcess proc;
	proc << "rm -rf "<< arcTempDir;
	proc.start(KProcess::Block);
}

/* ---------------------------------------------------------------------------------- */
void kio_krarcProtocol::receivedData(KProcess*,char* buf,int len){
	QByteArray d(len);
	d.setRawData(buf,len);
	data(d);
	d.resetRawData(buf,len);
	processedSize(len);	
}

void kio_krarcProtocol::mkdir(const KURL& url,int permissions){
  KRDEBUG(url.path());
  setArcFile(url.path());
  if( putCmd.isEmpty() ){
    error(ERR_UNSUPPORTED_ACTION,
    i18n("Creating directories is not supported with %1 archives").arg(arcType) );
    return;
  }

  //QString tmpDir = arcTempDir+url.path();
  QString arcDir  = findArcDirectory(url);
  QString tmpDir = arcTempDir + arcDir.mid(1) + url.path().mid(url.path().findRev("/")+1);
  if( tmpDir.right(1) != "/" ) tmpDir = tmpDir+"/";
    
  if( permissions == -1 ) permissions = 0777; //set default permissions
  for( unsigned int i=arcTempDir.length();i<tmpDir.length(); i=tmpDir.find("/",i+1)){
    ::mkdir(tmpDir.left(i).local8Bit(),permissions);
  }

  if( tmpDir.endsWith( "/" ) )
    tmpDir.truncate( tmpDir.length() - 1 );
       
	// pack the directory
	KShellProcess proc;
	proc << putCmd << "\""+arcFile->url().path()+"\" " << "\""+tmpDir.mid(arcTempDir.length())+"\"";
  infoMessage(i18n("Creating %1 ...").arg( url.fileName() ) );
	QDir::setCurrent(arcTempDir);
	proc.start(KProcess::Block);

  // delete the temp directory
  QDir().rmdir(arcTempDir);
	//  force a refresh of archive information
  initDirDict(url,true);
	finished();
}

void kio_krarcProtocol::put(const KURL& url,int permissions,bool overwrite,bool resume){
  KRDEBUG(url.path());
  setArcFile(url.path());
  if( putCmd.isEmpty() ){
    error(ERR_UNSUPPORTED_ACTION,
    i18n("Writing to %1 archives is not supported").arg(arcType) );
    return;
  } 
	if( !overwrite && findFileEntry(url) ){
		error( ERR_FILE_ALREADY_EXIST,url.path() );
		return;
	}

	QString arcDir  = findArcDirectory(url);
  QString tmpFile = arcTempDir + arcDir.mid(1) + url.path().mid(url.path().findRev("/")+1);

	QString tmpDir = arcTempDir+arcDir.mid(1)+"/";
  for( unsigned int i=arcTempDir.length();i<tmpDir.length(); i=tmpDir.find("/",i+1)){
		QDir("/").mkdir(tmpDir.left(i));
	}
	int fd;
	if ( resume ) {
		fd = KDE_open( tmpFile.local8Bit(), O_RDWR );  // append if resuming
		KDE_lseek(fd, 0, SEEK_END); // Seek to end
	} else {
		// WABA: Make sure that we keep writing permissions ourselves,
		// otherwise we can be in for a surprise on NFS.
		mode_t initialMode;
		if ( permissions != -1)
			initialMode = permissions | S_IWUSR | S_IRUSR;
		else
			initialMode = 0666;

		fd = KDE_open(tmpFile.local8Bit(), O_CREAT | O_TRUNC | O_WRONLY, initialMode);
	}
  QByteArray buffer;
	int readResult;
	do{
		dataReq();
		readResult = readData(buffer);
		write(fd,buffer.data(),buffer.size());
 	} while( readResult > 0 );
	close(fd);
	// pack the file
	KShellProcess proc;
	proc << putCmd << "\""+arcFile->url().path()+"\" " << "\""+tmpFile.mid(arcTempDir.length())+"\"";
  infoMessage(i18n("Packing %1 ...").arg( url.fileName() ) );
  QDir::setCurrent(arcTempDir);
  proc.start(KProcess::Block);
  // remove the file
	QFile::remove(tmpFile);
	//  force a refresh of archive information
  initDirDict(url,true);
	finished();
}

void kio_krarcProtocol::get(const KURL& url ){
  KRDEBUG(url.path());
  if( !setArcFile(url.path()) ) return;
  if( getCmd.isEmpty() ){
    error(ERR_UNSUPPORTED_ACTION,
    i18n("Retrieving data from %1 archives is not supported").arg(arcType) );
    return;
  } 
	UDSEntry* entry = findFileEntry(url);
	if( !entry ){
		error(KIO::ERR_DOES_NOT_EXIST,url.path());
		return;
	}
	if(KFileItem(*entry,url).isDir()){
  	error(KIO::ERR_IS_DIRECTORY,url.path());
		return;
	}
  // for RPM files extract the cpio file first
  if( !cpioReady && arcType == "rpm"){
    KShellProcess cpio;
    cpio << "rpm2cpio" << arcPath << " > " << arcTempDir+"contents.cpio";
    cpio.start(KProcess::Block);
    cpioReady = true;
  }
  // Use the external unpacker to unpack the file
  QString file = url.path().mid(arcFile->url().path().length()+1);
	KShellProcess proc;
  if( cpioReady ){
    proc << getCmd << arcTempDir+"contents.cpio " << "\"*"+file+"\"";
  } else {
    // Determine the mimetype of the file to be retrieved, and emit it.
    // This is mandatory in all slaves (for KRun/BrowserRun to work).
    KMimeType::Ptr mt = KMimeType::findByURL( arcTempDir+file, 0, false /* NOT local URL */ );
    emit mimeType( mt->name() );
	  proc << getCmd << "\""+arcFile->url().path()+"\" " << "\""+file+"\"";
    connect(&proc,SIGNAL(receivedStdout(KProcess*,char*,int)),
           this,SLOT(receivedData(KProcess*,char*,int)) );
  }
  infoMessage(i18n("Unpacking %1 ...").arg( url.fileName() ) );
  // change the working directory to our arcTempDir
  QDir::setCurrent(arcTempDir);
  proc.start(KProcess::Block,KProcess::AllOutput);
  if( cpioReady ){
    // the follwing block is ripped from KDE file KIO::Slave
    // $Id$
    QCString _path( QFile::encodeName(arcTempDir+file) );
    KDE_struct_stat buff;
    if( KDE_lstat( _path.data(), &buff ) == -1 ) {
      if ( errno == EACCES )
        error( KIO::ERR_ACCESS_DENIED, url.path() );
      else
        error( KIO::ERR_DOES_NOT_EXIST, url.path() );
      return;
    }
    if ( S_ISDIR( buff.st_mode ) ) {
      error( KIO::ERR_IS_DIRECTORY, url.path() );
      return;
    }
    if ( !S_ISREG(buff.st_mode) ) {
      error( KIO::ERR_CANNOT_OPEN_FOR_READING, url.path() );
      return;
    }
    int fd = KDE_open( _path.data(), O_RDONLY );
    if ( fd < 0 ) {
      error( KIO::ERR_CANNOT_OPEN_FOR_READING, url.path() );
      return;
    }
    // Determine the mimetype of the file to be retrieved, and emit it.
    // This is mandatory in all slaves (for KRun/BrowserRun to work).
    KMimeType::Ptr mt = KMimeType::findByURL( arcTempDir+file, buff.st_mode, true /* local URL */ );
    emit mimeType( mt->name() );

    KIO::filesize_t processed_size = 0;

    QString resumeOffset = metaData("resume");
    if ( !resumeOffset.isEmpty() ){
      bool ok;
#if QT_VERSION >= 0x030200
      KIO::fileoffset_t offset = resumeOffset.toLongLong(&ok);
#else
      KIO::fileoffset_t offset = resumeOffset.toULong(&ok);
#endif
      if (ok && (offset > 0) && (offset < buff.st_size)){
        if (KDE_lseek(fd, offset, SEEK_SET) == offset){
          canResume ();
          processed_size = offset;
        }
      }
    }

    totalSize( buff.st_size );

    char buffer[ MAX_IPC_SIZE ];
    QByteArray array;
    while( 1 ){
      int n = ::read( fd, buffer, MAX_IPC_SIZE );
      if (n == -1){
        if (errno == EINTR)
        continue;
        error( KIO::ERR_COULD_NOT_READ, url.path());
        close(fd);
        return;
      }
      if (n == 0)
      break; // Finished

      array.setRawData(buffer, n);
      data( array );
      array.resetRawData(buffer, n);

      processed_size += n;
    }

    data( QByteArray() );
    close( fd );
    processedSize( buff.st_size );
    finished();
    return;
  }
  // send empty buffer to mark EOF
  data(QByteArray());
  finished();
}

void kio_krarcProtocol::del(KURL const & url, bool isFile){
  KRDEBUG(url.path());
  setArcFile(url.path());
  if( delCmd.isEmpty() ){
    error(ERR_UNSUPPORTED_ACTION,
    i18n("Deleting files from %1 archives is not supported").arg(arcType) );
    return;
  }
	if( !findFileEntry(url) ){
		error(KIO::ERR_DOES_NOT_EXIST,url.path());
		return;
	}

	QString file = url.path().mid(arcFile->url().path().length()+1);
 if( !isFile && file.right(1) != "/" ) {
    if(arcType == "zip") file = file + "/";
  }
  KShellProcess proc;
	proc << delCmd << "\""+arcFile->url().path()+"\" " << "\""+file+"\"";
	infoMessage(i18n("Deleting %1 ...").arg( url.fileName() ) );
	proc.start(KProcess::Block);
	//  force a refresh of archive information
  initDirDict(url,true);
	finished();
}

void kio_krarcProtocol::stat( const KURL & url ){  
  KRDEBUG(url.path());
  if( !setArcFile(url.path()) ) return;
  if( listCmd.isEmpty() ){
    error(ERR_UNSUPPORTED_ACTION,
    i18n("Accessing files is not supported with the %1 archives").arg(arcType) );
    return;
  }    
  QString path = url.path(-1);
	KURL newUrl = url;
	
  // but treat the archive itself as the archive root
	if( path == arcFile->url().path(-1) ){
    newUrl.setPath(path+"/");
		path = newUrl.path();
	}
	// we might be stating a real file
	if( QFileInfo(path).exists() ){
		KDE_struct_stat buff;
    KDE_stat( path.local8Bit(), &buff );
		QString mime = KMimeType::findByPath(path,buff.st_mode)->name();
		statEntry(KFileItem(path,mime,buff.st_mode).entry());
    finished();
		return;
	}
  UDSEntry* entry = findFileEntry(newUrl);
	if( entry ){
    statEntry( *entry );
		finished();
	}
	else error(KIO::ERR_DOES_NOT_EXIST,path);
}

void kio_krarcProtocol::listDir(const KURL& url){
  KRDEBUG(url.path());
  if( !setArcFile(url.path()) ) return;
  if( listCmd.isEmpty() ){
    error(ERR_UNSUPPORTED_ACTION,
    i18n("Listing directories is not supported for %1 archives").arg(arcType) );
    return;
  }  
	QString path = url.path();
	if( path.right(1) != "/" ) path = path+"/";

	// it might be a real dir !
	if( QFileInfo(path).exists() ){
    if( QFileInfo(path).isDir() ){
			KURL redir;
			redir.setPath( url.path() );
    	redirection(redir);
			finished();
		}
		else { // maybe it's an archive !
			error(ERR_IS_FILE,path);
		}
		return;
	}
  if( !initDirDict(url) ){ 
		error(ERR_CANNOT_ENTER_DIRECTORY,url.path());
    return;
	}
 	QString arcDir = path.mid(arcFile->url().path().length());
	arcDir.truncate(arcDir.findRev("/"));
	if(arcDir.right(1) != "/") arcDir = arcDir+"/";

	UDSEntryList* dirList = dirDict.find(arcDir);
	if( dirList == 0 ){
		error(ERR_CANNOT_ENTER_DIRECTORY,url.path());
		return;
	}
	totalSize(dirList->size());
  listEntries(*dirList);
	finished();
}

bool kio_krarcProtocol::setArcFile(const QString& path){
	time_t currTime = time( 0 );
	archiveChanged = true;
	// is the file already set ?
	if( arcFile && arcFile->url().path(-1) == path.left(arcFile->url().path(-1).length()) ){
		// Has it changed ?
		KFileItem* newArcFile = new KFileItem(arcFile->url(),QString::null,0);
		if( !newArcFile->cmp( *arcFile ) ){
			delete arcFile;
			password = QString::null;
			arcFile = newArcFile;
		}
		else { // same old file
			delete newArcFile;
			archiveChanged = false;
		}
	}
	else {// it's a new file...
		if( arcFile ){
			delete arcFile;
      password = QString::null;
			arcFile = 0L;
		}
		QString newPath = path;
		if(newPath.right(1) != "/") newPath = newPath+"/";
		for(int pos=0; pos >= 0; pos = newPath.find("/",pos+1)){
			QFileInfo qfi(newPath.left(pos));
			if( qfi.exists() && !qfi.isDir() ){
    		arcFile = new KFileItem(newPath.left(pos),QString::null,0);
				break;
			}
		}
		if( !arcFile ){
       error( ERR_DOES_NOT_EXIST,path );
       return false; // file not found
    }
	}
  
  /* FIX: file change can only be detected if the timestamp between the two consequent 
     changes is more than 1s. If the archive is continuously changing (check: move files 
     inside the archive), krarc may erronously think, that the archive file is unchanged, 
     because the timestamp is the same as the previous one. This situation can only occur
     if the modification time equals with the current time. While this condition is true,
     we can say, that the archive is changing, so content reread is always necessary 
     during that period. */
  if( archiveChanging )
    archiveChanged = true;
  archiveChanging = ( currTime == arcFile->time( UDS_MODIFICATION_TIME ) );
  
  arcType = arcFile->mimetype();
  arcType = arcType.mid(arcType.findRev("-")+1);
  arcPath = "\""+arcFile->url().path(-1)+"\"";
  arcPath.replace(QRegExp(" "),"\\ ");
  return initArcParameters();
}

bool kio_krarcProtocol::initDirDict(const KURL&url, bool forced){
	// set the archive location
	//if( !setArcFile(url.path()) ) return false;
	// no need to rescan the archive if it's not changed
	if( !archiveChanged && !forced ) return true;

	setArcFile( url.path() );   /* if the archive was changed refresh the file information */
	
	// write the temp file
	KShellProcess proc;
	KTempFile temp("krarc","tmp");
	temp.setAutoDelete(true);
	if( arcType != "bzip2" ){
		proc << listCmd << "\""+arcFile->url().path(-1)+"\"" <<" > " << temp.name();
		proc.start(KProcess::Block);
		if( !proc.normalExit() || !proc.exitStatus() == 0 )	return false;
  }
	// clear the dir dictionary
	dirDict.clear();

	// add the "/" directory
	UDSEntryList* root = new UDSEntryList();
	dirDict.insert("/",root);
	// and the "/" UDSEntry
	UDSEntry entry;
	UDSAtom atom;
	atom.m_uds = UDS_NAME;
	atom.m_str = ".";
	entry.append(atom);
  mode_t mode = parsePermString("drwxr-xr-x");
  atom.m_uds = UDS_FILE_TYPE;
  atom.m_long = mode & S_IFMT; // keep file type only
  entry.append( atom );
  atom.m_uds = UDS_ACCESS;
  atom.m_long = mode & 07777; // keep permissions only
  entry.append( atom );

	root->append(entry);

	if( arcType == "bzip2" ){
		KRDEBUG("Got me here...");
		parseLine(0,"",temp.file());
		return true;
	}

	// parse the temp file
	temp.file()->open(IO_ReadOnly);
	char buf[1000];
	QString line;

	int lineNo = 0;
  // the rar list is started with a ------ line.
  if(arcType == "rar"){
    while(temp.file()->readLine(buf,1000) != -1){
      line = QString::fromLocal8Bit(buf);
      if( line.startsWith("-----------") ) break;
    }
  } 
	while(temp.file()->readLine(buf,1000) != -1){
		line = QString::fromLocal8Bit(buf);
    if( arcType == "rar" ) {
       // the rar list is ended with a ------ line.
       if( line.startsWith("-----------") ) break;
       else{
         temp.file()->readLine(buf,1000);
         line = line+QString::fromLocal8Bit(buf);
       }
    }
    parseLine(lineNo++,line.stripWhiteSpace(),temp.file());
	}
  // close and delete our file
	temp.file()->close();

	archiveChanged = false;
	return true;
}

QString kio_krarcProtocol::findArcDirectory(const KURL& url){
	QString path = url.path();
	if( path.right(1) == "/" ) path.truncate(path.length()-1);

  if( !initDirDict(url) ){
		return QString::null;
	}
 	QString arcDir = path.mid(arcFile->url().path().length());
	arcDir.truncate(arcDir.findRev("/"));
	if(arcDir.right(1) != "/") arcDir = arcDir+"/";

	return arcDir;
}

UDSEntry* kio_krarcProtocol::findFileEntry(const KURL& url){
  QString arcDir = findArcDirectory(url);
	if( arcDir.isEmpty() ) return 0;

	UDSEntryList* dirList = dirDict.find(arcDir);
	if( !dirList ){
		return 0;
	}
	QString name = url.path();
	if( arcFile->url().path(-1) == url.path(-1) ) name = "."; // the "/" case
	else{
		if( name.right(1) == "/" ) name.truncate(name.length()-1);
		name = name.mid(name.findRev("/")+1);
	}

  UDSEntryList::iterator entry;
	UDSEntry::iterator atom;

	for ( entry = dirList->begin(); entry != dirList->end(); ++entry ){
    for( atom = (*entry).begin(); atom != (*entry).end(); ++atom ){
      if( (*atom).m_uds == UDS_NAME ){
				if((*atom).m_str == name){
          return &(*entry);
				}
				else break;
			}
		}
	}
	return 0;

}

QString kio_krarcProtocol::nextWord(QString &s,char d) {
  s=s.stripWhiteSpace();
  int j=s.find(d,0);
  QString temp=s.left(j); // find the leftmost word.
  s.remove(0,j);
  return temp;
}

mode_t kio_krarcProtocol::parsePermString(QString perm){
	mode_t mode=0;
  // file type
	if(perm[0] == 'd') mode |= S_IFDIR;
	if(perm[0] == 'l') mode |= S_IFLNK;
	if(perm[0] == '-') mode |= S_IFREG;
	// owner permissions
	if(perm[1] != '-') mode |= S_IRUSR;
	if(perm[2] != '-') mode |= S_IWUSR;
	if(perm[3] != '-') mode |= S_IXUSR;
	// group permissions
	if(perm[4] != '-') mode |= S_IRGRP;
	if(perm[5] != '-') mode |= S_IWGRP;
	if(perm[6] != '-') mode |= S_IXGRP;
	// other permissions
	if(perm[7] != '-') mode |= S_IROTH;
	if(perm[8] != '-') mode |= S_IWOTH;
	if(perm[9] != '-') mode |= S_IXOTH;

	return mode;
}

UDSEntryList* kio_krarcProtocol::addNewDir(QString path){
  UDSEntryList* dir;

  // check if the current dir exists
  dir = dirDict.find(path);
  if(dir != 0) return dir; // dir exists- return it !

  // set dir to the parent dir
  dir = addNewDir(path.left(path.findRev("/",-2)+1));

  // add a new entry in the parent dir
  QString name = path.mid(path.findRev("/",-2)+1);
  name = name.left(name.length()-1);

  UDSEntry entry;
  UDSAtom atom;
  atom.m_uds = UDS_NAME;
  atom.m_str = name;
  entry.append(atom);
    
  mode_t mode = parsePermString("drwxr-xr-x");

  atom.m_uds = UDS_FILE_TYPE;
  atom.m_long = mode & S_IFMT; // keep file type only
  entry.append( atom );

  atom.m_uds = UDS_ACCESS;
  atom.m_long = mode & 07777; // keep permissions only
  entry.append( atom );

  atom.m_uds = UDS_SIZE;
  atom.m_long = 0;
  entry.append( atom );

  atom.m_uds = UDS_MODIFICATION_TIME;
  atom.m_long = arcFile->time(UDS_MODIFICATION_TIME);
  entry.append( atom );
   
  dir->append(entry);

  // create a new directory entry and add it..
  dir = new UDSEntryList();
  dirDict.insert(path,dir);

  return dir;
}

void kio_krarcProtocol::parseLine(int lineNo, QString line, QFile*){
  UDSEntryList* dir;
	UDSEntry entry;
	UDSAtom atom;

  QString owner        = QString::null;
  QString group        = QString::null;
  QString symlinkDest  = QString::null;
  QString perm         = QString::null;
  mode_t mode          = 0666;
  size_t size          = 0;
  time_t time          = ::time(0);
  QString fullName     = QString::null;
  
  if(arcType == "zip"){
    // permissions
    perm = nextWord(line);
    if(perm.length() != 10) perm = (perm.at(0)=='d')? "drwxr-xr-x" : "-rw-r--r--" ;
    mode = parsePermString(perm);
    // ignore the next 2 fields
    nextWord(line); nextWord(line);
    // size
    size = nextWord(line).toLong();
    // ignore the next 2 fields
    nextWord(line);nextWord(line);
    // date & time
    QString d = nextWord(line);
    QDate qdate(d.mid(0,4).toInt(),d.mid(4,2).toInt(),d.mid(6,2).toInt());
    QTime qtime(d.mid(9,2).toInt(),d.mid(11,2).toInt(),d.mid(13,2).toInt());
    time = QDateTime(qdate,qtime).toTime_t();
    // full name
    fullName = nextWord(line,'\n');
  }
  if(arcType == "rar"){
    // full name
    fullName = nextWord(line,'\n');
    // size
    size = nextWord(line).toLong();
    // ignore the next 2 fields
    nextWord(line); nextWord(line);
    // date & time
    QString d = nextWord(line);
    int year = 1900 + d.mid(6,2).toInt();
    if( year < 1930 ) year+=100;
    QDate qdate( year, d.mid(3,2).toInt(), d.mid(0,2).toInt() );
    QString t = nextWord(line);
    QTime qtime(t.mid(0,2).toInt(),t.mid(4,2).toInt(),0);
    time = QDateTime(qdate,qtime).toTime_t();    
    // permissions
    perm = nextWord(line);
    if(perm.length() != 10) perm = (perm.at(0)=='d')? "drwxr-xr-x" : "-rw-r--r--" ;
    mode = parsePermString(perm);
  }
  if(arcType == "rpm"){
    // full name
    fullName = nextWord(line);
    // size
    size = nextWord(line).toULong();
    // date & time
    time = nextWord(line).toULong();
    // next field is md5sum, ignore it
    nextWord(line);
    // permissions
    mode = nextWord(line).toULong(0,8);
    // Owner & Group
    owner = nextWord(line);
    group = nextWord(line);
    // symlink destination
    if( S_ISLNK(mode) ){
			// ignore the next 3 fields
			nextWord(line); nextWord(line); nextWord(line);
			symlinkDest = nextWord(line);
    }
  }
  if( arcType == "gzip" ){
		if( !lineNo ) return; //ignore the first line
		// first field is uncompressed size - ignore it
    nextWord(line);
    // size
    size = nextWord(line).toULong();
    // ignore the next field
    nextWord(line);
    // full name
    fullName = nextWord(line);
    fullName = fullName.mid(fullName.findRev("/")+1);
  }
  if( arcType == "bzip2" ){
		// There is no way to list bzip2 files, so we take our information from
    // the archive itself...
    fullName = arcFile->name();
    if( fullName.endsWith("bz2") ) fullName.truncate(fullName.length()-4);
    mode = arcFile->mode();
    size = arcFile->size();
  }
  if( fullName.right(1) == "/" ) fullName = fullName.left(fullName.length()-1);
  if( !fullName.startsWith("/") ) fullName = "/"+fullName;
  QString path = fullName.left(fullName.findRev("/")+1);
  // set/create the directory UDSEntryList
  dir = dirDict.find(path);
  if(dir == 0) dir = addNewDir(path);
  QString name = fullName.mid(fullName.findRev("/")+1);
  // file name
  atom.m_uds = UDS_NAME;
  atom.m_str = name;
  entry.append(atom);
  // file type
  atom.m_uds = UDS_FILE_TYPE;
  atom.m_long = mode & S_IFMT; // keep file type only
  entry.append( atom );
  // file permissions
  atom.m_uds = UDS_ACCESS;
  atom.m_long = mode & 07777; // keep permissions only
  entry.append( atom );
  // file size
  atom.m_uds = UDS_SIZE;
  atom.m_long = size;
  entry.append( atom );
  // modification time
  atom.m_uds = UDS_MODIFICATION_TIME;
  atom.m_long = time;
  entry.append( atom );
  // link destination
  if( !symlinkDest.isEmpty() ){
    atom.m_uds = UDS_LINK_DEST;
    atom.m_str = symlinkDest;
    entry.append( atom );
  }
  if( S_ISDIR(mode) ){
    fullName=fullName+"/";
    if(dirDict.find(fullName) == 0)
      dirDict.insert(fullName,new UDSEntryList());
    else
    {
      // try to overwrite an existing entry
      UDSEntryList::iterator entryIt;
      UDSEntry::iterator atomIt;
      
      for ( entryIt = dir->begin(); entryIt != dir->end(); ++entryIt )
        for( atomIt = (*entryIt).begin(); atomIt != (*entryIt).end(); ++atomIt )
          if( (*atomIt).m_uds == UDS_NAME )
            if((*atomIt).m_str == name)
            {
              for( atomIt = (*entryIt).begin(); atomIt != (*entryIt).end(); ++atomIt )
              {
                switch( (*atomIt).m_uds )
                {
                case UDS_MODIFICATION_TIME:
                  (*atomIt).m_long = time;
                  break;
                case UDS_ACCESS:
                  (*atomIt).m_long = mode & 07777;
                  break;
                }
              }
              return;
            }
      return; // there is alreay an entry for this directory
    }
  }
  
  dir->append(entry);
}

bool kio_krarcProtocol::initArcParameters(){
	KRDEBUG("arcType: "<<arcType);

  if(arcType == "zip"){
    cmd = "unzip";
    listCmd = "unzip -ZTs-z-t-h ";
    getCmd  = "unzip -p ";
    delCmd  = "zip -d ";
    putCmd  = "zip -ry ";

    if( !getPassword().isEmpty() )
    {
      getCmd += "-P '"+password+"' ";
      putCmd += "-P '"+password+"' ";
    }
  } else if (arcType == "rar"){
    cmd = "unrar" ;
    listCmd = "unrar -c- v ";
    getCmd  = "unrar p -ierr -idp -c- -y ";
    delCmd  = "rar d ";
    putCmd  = "rar -r a ";
  } else if(arcType == "rpm"){
    cmd = "rpm";
    listCmd = "rpm --dump -lpq ";
    getCmd  = "cpio --force-local --no-absolute-filenames -ivdF";
    delCmd  = QString::null;
    putCmd  = QString::null;
  } else if(arcType == "gzip"){
    cmd = "gzip";
    listCmd = "gzip -l";
    getCmd  = "gzip -dc";
    delCmd  = QString::null;
    putCmd  = QString::null;
  } else if(arcType == "bzip2"){
    cmd = "bzip2" ;
    listCmd = "bzip2";
    getCmd  = "bzip2 -dc";
    delCmd  = QString::null;
    putCmd  = QString::null;
  } else {
    cmd     = QString::null;
    listCmd = QString::null;
    getCmd  = QString::null;
    delCmd  = QString::null;
    putCmd  = QString::null;
  }

  if( KStandardDirs::findExe(cmd).isEmpty() ){
    error( KIO::ERR_CANNOT_LAUNCH_PROCESS,
    cmd+
    i18n("\nMake sure that the %1 binary are installed properly on your system.").arg(cmd));
    KRDEBUG("Failed to find cmd: " << cmd);
    return false;
  }
  return true;
}


QString kio_krarcProtocol::getPassword() {    
  if( !password.isNull() )
    return password;

  QFile zipFile( arcFile->url().path() );
  if( zipFile.open( IO_ReadOnly ) )
  {
    char zipHeader[8];

    if( zipFile.readBlock( zipHeader, 8 ) != 8 )
      return (password = "" );

    if( zipHeader[0] != 'P' || zipHeader[1] != 'K' || zipHeader[2] != 3 || zipHeader[3] != 4 )
      return (password = "" );
    
    if( ! ( zipHeader[6] & 1 ) )  /* not encrypted */
      return (password = "" );
        
    KIO::AuthInfo authInfo;
    authInfo.caption= i18n( "Zip Password Dialog" );
    authInfo.username= "zipfile";
    authInfo.readOnly = true;
    authInfo.keepPassword = true;
    authInfo.verifyPath = true;
    authInfo.url = KURL::fromPathOrURL( arcFile->url().path() );

    if( checkCachedAuthentication( authInfo ) )
      return ( password = authInfo.password );

    if ( openPassDlg( authInfo ) )
      return ( password = authInfo.password );
  }

  return (password = "" );
}

#include "krarc.moc"
