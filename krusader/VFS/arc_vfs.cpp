/***************************************************************************
                                arc_vfs.cpp
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
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

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
// QT includes
#include <qregexp.h>
#include <qdir.h>
#include <qdatetime.h>
#include <qfileinfo.h>
// KDE includes
#include <kmessagebox.h>
#include <klocale.h>
#include <k3process.h>
#include <kio/jobclasses.h>
#include <q3progressdialog.h>
#include <kglobalsettings.h>
#include <kmimetype.h>
#include <kcursor.h>
#include <kde_file.h>
// krusader includes
#include "arc_vfs.h"
#include "krpermhandler.h"
#include "krarchandler.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../resources.h"
#include "../Dialogs/krdialogs.h"

#define MAX_FILES 500

//constructor
arc_vfs::arc_vfs(QString origin,QString type,QObject* panel,bool write):
  vfs(panel),arcFile(origin),changed(false),prefix(""),ignoreLines(0){

	if ( type == "tarz" ) type = "-tgz";
	
	// set the cursor to busy mode
  if (!quietMode) krApp->setCursor(Qt::WaitCursor);

  // set the writable attribute
	isWritable = KRpermHandler::fileWriteable(origin);
  isWritable = ( write && isWritable ); 	

  vfs_type = vfs::ERROR;

	// create the temp dir..
  tmpDir = krApp->getTempDir();
 	if( tmpDir.isEmpty() ){
  	error = true;
		return;
	}

	QString password = QString::null;
  krConfig->setGroup("Archives");
  // fill the command options
  if( type == "gzip" ){
    cmd = KrServices::fullPathName ( "gzip" );
    listCmd = "-l";
    delCmd  = "";
    addCmd  = KrServices::fullPathName ( "gzip" ) + " -c";
    getCmd  = "-dc";
		ignoreLines = -1;
		isWritable = false;
  }
  if(type == "zip2"){
    cmd = KrServices::fullPathName( "bzip2" );
    listCmd = "";
    delCmd  = "";
    addCmd  = KrServices::fullPathName( "bzip2" )+ " -c";
    getCmd  = "-dc";
		ignoreLines = -1;
    isWritable = false;
  }
  if(type == "-tar"){
    cmd = KrServices::fullPathName( "tar" );
    listCmd = " -tvf";
    delCmd  = cmd+" --delete -vf";
    addCmd  = cmd+" -uvf";
    getCmd  = " -xvf";
  }
	if(type == "-tgz"){
    cmd = KrServices::fullPathName( "tar" );
    listCmd = " -tzvf";
    delCmd  = "";
    addCmd  = cmd+" -uvzf";
    getCmd  = " -xzvf";
    isWritable = false;
  }
  if(type == "-tbz"){
    cmd = KrServices::fullPathName( "tar" );
    listCmd = " -tjvf";
    delCmd  = "";
    addCmd  = cmd+" -uvjf";
    getCmd  = " -xjvf";
    isWritable = false;
  }
	if(type == "-zip"){
    password = KRarcHandler::getPassword(arcFile,type);
		cmd = KrServices::fullPathName( "unzip" );
    listCmd = "-ZTs ";
    QString zipcmd = KrServices::fullPathName( "zip" );
    delCmd  = zipcmd+" -d";
    addCmd  = zipcmd+" -ry";
    getCmd  = " -o";
		if( !password.isEmpty() ){
    	//listCmd = listCmd + " -P "+password;
			delCmd = delCmd + " -P "+password;
			addCmd = addCmd + " -P "+password;
			getCmd = getCmd + " -P "+password;
		}
    ignoreLines = 1;
  }
  // "-rpm" is used only to list the rpm - to extract files use "+rpm"
  if(type == "-rpm"){
    //rpm can't handle files with " " in them so replace " " with "\ "
    arcFile.replace(QRegExp(" "),"\\ ");

    cmd = KrServices::fullPathName( "rpm" );
    listCmd = " --dump -lpq ";
    delCmd  = "";
    addCmd  = "";
    getCmd  = "";
    isWritable    = false;
  }
  if( type == "+rpm" ){
		// extract the cpio archive from the rpm
		K3ShellProcess rpm;
  	rpm << "rpm2cpio"<<"\""+arcFile+"\""+" > "+tmpDir+"/contents.cpio";
  	rpm.start(K3Process::Block);
		arcFile = tmpDir+"/contents.cpio";
	}
	if(type == "cpio" || type == "+rpm" ){
    cmd = KrServices::fullPathName( "cpio" );
    listCmd = "-tvF ";
    delCmd  = "";
    addCmd  = "";
    getCmd  = " --force-local --no-absolute-filenames -ivdF";
    isWritable    = false;
  }
  if(type == "-rar"){
    bool doRar = krConfig->readBoolEntry("Do Rar",_DoRar);
    cmd = KrServices::fullPathName( "unrar" );
    listCmd = " -c- v ";
    delCmd  = "";
    addCmd  = (doRar ? QString(KrServices::fullPathName( "rar" ) + " -r a ") : QString("")) ;
    getCmd  = " x -y ";
    ignoreLines = 8;
    isWritable    = (doRar && isWritable );
  }

	getDirs();
	// set the cursor to normal mode
  if (!quietMode) krApp->setCursor(Qt::ArrowCursor);
}

// return the working dir
QString arc_vfs::vfs_workingDir(){
  // get the path inside the archive
  QString path = vfs_origin.right((vfs_origin.length()-vfs_origin.findRev('\\'))-1);
  if(path.left(1) != "/") path = "/"+path;
  QDir().mkdir(tmpDir+path);
  return tmpDir+path;
}

arc_vfs::~arc_vfs(){
  // set the cursor to busy mode
  if (!quietMode) krApp->setCursor(Qt::WaitCursor);
	// don't touch messed-up archives
	if(!error) repack();
  // delete the temp dir
  K3ShellProcess proc;
  proc<<"rm"<<"-rf"<<tmpDir;
  proc.start(K3Process::Block);

	// set the cursor to normal mode
  if (!quietMode) krApp->setCursor(Qt::ArrowCursor);
}	

bool arc_vfs::getDirs(){
  if( !listCmd.isEmpty() ){
    // write the temp file
    K3ShellProcess proc;
    proc << cmd << listCmd << "\""+arcFile+"\"" <<" > " << tmpDir+"/tempfilelist";
    proc.start(K3Process::Block);
    if( !proc.normalExit() || !proc.exitStatus() == 0 ){
      if (!quietMode) KMessageBox::error(krApp, i18n("<qt>Can't read <b>%1</b>. Archive "
                      "might be corrupted!</qt>", arcFile.mid(arcFile.findRev('/')+1)));
     error = true;
		 return false;
    }
		
		// clear the dir list
  	dirList.clear();

  	// prepare the first dir entry - the "" entry
  	arc_dir *tempdir = new arc_dir("");
  	vfs_filesP = &(tempdir->entries);
  	dirList.append(tempdir);

    // parse the temp file
    QFile temp(tmpDir+"/tempfilelist");
    temp.open(QIODevice::ReadOnly);
    char buf[1000];
    QString line;
    if(vfs_type == "gzip" || vfs_type == "-zip" )
      temp.readLine(line,10000);  // skip the first line - it's garbage
    if( vfs_type == "-rar" ){
      while(temp.readLine(line,10000) != -1)
        if ( line.contains("----------") ) break;
    }
    while(temp.readLine(buf,1000) != -1){
      line = QString::fromLocal8Bit(buf);
      if ( line.contains("----------") ) break;
      parseLine(line.trimmed(),&temp);

    }
    temp.close();
    QDir().remove(tmpDir+"/tempfilelist");
  }
  else { // bzip2
		// clear the dir list
  	dirList.clear();

  	// prepare the first dir entry - the "" entry
  	arc_dir *tempdir = new arc_dir("");
  	vfs_filesP = &(tempdir->entries);
  	dirList.append(tempdir);

	 	parseLine("",0);
	}
	return true;
}


// copy a file to the vfs (physical)
void arc_vfs::vfs_addFiles(KUrl::List *fileUrls,KIO::CopyJob::CopyMode mode,QObject* toNotify,QString dir, PreserveMode /*pmode*/ ){
  if ( addCmd.isEmpty() ) return;

  // get the path inside the archive
  QString path = vfs_origin.right((vfs_origin.length()-vfs_origin.findRev('\\'))-1);
  path = path+"/";
  if(dir != "" ) dir = "/"+dir;
  if(path.left(1) != "/") path = "/"+path;

  // make sure the destination exist
  for( int i=0; i >= 0 ; i= QString(tmpDir+path+dir).find('/',i+1) ){
    QDir().mkdir(QString(tmpDir+path+dir).left(i));
  }

  changed = true; //rescan the archive
	
	KUrl dest;
	dest.setPath(tmpDir+path+dir);

  KIO::Job* job = new KIO::CopyJob(*fileUrls,dest,mode,false,true);
  connect(job,SIGNAL(result(KJob*)),this,SLOT(vfs_refresh(KJob*)) );
  if(mode == KIO::CopyJob::Move) // notify the other panel
   connect(job,SIGNAL(result(KJob*)),toNotify,SLOT(vfs_refresh(KJob*)) );
}
	

// remove a file from the vfs (physical)
void arc_vfs::vfs_delFiles(QStringList *fileNames){
  if ( delCmd.isEmpty() ) return;
	// if we move to trash - just extract files and move them to trash -
  // the repack() will delete them for us
  krConfig->setGroup("General");
	if( krConfig->readBoolEntry("Move To Trash",_MoveToTrash) ) {
	  KUrl::List* filesUrls = vfs_getFiles(fileNames); // extract
	  changed = true;
	
	  KIO::Job *job = new KIO::CopyJob(*filesUrls,KGlobalSettings::trashPath(),KIO::CopyJob::Move,false,true );
	  connect(job,SIGNAL(result(KJob*)),this,SLOT(vfs_refresh(KJob*)));
	}
	// else we have to delete the files from both the archive and the temp dir
	else {
	  // change dir to the temp dir
    QString save = getcwd(0,0);
    chdir(tmpDir.local8Bit());

    QStringList files;
    KIO::filesize_t totalSizeVal = 0;
    unsigned long totalFilesVal =  0;
    	
	  // names -> urls
    for(QStringList::Iterator name = fileNames->begin(); name != fileNames->end(); ++name )
      processName(*name,&files,&totalSizeVal,&totalFilesVal);


		K3ShellProcess proc1 , proc2;
		krApp->startWaiting(i18n("Deleting Files..."),files.count()+ignoreLines);
	  connect(&proc1,SIGNAL(receivedStdout(K3Process*,char*,int)),
            krApp->plzWait, SLOT(incProgress(K3Process*,char*,int)) );

    proc1 <<  delCmd << "\""+arcFile+"\"";
    proc2 << "rm -rf";
    for(unsigned int i =0; i < files.count(); ){
      proc1 << (prefix+*files.at(i));
      proc2 << tmpDir+"/"+(*files.at(i));
			extFiles.remove(*files.at(i++));
			if ( i%MAX_FILES==0 || i==files.count() ){
				proc1.start(K3Process::NotifyOnExit,K3Process::AllOutput);
    		proc2.start();
        while( proc1.isRunning() || proc2.isRunning() ) qApp->processEvents(); // busy wait - need to find something better...
        proc1.clearArguments() ; proc2.clearArguments();
				proc1 <<  delCmd << "\""+arcFile+"\"";
    		proc2 << "rm -rf";
			}
    }
	  krApp->stopWait();

	  changed = true;
	  chdir (save.local8Bit());
	  vfs_refresh(vfs_origin);
  }
}	

// return a path to the file
QString arc_vfs::vfs_getFile(QString name){
  // get the current file path
  QString path = vfs_origin.right((vfs_origin.length()-vfs_origin.findRev('\\'))-1);
  if(path.left(1)=="/") path.remove(0,1);
  if(path != "") path = path+"/";

  QStringList temp(name);
  vfs_getFiles(&temp);

  return tmpDir+"/"+path+name;
}

KUrl::List* arc_vfs::vfs_getFiles(QStringList* names){
  KUrl url;
  KUrl::List* urls = new KUrl::List();

  // get the current file path
  QString path = vfs_origin.right((vfs_origin.length()-vfs_origin.findRev('\\'))-1);
  if(path.left(1)=="/") path.remove(0,1);
  if(path != "") path = path+"/";

  // change dir to the temp dir
  QString save = getcwd(0,0);
	chdir(tmpDir.local8Bit());
	// names -> urls
  QStringList files;
  KIO::filesize_t totalSize = 0;
  unsigned long totalFiles = 0;
  for(QStringList::Iterator name = names->begin(); name != names->end(); ++name ){
    processName(*name,&files,&totalSize,&totalFiles);
    url.setPath(tmpDir+"/"+path+(*name));
    urls->append(url);
  }
  // check the urls for unpacked files and directories
	for(QStringList::Iterator file = files.begin(); file != files.end(); ++file ){
  	if ( (*file).right(1)=="/" ){
			QDir(tmpDir).mkdir(*file);
      if( vfs_type == "-rar" ) file = files.remove(file--);
		}
		// don't unpack the same file twice
		else if( extFiles.contains(*file) ){
      file = files.remove(file--);
		}
  }
  // unpack ( if needed )
	if ( files.count() > 0 ){
		krApp->startWaiting(i18n("Unpacking Files"),files.count()+ignoreLines);
    K3ShellProcess proc;
	  connect(&proc,SIGNAL(receivedStdout(K3Process*,char*,int)),
            krApp->plzWait, SLOT(incProgress(K3Process*,char*,int)) );
		
		proc << cmd << getCmd << "\""+arcFile+"\"";
  	if( vfs_type == "gzip" || vfs_type == "zip2" ) proc << ">";
		for(unsigned int i=0 ; i < files.count() ; ){
  		proc << (prefix+*files.at(i++));
			if ( i%MAX_FILES==0 || i==files.count() ){
				proc.start(K3Process::NotifyOnExit,K3Process::AllOutput);
        while( proc.isRunning() ) qApp->processEvents();
				proc.clearArguments();
				proc << cmd << getCmd << "\""+arcFile+"\"";
  		}
		}
    getExtFiles(); // this will update the extFiles list.
		krApp->stopWait();
	}
  // restore dir
	chdir(save.local8Bit());
	
  return urls;
}

// make dir
void arc_vfs::vfs_mkdir(QString name){
  QString path = vfs_origin.right((vfs_origin.length()-vfs_origin.findRev('\\'))-1);
  if(path.left(1)=="/") path.remove(0,1);
  if(path != "") path = path+"/";

  QDir(tmpDir).mkdir(path+name);
  changed = true; //rescan the archive
  vfs_refresh(vfs_origin);
}
	
// rename file
void arc_vfs::vfs_rename(QString fileName,QString newName){
	KUrl::List temp;
	temp.append(vfs_getFile(fileName));
  QString path = vfs_origin.right((vfs_origin.length()-vfs_origin.findRev('\\'))-1);
  if(path.left(1)=="/") path.remove(0,1);
  if(path != "") path = path+"/";

  QDir(tmpDir).mkdir(path);
  changed = true; //rescan the archive

	KUrl dest;
	dest.setPath(tmpDir+path+"/"+newName);

  KIO::Job* job = KIO::move(temp,dest,false);
  connect(job,SIGNAL(result(KJob*)),this,SLOT(vfs_refresh(KJob*)) );
}

bool arc_vfs::vfs_refresh(QString origin){
	if ( error ) return false;
	
  if ( changed || origin == vfs_origin ){
		repack(); // repack dirs only if needed
		if ( !getDirs() ){
			if (!quietMode) emit startUpdate();
    	return true;
		}
		changed = false;
	}

  vfs_origin = origin;
  // get the directory...
  QString path = origin.right((origin.length()-origin.findRev('\\'))-1);
  if(path.left(1) =="/") path.remove(0,1);

  vfs_filesP = findDir(path);

  if (!quietMode) emit startUpdate();
  return true;
}

// service functions
QString arc_vfs::nextWord(QString &s,char d) {
  s=s.trimmed();
  int j=s.find(d,0);
  QString temp=s.left(j); // find the leftmost word.
  s.remove(0,j);
  return temp;
}

void arc_vfs::getFilesToPack(QStringList* filesToPack,QString dir_name){
  bool newDir = false;
  vfileDict *vfs_filesP_backup = vfs_filesP; // save vfs_filesP

  // init all the diffrent lists (and list pointers);
  vfs_filesP=findDir(dir_name);
  if ( vfs_filesP == 0) newDir = true;
  if(dir_name != "") dir_name = dir_name+"/";

 	register DIR* dir = opendir(tmpDir.local8Bit()+"/"+dir_name.local8Bit());
  if(!dir) return ;

	register struct dirent* dirEnt;
  QString name;
	KDE_struct_stat stat_p;
	while( (dirEnt=readdir(dir)) != NULL ){
    name = dirEnt->d_name;
		if ( name == ".." || name == "." ) continue;
	  if( KDE_lstat(tmpDir.local8Bit()+"/"+dir_name.local8Bit()+name.local8Bit(),&stat_p) ) continue;
	  extFile temp(dir_name+name,stat_p.st_mtime,stat_p.st_size);
		// add to the list file that are diffrent than the ones packed
    if( S_ISDIR(stat_p.st_mode) ){ // recurse on all sub dirs
      if( !findDir(dir_name+name) ){
      	// add to the list only new && empty dirs
				if( newDir && QDir(dir_name+name).entryList(QDir::TypeMask | QDir::AccessMask).count() <= 2 )
					filesToPack->append( dir_name+name);
      }
			getFilesToPack(filesToPack,dir_name+name);
			continue;
		}

		// if the file don't exist add it to the archive and to the extFiles
    if( newDir || !extFiles.contains( dir_name+name ) ){
    	filesToPack->append( dir_name+name );
			extFiles.append( temp );
    } // else if the file exist but was modified - repack it;
    else if( !extFiles.contains( temp ) ){
			filesToPack->append( dir_name+name );
			extFiles.remove( dir_name+name );
			extFiles.append( temp );
    }
  }
  vfs_filesP = vfs_filesP_backup; // restore  vfs_filesP
}

void arc_vfs::getFilesToDelete(QStringList* filesToDelete,QString){
	// sync the extFiles - and find out which files were deleted
	QString file;
	for(unsigned int i=0 ; i<extFiles.count(); ){
		file = tmpDir+"/"+(*extFiles.at(i)).url;
		if( !KRpermHandler::fileExist(file) ){
  		filesToDelete->append( (*extFiles.at(i)).url );
    	extFiles.remove(extFiles.at(i));
   	}
		else ++i;
  }
}

void arc_vfs::getExtFiles(QString dir_name){
	register DIR* dir = opendir(tmpDir.local8Bit()+"/"+dir_name.local8Bit());
  if(!dir){
    kWarning() << "faild to opendir(): " << tmpDir.local8Bit()+"/"+dir_name.local8Bit() << endl;
		return ;
	}

	if( dir_name != "") dir_name = dir_name+"/";
	
	register struct dirent* dirEnt;
  QString name;
	KDE_struct_stat stat_p;
	while( (dirEnt=readdir(dir)) != NULL ){
    name = dirEnt->d_name;
		if ( name == ".." || name == "." ) continue;
	  if( KDE_lstat(tmpDir.local8Bit()+"/"+dir_name.local8Bit()+name.local8Bit(),&stat_p) ) continue;
	  extFile temp(dir_name+name,stat_p.st_mtime,stat_p.st_size);
		// recurse on all sub dirs
    if( S_ISDIR(stat_p.st_mode) ){
      getExtFiles(dir_name+name);
		}
    // if the file is not in extFiles - it is newly extracted.
    // note: getFilesToPack() updates time + size !
		else if( !extFiles.contains( dir_name+name ) ){
			extFiles.append( temp );
    }
  }	
}

void arc_vfs::repack(){
  QString save = getcwd(0,0);
  chdir(tmpDir.local8Bit());
	
  // delete from the archive files that were unpacked and deleted
	if( vfs_isWritable() ){
		QStringList filesToDelete;
		getFilesToDelete(&filesToDelete);
		if( !filesToDelete.isEmpty() ){
			K3ShellProcess delProc;
  		krApp->startWaiting(i18n("Deleting Files..."),filesToDelete.count()+ignoreLines);
	 		connect(&delProc,SIGNAL(receivedStdout(K3Process*,char*,int)),
               krApp->plzWait, SLOT(incProgress(K3Process*,char*,int)) );

			delProc << delCmd << "\""+arcFile+"\"";
			for( unsigned int i=0 ; i < filesToDelete.count() ;){
				delProc << (*filesToDelete.at(i++));
				if( i%MAX_FILES==0 || i==filesToDelete.count() ){
  	  		delProc.start(K3Process::NotifyOnExit,K3Process::AllOutput);
    			while( delProc.isRunning() )  qApp->processEvents();
					delProc.clearArguments();
		    	delProc << delCmd << "\""+arcFile+"\"";
  			}
			}
  		krApp->stopWait();
		}
	}

  // finaly repack tmpDir
  if( vfs_isWritable() || vfs_type=="gzip" || vfs_type=="zip2" ){
    QStringList filesToPack;
		getFilesToPack(&filesToPack);
		if( !filesToPack.isEmpty() ){
			K3ShellProcess addProc;
			krApp->startWaiting(i18n("Repacking..."),filesToPack.count()+ignoreLines);
    	connect(&addProc,SIGNAL(receivedStdout(K3Process*,char*,int)),
            krApp->plzWait, SLOT(incProgress(K3Process*,char*,int)) );

			if( vfs_type=="gzip" || vfs_type=="zip2" ){
      	addProc << addCmd << *filesToPack.at(0)<< ">" << "\""+arcFile+"\"";
				addProc.start(K3Process::NotifyOnExit);
      	while( addProc.isRunning() ) qApp->processEvents();
     	}
    	else {
				addProc << addCmd << "\""+arcFile+"\"";
    		for( unsigned int i=0 ; i<filesToPack.count(); ){
      		addProc << "\""+prefix+(*filesToPack.at(i++))+"\"";
					if( i%MAX_FILES==0 || i==filesToPack.count() ){
      			addProc.start(K3Process::NotifyOnExit,K3Process::AllOutput);
      			while( addProc.isRunning() ) qApp->processEvents(); // busy wait - need to find something better...
        		addProc.clearArguments();
		    		addProc << addCmd << "\""+arcFile+"\"";
					}
    		}
			}
			krApp->stopWait();
		}
 	}
  chdir(save.local8Bit());
}

vfileDict* arc_vfs::findDir(QString name){
  for(arc_dir* temp = dirList.first();temp != 0 ; temp = dirList.next()){
    if(temp->name == name) return &(temp->entries);
  }
  return 0;
}

arc_vfs::arc_dir* arc_vfs::findArcDir(QString name){
for(arc_dir* temp = dirList.first();temp != 0 ; temp = dirList.next()){
    if(temp->name == name) return temp;
  }
  return 0;
}

QString arc_vfs::changeDir(QString name){
  if(name.left(2) == "./") {
    prefix = "./";
    name.remove(0,2);
  }

  if(!name.contains('/')){
    vfs_filesP = findDir("");
    return name;
  }
  // seperate the path from the name
  QString path = name.left(name.findRev('/'));
  name = name.mid(name.findRev('/')+1);
  // see if the path exists
  if ((vfs_filesP=findDir(path)) == 0){
   //create a new dir entry
   QString Pname = path.mid(path.findRev('/')+1);
   if(Pname.isEmpty()) return name;
   QString tempName = arcFile;
   QFileInfo qfi(tempName.replace(QRegExp("\\"),""));
   vfile* vf=new vfile(Pname,0,"drwxr-xr-x",qfi.lastModified().toTime_t(),false,
                 qfi.owner(),qfi.group(),"inode/directory","",0 );
   // add  dirs if needed
   changeDir(path);

   vfile* vf2 = vfs_search(Pname);
   if(vf2 != 0) vfs_removeFromList(vf2);
   vfs_addToList(vf);

   // add a new arc_dir
   dirList.append(new arc_dir(path));
   vfs_filesP = findDir(path);
  }
  return name;
}

// calculate space
void arc_vfs::vfs_calcSpace(QString name ,KIO::filesize_t *totalSize,unsigned long *totalFiles,unsigned long *totalDirs,bool* stop){
  if ( *stop ) return;
  vfile* vf = vfs_search(name);

  // get the path inside the archive
  QString path = vfs_origin.right((vfs_origin.length()-vfs_origin.findRev('\\'))-1);
  path = path+"/";
  if(path.left(1) == "/") path.remove(0,1);

  if( !vf->vfile_isDir() ){ // single files are simple :)
    ++(*totalFiles);
    (*totalSize) += vf->vfile_getSize();
  }
  else { // handle directories : (
    ++(*totalDirs);

    QString origin_backup = vfs_origin; // backup the vfs origin
    vfs_origin = vfs_origin+"/"+name;
    vfileDict* vfs_filesP_backup = vfs_filesP; // backup the vfs_filesP
    vfs_filesP = findDir(path+name);

    // process all the files in the directory.
    for( vf = vfs_getFirstFile(); vf != 0; vf = vfs_getNextFile() ){
      if (*stop) return;
      vfs_calcSpace(vf->vfile_getName(),totalSize,totalFiles,totalDirs,stop);
    }

    vfs_origin = origin_backup;     // restore origin
    vfs_filesP = vfs_filesP_backup; // restore vfs_filesP
  }
}

void arc_vfs::processName(const QString& name, QStringList *urls,KIO::filesize_t *totalSize,unsigned long *totalFiles ){
  vfile* vf = vfs_search(name);
	if ( vf == 0 ) return;

  // get the path inside the archive
  QString path = vfs_origin.right((vfs_origin.length()-vfs_origin.findRev('\\'))-1);
  path = path+"/";
  if(path.left(1) == "/") path.remove(0,1);

  if( !vf->vfile_isDir() || vf->vfile_isSymLink() ){ // single files are simple :)
    ++(*totalFiles);
    (*totalSize) += vf->vfile_getSize();
    urls->append(path+name);
  } else { // handle directories : (
    urls->append(path+name+"/");
    QString origin_backup = vfs_origin; // backup the vfs origin
    vfs_origin = vfs_origin+"/"+name;
    vfileDict* vfs_filesP_backup = vfs_filesP; // backup the vfs_filesP
    vfs_filesP = findDir(path+name);

    // process all the files in the directory.
    for( vf = vfs_getFirstFile(); vf != 0; vf = vfs_getNextFile() )
      processName(vf->vfile_getName(),urls,totalSize,totalFiles);

    vfs_origin = origin_backup;     // restore origin
    vfs_filesP = vfs_filesP_backup; // restore vfs_filesP
  }
}

void arc_vfs::parseLine(QString line, QFile* temp){
  QString name;
  KIO::filesize_t size = 0;
  QString perm;
  QFileInfo qfi(arcFile);
  time_t mtime = qfi.lastModified().toTime_t();
  bool link = false;
  uid_t owner = getuid();
  gid_t group = getgid();
  QString dest = "";
  mode_t mode = 0;


  // parse gziped files
  if(vfs_type == "gzip"){
    KDE_struct_stat stat_p;
    KDE_stat(arcFile.local8Bit(),&stat_p);

    nextWord(line);
    size = nextWord(line).toLong();
    nextWord(line);
    name = nextWord(line,'\n');
    if(name.contains('/')) name = name.mid(name.findRev('/')+1,name.length());
    perm  = KRpermHandler::mode2QString(stat_p.st_mode) ;
    owner = KRpermHandler::user2uid(qfi.owner());
    group = KRpermHandler::group2gid(qfi.group());
    mode  = stat_p.st_mode;
  }

  // parse bzip2ed files
  if( vfs_type == "zip2" ){
    KDE_struct_stat stat_p;
    KDE_stat(arcFile.local8Bit(),&stat_p);

    name = qfi.fileName();
    name = name.left(name.findRev('.'));
    //long size = qfi.size();
    perm  = KRpermHandler::mode2QString(stat_p.st_mode) ;
    owner = KRpermHandler::user2uid(qfi.owner());
    group = KRpermHandler::group2gid(qfi.group());
    mode  = stat_p.st_mode;
  }

  // parse tar files
  if(vfs_type == "-tar" || vfs_type == "-tbz" || vfs_type == "-tgz" ){
    perm = nextWord(line);
    QString temp = nextWord(line);
    owner = temp.left(temp.findRev('/')).toInt();
    group = temp.mid(temp.find('/')+1,temp.length()).toInt();
    size = nextWord(line).toLong();
    temp = nextWord(line);
    name = nextWord(line,'\n');
    if (name.startsWith("/"))  // fix full-paths problem in tar (thanks to Heiner!)
      name.remove(0, 1);
    if( name.contains(" -> ") ){
      link = true;
			dest = name.mid(name.find(" -> ")+4);
			name = name.left(name.find(" -> "));
    }
  }

  // parse zipped files
  if(vfs_type == "-zip"){
    perm = nextWord(line);
    if(perm.length() != 10)
      perm = (perm.at(0)=='d')? "drwxr-xr-x" : "-rw-r--r--" ;
    if (nextWord(line).contains("file")) return;
    nextWord(line);
    size = nextWord(line).toLong();
    nextWord(line);nextWord(line);
    QString temp = nextWord(line);
    name = nextWord(line,'\n');
  }

  // parse cpio packages
  if(vfs_type == "cpio" || vfs_type == "+rpm"){
		perm = nextWord(line);
    nextWord(line);nextWord(line);nextWord(line);
		size = nextWord(line).toLong();
		nextWord(line);nextWord(line);nextWord(line);
		QString tempName = arcFile;
    QFileInfo qfi(tempName.replace(QRegExp("\\"),""));
		name = nextWord(line,'\n');
    if ( name.left(1) == "/" ) name.remove(0,1);
    if( name.contains(" -> ") ){
      link = true;
      dest = name.mid(name.find(" -> ")+4);
			name = name.left(name.find(" -> "));
    }
  }
  // parse rared files
  if(vfs_type == "-rar"){
    name = nextWord(line,'\n');
    temp->readLine(line,10000);
    size = nextWord(line).toLong();
    nextWord(line);
    nextWord(line);
    perm = nextWord(line);
    if(perm.length() != 10)
      perm = (perm.at(1)=='D')? "drwxr-xr-x" : "-rw-r--r--" ;
  }
  // parse rpm packages
  if(vfs_type == "-rpm"){
		name = nextWord(line);
    if ( name.left(1) == "/" ) name.remove(0,1);
    size = nextWord(line).toLong();
    mtime = nextWord(line).toLong();
    nextWord(line);
    perm = KRpermHandler::mode2QString(nextWord(line).toLong());
    perm = (perm.at(0)=='d')? "drwxr-xr-x" : "-rw-r--r--" ;
  }

  if ( perm[0]=='d'  && name.right(1) != "/" )  name = name+"/";
  name = changeDir(name);
  if(name.length() < 1) return;


  QString mime = KMimeType::findByUrl( "/"+name,0,true,true)->name();
  vfile* vf=new vfile(name,size,perm,mtime,link,owner,group,mime,dest,mode);
  vfile* vf2 = vfs_search(name);
  if(vf2 != 0) vfs_removeFromList(vf2);
  vfs_addToList(vf);
}

#include "arc_vfs.moc"
