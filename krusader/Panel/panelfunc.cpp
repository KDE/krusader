/***************************************************************************
                                panelfunc.cpp
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
// Qt Includes
#include <qdir.h>
#include <qtextstream.h>
// KDE Includes
#include <klocale.h>
#include <kio/jobclasses.h>
#include <kprocess.h>
#include <kpropertiesdialog.h>
#include <kopenwith.h>
#include <kmessagebox.h>
#include <kcursor.h>
#include <kstddirs.h>
#include <ktempfile.h>
#include <kurl.h>
#include <krun.h>
#include <klineeditdlg.h>
// Krusader Includes
#include "panelfunc.h"
#include "krlistitem.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../VFS/normal_vfs.h"
#include "../VFS/ftp_vfs.h"
#include "../VFS/arc_vfs.h"
#include "../VFS/temp_vfs.h"
#include "../VFS/vfile.h"
#include "../VFS/krarchandler.h"
#include "../Dialogs/packgui.h"
#include "../Dialogs/krdialogs.h"
#include "../Dialogs/krpleasewait.h"
#include "../Dialogs/krspwidgets.h"
#include "../KViewer/kviewer.h"
#include "../MountMan/kmountman.h"
#include "../resources.h"

//////////////////////////////////////////////////////////
//////		----------	List Panel -------------		////////
//////////////////////////////////////////////////////////

ListPanelFunc::ListPanelFunc(ListPanel *parent):panel(parent){}

void ListPanelFunc::goBack(){
  if(backStack.isEmpty()) return;
	
  if(backStack.last() == "//WARNING//"){
    KMessageBox::information(0, i18n("Can't re-enter archive. Going to the nearest path"), QString::null, "BackArchiveWarning");
    backStack.remove(backStack.fromLast()); //remove the //WARNING// entry
  }
  QString path = backStack.last();
  panel->refresh(path);

  backStack.remove(backStack.fromLast());
  if( !backStack.isEmpty() ) // the "vfs_refresh" push too much :)
    backStack.remove(backStack.fromLast());

  if (backStack.isEmpty()) krBack->setEnabled(false);
}

void ListPanelFunc::redirectLink(){
	if ( panel->files->vfs_getType() == "ftp" ){
		KMessageBox::sorry(krApp,i18n("You can edit links only on local file systems") );
	  return;
	}

	vfile *vf = panel->files->vfs_search( panel->getCurrentName() );
	if ( !vf ) return;

	QString file = panel->files->vfs_getFile( vf->vfile_getName() );
	QString currentLink = vf->vfile_getSymDest();
	if ( currentLink.isEmpty() ){
		KMessageBox::sorry(krApp,i18n("The current file is not a link, so i can't redirect it.") );
	  return;
	}

	// ask the user for a new destination
  bool ok=false;
  QString newLink =
		KLineEditDlg::getText(i18n("Please enter the new link destination:"),currentLink,&ok,krApp);

	// if the user canceled - quit
	if ( !ok || newLink==currentLink ) return;
	// delete the current link
	if ( unlink(file.local8Bit()) == -1 ){
		KMessageBox::sorry(krApp,i18n("Can't remove old link: ")+file.latin1() );
	  return;
  }
	// try to create a new symlink
	if ( symlink(newLink.local8Bit(),file.local8Bit()) == -1){
		KMessageBox::sorry(krApp,i18n("Failed to create a new link: ")+file.latin1());
		return;
	}
}

void ListPanelFunc::krlink(bool sym){
	if ( panel->files->vfs_getType() == "ftp" ){
		KMessageBox::sorry(krApp,i18n("You can create links only on local file systems") );
	  return;
	}

	QString name = panel->getCurrentName();

	// ask the new link name..
	bool ok=false;
  QString linkName =
		KLineEditDlg::getText(i18n("Create a new link to: ")+name.latin1(),name.latin1(),&ok,krApp);

	// if the user canceled - quit
	if ( !ok || linkName==name ) return;

	// if the name is already taken - quit
	if (panel->files->vfs_search(linkName) != 0){
		KMessageBox::sorry(krApp,i18n("A directory or a file with this file already exists."));
		return;
	}
	
  if ( linkName.left(1) != "/" )
		linkName = panel->files->vfs_workingDir()+"/"+linkName;

	if ( linkName.contains("/") )
		name = panel->files->vfs_getFile( name );

  if ( sym ){
	  if ( symlink(name.latin1(),linkName.latin1()) == -1)
		  KMessageBox::sorry(krApp,i18n("Failed to create a new symlink: ")+linkName+
	                            i18n(" To: ")+name);
  }
  else {
	  if ( link(name.latin1(),linkName.latin1()) == -1)
		  KMessageBox::sorry(krApp,i18n("Failed to create a new link: ")+linkName+
                               i18n(" To: ")+name);
  }
}

void ListPanelFunc::view(){
	// if we're trying to view a directory, just exit
  vfile* vf = panel->files->vfs_search( panel->getCurrentName() );
	if ( !vf || vf->vfile_isDir() ) return;
	// let's get the file name (without the full path)
	QString fileName = panel->getCurrentName();
	if (fileName.isNull()) return;
	// now, we need it's mimetype, thanks to the killer vFile
	QString mimetype = panel->files->vfs_search(fileName)->vfile_getMime();
	// at this point, let's take the full path
	fileName = panel->files->vfs_getFile(fileName);
  // and call KViewer.
  KViewer::view(fileName,mimetype);
  // nothing more to it!	
}

void ListPanelFunc::terminal(){
	QString save = getcwd(0,0);
	chdir( panel->realPath.local8Bit() );
	
	KProcess proc;
	krConfig->setGroup("General");
	QString term = krConfig->readEntry("Terminal",_Terminal);
	proc <<  term;
	if(!proc.start(KProcess::DontCare))
	  KMessageBox::sorry(krApp,i18n("Can't open ")+"\""+term+"\"");
	
	chdir(save.local8Bit());
}

void ListPanelFunc::editFile(){
  QString name = panel->getCurrentName();
  if (name.isNull()) return;
	// verify read permissions
	krConfig->setGroup("Advanced");
	if(krConfig->readBoolEntry("Permission Check",_PermCheck) &&
     panel->files->vfs_getType() == "normal" &&
	  !panel->files->vfs_search(name)->vfile_isReadable()){
		KMessageBox::sorry(krApp,i18n("You do not have permission to read ")+name);
	  return;
	}
	
	if(panel->files->vfs_search(name)->vfile_isDir()){
		KMessageBox::sorry(krApp,i18n("You can't edit a directory"));
	  return;
	}
	
	KProcess proc;
	krConfig->setGroup("General");
	QString edit = krConfig->readEntry("Editor",_Editor);
	proc << edit << panel->files->vfs_getFile(name);
  if(!proc.start(KProcess::DontCare))
    KMessageBox::sorry(krApp,i18n("Can't open ")+"\""+edit+"\"");
}

void ListPanelFunc::moveFiles(){
  // check that the current file system support move
  if( !panel->files->supportMoveFrom ){
    KMessageBox::sorry(krApp,i18n("You can't move files from this file system"));
	  return;
  }

  // check that the you have write perm
	krConfig->setGroup("Advanced");
	if(krConfig->readBoolEntry("Permission Check",_PermCheck) &&
	  !panel->files->isWritable){
    KMessageBox::sorry(krApp,i18n("You do not have write permission to this directory"));
	  return;
  }

  QStringList fileNames;
	panel->getSelectedNames(&fileNames);
	if (fileNames.isEmpty()) return;  // safety

	QString dest = panel->otherPanel->getPath();

	krConfig->setGroup("Advanced");
	if( krConfig->readBoolEntry("Confirm Move",_ConfirmMove) ){
	  QString s;
    if (fileNames.count()==1 ) s = i18n("Move ")+fileNames.first()+" "+i18n("to")+":";
	  else s.sprintf(i18n("Move %d files to:").local8Bit(),fileNames.count() );

	  // ask the user for the copy dest
	  KChooseDir *chooser = new KChooseDir( 0,s,panel->otherPanel->getPath() );
	  dest=chooser->dest;
	  if(dest==QString::null) return; // the usr canceled
  }
	
  bool skipAll=false;
	for (QStringList::Iterator name = fileNames.begin(); name != fileNames.end();){
	  // verify write (delete) permissions
	  krConfig->setGroup("Advanced");
	  if(krConfig->readBoolEntry("Permission Check",_PermCheck) &&
		  !panel->files->vfs_search(*name)->vfile_isReadable()){
		  if( skipAll ) name=fileNames.remove(name);
		  else switch( KMessageBox::warningYesNoCancel(krApp,
	                 i18n("You do not have permission \nto move ")+(*name),
	                 QString::null,i18n("&Skip"),i18n("Skip &All")) ){
          case KMessageBox::Cancel : return; // stop operation
          case KMessageBox::No     : skipAll=true;// DONT BREAK !
          case KMessageBox::Yes    : name=fileNames.remove(name);
      }
	  }
	  else name++;
	}
	
	if (fileNames.isEmpty()) return; // nothing to copy
	
	KURL::List* fileUrls = panel->files->vfs_getFiles(&fileNames);
	
	// if we are not copyning to the other panel :
	if(panel->otherPanel->getPath() != dest){
    bool changed = false;
		if (dest.left(1) != "/"){
			dest = panel->files->vfs_workingDir()+"/"+dest;
			changed = true;
		}	
		// you can rename only *one* file not a batch,
	  // so a batch dest must alwayes be a directory
	  if((fileNames.count()>1) && (dest.right(1) != "/")) dest = dest+"/";
    QDir().mkdir(dest.left(dest.findRev("/")));
		KURL destUrl;
		destUrl.setPath(dest);
		KIO::Job* job = new KIO::CopyJob(*fileUrls,destUrl, KIO::CopyJob::Move,false,true );
		if (changed) connect(job,SIGNAL(result(KIO::Job*)),panel,SLOT(refresh()) );
  //else let the other panel do the dirty job
	}else{
		//check if copy is supported
    if(!panel->otherPanel->files->supportMoveTo){
      KMessageBox::sorry(krApp,i18n("You can't move files to this file system"));
	    return;
    }
    // check that the you have write perm
	  krConfig->setGroup("Advanced");
	  if(krConfig->readBoolEntry("Permission Check",_PermCheck) &&
	    !panel->otherPanel->files->isWritable){
      KMessageBox::sorry(krApp,i18n("You do not have write permission to this directory"));
	    return;
    }
    // finally..
    panel->otherPanel->files->vfs_addFiles(fileUrls,KIO::CopyJob::Move,panel);
  }
}

void ListPanelFunc::rename(){
  krConfig->setGroup("Advanced");
  QString fileName = panel->getCurrentName();
  if (fileName.isNull()) return;
  if(krConfig->readBoolEntry("Permission Check",_PermCheck)){
	  if(!panel->files->vfs_search(fileName)->vfile_isWriteable() ||
	     !panel->files->isWritable){
		  KMessageBox::sorry(krApp,i18n("You do not have permission to rename ")+fileName);
	    return;
    }
  } 			

	bool ok=false;
  QString newName =
		KLineEditDlg::getText(i18n("Rename ")+fileName+i18n(" to:"),fileName,&ok,krApp);

	// if the user canceled - quit
	if ( !ok || newName==fileName ) return;
	// if the name is already taken - quit
	if (panel->files->vfs_search(newName) != 0){
		KMessageBox::sorry(krApp,i18n("A directory or a file with this name already exists."));
		return;
	}	
	panel->nameToMakeCurrent = newName;
	// as always - the vfs do the job
	panel->files->vfs_rename(fileName,newName);
}

void ListPanelFunc::mkdir(){
	// check that the you have write perm
	krConfig->setGroup("Advanced");
	if(krConfig->readBoolEntry("Permission Check",_PermCheck) &&
	  !panel->files->isWritable){
    KMessageBox::sorry(krApp,i18n("You do not have write permission to this directory"));
	  return;
  }
	
	// ask the new dir name..
	bool ok=false;
  QString dirName =
		KLineEditDlg::getText(i18n("Directory's name:"),"",&ok,krApp);

	// if the user canceled - quit
	if ( !ok || dirName.isEmpty() ) return;
	
	// if the name is already taken - quit
	if (panel->files->vfs_search(dirName) != 0){
		KMessageBox::sorry(krApp,i18n("A directory or a file with this file already exists."));
		return;
	}	
	panel->nameToMakeCurrent = dirName;
	// as always - the vfs do the job
	panel->files->vfs_mkdir(dirName);
}

void ListPanelFunc::copyFiles() {
	QStringList fileNames;
	
	panel->getSelectedNames(&fileNames);
	if (fileNames.isEmpty()) return;  // safety	
	
  QString dest = panel->otherPanel->getPath();
	
	krConfig->setGroup("Advanced");
	if( krConfig->readBoolEntry("Confirm Copy",_ConfirmCopy) ){
    QString s;
    if (fileNames.count()==1 ) s = i18n("Copy ")+fileNames.first()+" "+i18n("to")+":";
	  else s.sprintf(i18n("Copy %d files to:").local8Bit(),fileNames.count() );
	  // ask the user for the copy dest
	  KChooseDir *chooser = new KChooseDir( 0,s,panel->otherPanel->getPath() );
	  dest=chooser->dest;
	  if(dest==QString::null) return; // the usr canceled
  }
  bool skipAll=false;
	for (QStringList::Iterator name = fileNames.begin(); name != fileNames.end();){
	  // verify write (delete) permissions
	  krConfig->setGroup("Advanced");
	  if(krConfig->readBoolEntry("Permission Check",_PermCheck) &&
		  !panel->files->vfs_search(*name)->vfile_isReadable()){
		  if( skipAll ) name=fileNames.remove(name);
		  else switch( KMessageBox::warningYesNoCancel(krApp,
	                 i18n("You do not have permission \nto copy ")+(*name),
	                 QString::null,i18n("&Skip"),i18n("Skip &All")) ){
          case KMessageBox::Cancel : return; // stop operation
          case KMessageBox::No     : skipAll=true;// DONT BREAK !
          case KMessageBox::Yes    : name=fileNames.remove(name);
      }
	  }
	  else name++;
	}

  if (fileNames.isEmpty()) return; // nothing to copy
	
	KURL::List* fileUrls = panel->files->vfs_getFiles(&fileNames);
	
	// if we are  not copying to the other panel :
	if(panel->otherPanel->getPath() != dest){
    bool refresh = false;
		if (dest.left(1) != "/"){
			 dest = panel->files->vfs_workingDir()+"/"+dest;
			 refresh =true;
		}
		// you can rename only *one* file not a batch,
	  // so a batch dest must alwayes be a directory
	  if((fileNames.count()>1) && (dest.right(1) != "/")) dest = dest+"/";
		QDir().mkdir(dest.left(dest.findRev("/")));
		KURL destUrl;
		destUrl.setPath(dest);
		KIO::Job* job = new KIO::CopyJob(*fileUrls,destUrl, KIO::CopyJob::Copy,false,true );
		if (refresh) connect(job,SIGNAL(result(KIO::Job*)),panel,SLOT(refresh()) );
  // let the other panel do the dirty job
	}else{
		//check if copy is supported
    if(!panel->otherPanel->files->supportCopyTo){
      KMessageBox::sorry(krApp,i18n("You can't copy files to this file system"));
	    return;
    }
    // check that the you have write perm
	  krConfig->setGroup("Advanced");
	  if(krConfig->readBoolEntry("Permission Check",_PermCheck) &&
	    !panel->otherPanel->files->isWritable){
      KMessageBox::sorry(krApp,i18n("You do not have write permission to this directory"));
	    return;
    }
    // finally..
    panel->otherPanel->files->vfs_addFiles(fileUrls,KIO::CopyJob::Copy,0);
  }
}

void ListPanelFunc::deleteFiles() {
 	QStringList fileNames;
 	QStringList::Iterator name;
  QDir dir;
  krConfig->setGroup("Advanced");
	bool permVerify = krConfig->readBoolEntry("Permission Check",_PermCheck);
	
  //check if delete is supported
  if(!panel->files->supportDelete){
    KMessageBox::sorry(krApp,i18n("You can't delete files from this file system"));
	  return;
  }

  // check that the you have write perm
	if(permVerify && !panel->files->isWritable){
    KMessageBox::sorry(krApp,i18n("You do not have write permission to this directory"));
	  return;
  }

  // first get the selected file names list
	panel->getSelectedNames(&fileNames);
	if(fileNames.isEmpty()) return;

	// now ask the user if he want to delete:
	krConfig->setGroup("Advanced");
	if( krConfig->readBoolEntry("Confirm Delete",_ConfirmDelete) ){
	  QString s,b;
	  if (fileNames.count()==1 )s=" "+fileNames.first()+" ?";
	  else s.sprintf( i18n(" %d files ?").local8Bit(),fileNames.count() );
	  krConfig->setGroup("General");
	  if( krConfig->readBoolEntry("Move To Trash",_MoveToTrash) &&
	    panel->files->vfs_getType() == "normal" ){
	    s = i18n("trash")+s;
	    b = i18n("&Trash");
	  }
	  else {
	    s = i18n("delete")+s;
	    b = i18n("&Delete");
	  }
	  // show message
	  if(KMessageBox::warningContinueCancel(krApp,i18n("Are you sure you want to ")+s
	    ,QString::null,b)
	    == KMessageBox::Cancel ) return;
  }
	//we want to warn the user about non empty dir
	// and files he don't have permission to delete
	krConfig->setGroup("Advanced");
	bool emptyDirVerify = krConfig->readBoolEntry("Confirm Unempty Dir",_ConfirmUnemptyDir);
  emptyDirVerify = ((emptyDirVerify)&&(panel->files->vfs_getType() == "normal"));
	bool skipAll = false;
	
	for( name = fileNames.begin(); name != fileNames.end(); ){
    vfile * vf = panel->files->vfs_search(*name);
	
		// verify write (delete) permissions
	  if( permVerify && !vf->vfile_isWriteable()){
	    if( skipAll ){
	      name = fileNames.remove(name);
        continue;
	    }
		  switch( KMessageBox::warningYesNoCancel(krApp,
	          i18n("You do not have permission \nto delete ")+(*name).latin1(),
	          QString::null,i18n("&Skip"),i18n("Skip &All")) ){
	      case KMessageBox::Cancel : return; // stop operation
        case KMessageBox::No     : skipAll = true; // DONT BREAK !
        case KMessageBox::Yes    : name = fileNames.remove(name);
                                   continue;
      }
	  }

		// verify non-empty dirs delete... (only for norml vfs)
		if ( emptyDirVerify && vf->vfile_isDir() && !vf->vfile_isSymLink() ){
			dir.setPath( panel->getPath()+"/"+(*name) );
			if ( dir.count() > 2 ){
			 	switch(KMessageBox::warningYesNoCancel( krApp,
								i18n("Directory ")+(*name).latin1()+i18n(" is not empty !\nSkip this one or Delete All ?"),
								QString::null,i18n("&Skip"),i18n("&Delete All")) ){
			   	case KMessageBox::Cancel : return;
			 	 	case KMessageBox::No     : emptyDirVerify = false; break;
			  	case KMessageBox::Yes    : name = fileNames.remove(name);
                 	                   continue;
			 	}
			}
		}
		++name;
	}

  if(fileNames.count() == 0) return;

  // after the delete return the cursor to the first unmarked
  // file above the current item;
  panel->prepareToDelete();

	// let the vfs do the job...
	panel->files->vfs_delFiles(&fileNames);
}

// this is done when you double click on a file
void ListPanelFunc::execute(QListViewItem *i) {
	if (!i) return;

	QString name=panel->getFilename(i);
	if(name==".."){
		dirUp();
		return;
	}

	vfile *vf=panel->files->vfs_search(name);
	if(vf == 0) return;
	QString origin= panel->files->vfs_getOrigin();

	// verify read permissions
	krConfig->setGroup("Advanced");
	if(krConfig->readBoolEntry("Permission Check",_PermCheck)){
    if(!vf->vfile_isReadable()){
      KMessageBox::sorry(krApp,i18n("You don't have premission to access this Directory / File !"));
	    return;
    }
	}
	QString type = vf->vfile_getMime().right(4);
 	if ( type == "-tbz" ) type = "zip2";
	if ( type == "-tgz" || type == "tarz" ) type = "gzip";
  if ( vf->vfile_getMime().contains("-rar") ) type = "-rar";

	if (vf->vfile_isDir()){
	  origin=="/"? origin+=name : origin+="/"+name;
	  panel->nameToMakeCurrent = QString::null;
		panel->refresh(origin);
	}
	else if(panel->files->vfs_getType() == "ftp" ){
	  if( !vf->vfile_isSymLink() ){
		  QString tmp = i18n("If you want to execute %1,\nTransfer it first to your computer.").arg(name);
      KMessageBox::sorry( krApp, tmp);
    }
	  else {
		  origin+="/"+name;
	    panel->nameToMakeCurrent = QString::null;
		  panel->refresh(origin);
		}
	}
	else if( KRarcHandler::arcHandled(type)){
  	changeVFS( type,panel->files->vfs_getFile(vf->vfile_getName()) );
	  // add warning to the backStack
	  if(backStack.last() != "//WARNING//" ) backStack.append("//WARNING//");
	}
  else {
      KURL url;
      url.setPath(panel->files->vfs_getFile(name));
      KRun::runURL(url, vf->vfile_getMime());
  }
}

void ListPanelFunc::dirUp(){
	QString origin= panel->virtualPath;
	
	//do we need to change vfs ?
	if( origin.right(1) == "\\" || panel->files->vfs_error()) { //yes
	  delete panel->files;
    panel->files = panel->vfsStack->pop();
    panel->files->blockSignals(false);
		// make the current archive the current item on the new list
		panel->nameToMakeCurrent =
			origin.mid(origin.findRev('/')+1,origin.length()-origin.findRev('/')-2);
		panel->files->vfs_refresh();
	  return;
	}
	
  // remove one dir from the path
	origin.truncate(origin.findRev('/'));
		
	// make the current dir the current item on the new list
	panel->nameToMakeCurrent = panel->virtualPath.right(panel->virtualPath.length()-origin.length()-1 );
	
	// check the '/' case
	if (origin=="") origin="/";
	
	// change dir..
	panel->refresh(origin);
}

void ListPanelFunc::changeVFS(QString type, QString origin){
	panel->nameToMakeCurrent = QString::null;
  vfs* v;
	if (type == "ftp")
    v = new ftp_vfs(origin,panel);
  else if( type == "-ace" || type == "-arj" || type == "-rpm" )
    v = new temp_vfs(origin,type,panel,panel->files->isWritable);
  else
    v = new arc_vfs(origin,type,panel,panel->files->isWritable);
   	
	if ( v->vfs_error() ){
			kdWarning() << "Failed to create vfs: " << origin.local8Bit() << endl;
			delete v;
			panel->refresh();
			return;
	}

	
	// save the current vfs
  panel->vfsStack->push(panel->files);
	panel->files->blockSignals(true);
  panel->files = v;
		
  while( origin.right(1) == "/" ) origin.truncate(origin.length()-1);

	if ( type != "ftp" ){
  	// refresh our newly created vfs
  	QString path = panel->virtualPath;
  	while( path.right(1) == "/" ) path.truncate(path.length()-1);
		QString name = origin.mid(origin.findRev('/'));
		panel->files->vfs_refresh( path+name+"\\" );
	}
}

void ListPanelFunc::pack() {
  QStringList fileNames;
  panel->getSelectedNames(&fileNames);
	if (fileNames.isEmpty()) return;  // safety
  bool skipAll = false;
  for (QStringList::Iterator name = fileNames.begin(); name != fileNames.end();){
    // verify read  permissions
	  krConfig->setGroup("Advanced");
	  if(krConfig->readBoolEntry("Permission Check",_PermCheck) &&
		  !panel->files->vfs_search(*name)->vfile_isReadable()){
		  if(skipAll) name = fileNames.remove(name);
		  else switch( KMessageBox::warningYesNoCancel(krApp,
	        i18n("You do not have permission to pack ")+(*name).latin1(),
	        QString::null,i18n("&Skip"),i18n("Skip &All")) ){
	        case KMessageBox::Cancel : return; // stop operation
          case KMessageBox::No     : skipAll = true;
          case KMessageBox::Yes    : name = fileNames.remove(name);
      }
    }
    else ++name;
  }
  if (fileNames.count() == 0) return; // nothing to pack

  // choose the default name
  QString defaultName = panel->virtualPath.right(panel->virtualPath.length()-panel->virtualPath.findRev('/')-1 );
  if (defaultName == "" || defaultName.right(1) == "\\" ) defaultName = "pack";
  if (fileNames.count() == 1) defaultName = fileNames.first();
  // ask the user for archive name and packer
  new PackGUI(defaultName,panel->otherPanel->virtualPath,fileNames.count(),fileNames.first());
  if (PackGUI::type == QString::null ) return; // the user canceled

  bool packToOtherPanel = (PackGUI::destination == panel->otherPanel->virtualPath );

  if ( PackGUI::destination.contains('\\')) // packing into archive
    if ( !packToOtherPanel ){
      KMessageBox::sorry(krApp,i18n("When Packing into archive - you must use the active directory"));
	    return;
    }
    else
      PackGUI::destination = panel->otherPanel->files->vfs_workingDir();

  QString arcFile = PackGUI::destination+"/"+PackGUI::filename+"."+PackGUI::type;

  krConfig->setGroup("Advanced");
	if(krConfig->readBoolEntry("Permission Check",_PermCheck) &&
    (packToOtherPanel && !panel->otherPanel->files->isWritable) || !QFileInfo(PackGUI::destination).isWritable()){
    KMessageBox::sorry(krApp,i18n("You don't have write permissions to the destination directory"));
	  return;
  }

  if(PackGUI::type != "zip" && QFileInfo(arcFile).exists()){
    if(KMessageBox::warningContinueCancel(krApp,
	        i18n("The Archive ")+PackGUI::filename+"."+PackGUI::type+
	        i18n(" already exists, Do you want to overwrite the archive ")+
	        i18n("(all data in previous archive will be lost)"),QString::null,i18n("&Overwite"))
	        == KMessageBox::Cancel ) return; // stop operation
  }
  // tell the user to wait
  krApp->startWaiting(i18n("Counting files to pack"), 0);

  // get the files to be packed:
  panel->files->vfs_getFiles(&fileNames);

  long long totalSize=0;
  long totalDirs=0, totalFiles=0;
  for(QStringList::Iterator file = fileNames.begin(); file != fileNames.end(); ++file){
    panel->files->vfs_calcSpace((*file),&totalSize,&totalFiles,&totalDirs);
  }

  // pack the files
  // we must chdir() first because we supply *names* not URL's
  QString save = getcwd(0,0);
  chdir(panel->files->vfs_workingDir().local8Bit());
  KRarcHandler::pack(fileNames,PackGUI::type,arcFile,totalFiles+totalDirs);
  chdir(save.local8Bit());

  if(packToOtherPanel) panel->otherPanel->refresh();
}

void ListPanelFunc::testArchive(){
  QString arcName = panel->getCurrentName();
  if (arcName.isNull()) return;
  if (arcName=="..") return; // safety
  QString type = panel->files->vfs_search(arcName)->vfile_getMime().right(4);

  // check we that archive is supported
  if( !KRarcHandler::arcSupported(type) ) {
    KMessageBox::sorry(krApp,i18n("%1, unknown archive type.").arg(arcName));
	  return;
  }

  // test the archive
  if ( KRarcHandler::test(panel->files->vfs_getFile(arcName),type) )
    KMessageBox::sorry(krApp,i18n("%1, test passed.").arg(arcName));
  else
    KMessageBox::error(krApp,i18n("%1, test failed !").arg(arcName));
}

void ListPanelFunc::unpack(){

  QStringList fileNames;
	panel->getSelectedNames(&fileNames);
	if (fileNames.isEmpty()) return;  // safety

  QString s;
  if( fileNames.count() == 1 )
    s = i18n("Unpack ")+fileNames[0]+" "+i18n("to")+":";
  else
    s = i18n("Unpack ")+i18n("%1 files").arg(fileNames.count())+i18n("to")+":";
	
	// ask the user for the copy dest
	KChooseDir *chooser = new KChooseDir( 0,s,panel->otherPanel->getPath() );
	QString dest=chooser->dest;
	if(dest==QString::null) return; // the usr canceled

	bool packToOtherPanel = (dest == panel->otherPanel->virtualPath );

  if ( dest.contains('\\') ) // unpacking into archive
    if ( !packToOtherPanel ){
      KMessageBox::sorry(krApp,i18n("When unpacking into archive - you must use the active directory"));
	    return;
    }
    else
      dest = panel->otherPanel->files->vfs_workingDir();

  krConfig->setGroup("Advanced");
	if(krConfig->readBoolEntry("Permission Check",_PermCheck) &&
    (packToOtherPanel && !panel->otherPanel->files->isWritable) || !QFileInfo(dest).isWritable()){
    KMessageBox::sorry(krApp,i18n("You don't have write permissions to the destination directory"));
	  return;
  }


  for(unsigned int i=0; i<fileNames.count(); ++i){
    QString arcName = fileNames[i];
    if (arcName.isNull()) return;
    if (arcName=="..") return; // safety

    if(krConfig->readBoolEntry("Permission Check",_PermCheck) &&
	    !panel->files->vfs_search(arcName)->vfile_isReadable()){
		  KMessageBox::sorry(krApp,i18n("You do not have permission to unpack ")+(arcName));
	    return;
    }

    QString mime = panel->files->vfs_search(arcName)->vfile_getMime();
    QString type = mime.right(4);
    if( mime.contains("-rar") ) type="-rar";

    // check we that archive is supported
    if( !KRarcHandler::arcSupported(type) ) {
      KMessageBox::sorry(krApp,i18n("%1, unknown archive type").arg(arcName));
	    continue;
    }
    // unpack the files
    KRarcHandler::unpack(panel->files->vfs_getFile(arcName),type,dest);
  }
  if(packToOtherPanel) panel->otherPanel->refresh();
}

void ListPanelFunc::calcSpace(){
  long long totalSize = 0;
  long totalFiles = 0;
  long totalDirs = 0;
  QStringList names;
  panel->getSelectedNames(&names);
  if(names.isEmpty()) return; // nothing to do

  // set the cursor to busy mode
  krApp->setCursor(KCursor::waitCursor());
	
	// ask the vfs to calculate the space for each name
  for (QStringList::Iterator name = names.begin(); name != names.end(); ++name)
    panel->files->vfs_calcSpace(*name,&totalSize,&totalFiles,&totalDirs);

  // show the results to the user...
 	krApp->setCursor(KCursor::arrowCursor());  // return the cursor to normal mode
  QString msg;
  QString fileName=((names.count()==1) ? (i18n("Name: ")+names.first()+"\n") : QString(""));
  msg=fileName+i18n("Total occupied space: %1\nin %2 directories and %3 files").
    arg(KIO::convertSize(totalSize)).arg(totalDirs).arg(totalFiles);
  KMessageBox::information(krApp,msg.latin1());
}

void ListPanelFunc::FTPDisconnect(){
  // you can disconnect only if connected !
  if( panel->files->vfs_getType() == "ftp" ){
    delete panel->files;
	  panel->files = panel->vfsStack->pop();
	  panel->files->blockSignals(false);
		krFTPDiss->setEnabled(false);
    krFTPNew->setEnabled(true);
    panel->nameToMakeCurrent = QString::null;
    panel->files->vfs_refresh();
  }
}

void ListPanelFunc::newFTPconnection(QString host){
  if (host==QString::null) {
    host=KRSpWidgets::newFTP();
    // if the user canceled - quit
	  if (host == QString::null) return;
  }
  krFTPDiss->setEnabled(true);
  changeVFS("ftp",host);
}

void ListPanelFunc::properties() {
  QStringList names;
  panel->getSelectedNames(&names);
  if ( names.isEmpty() ) return;  // no names...
	KURL::List* urls = panel->files->vfs_getFiles( &names );
	KFileItemList fi;
	
  for(unsigned int i=0 ; i < urls->count() ; ++i ) {
  	fi.append(new KFileItem( (mode_t)-1,(mode_t)-1,*(urls->at(i)) ));
  }
	// create a new url and get the file's mode
  KPropertiesDialog *dlg=new KPropertiesDialog(fi);
  connect(dlg,SIGNAL(applied()),panel,SLOT(refresh()));
}





//////////////////////////////////////////////////////////
//////		----------	Tree Panel -------------		////////
//////////////////////////////////////////////////////////


TreePanelFunc::TreePanelFunc(TreePanel *parent):panel(parent){}

void TreePanelFunc::terminal(){
  QString save = getcwd(0,0);
	chdir( panel->files->vfs_origin.local8Bit() );
	
  KProcess proc;
	krConfig->setGroup("General");
	QString term = krConfig->readEntry("Terminal",_Terminal);
	proc <<  term;
	if(!proc.start(KProcess::DontCare))
	  KMessageBox::sorry(krApp,i18n("Can't open ")+"\""+term+"\"");
	
	chdir(save.local8Bit());
}

void TreePanelFunc::mkdir(){
  // check that the you have write perm
	krConfig->setGroup("Advanced");
	///////////////////////////////
  if(krConfig->readBoolEntry("Permission Check",_PermCheck) &&
	  !panel->files->isWritable){
    KMessageBox::sorry(krApp,i18n("You do not have write permission to this directory"));
	  return;
  }
	
  // ask the new dir name..
	bool ok=false;
  QString dirName =
		KLineEditDlg::getText(i18n("Directory's name:"),"",&ok,krApp);

	// if the user canceled - quit
	if ( !ok || dirName.isEmpty() ) return;
	
	// if the name is already taken - quit
	if (QDir(panel->files->vfs_origin+"/"+dirName).exists()){
		KMessageBox::sorry(krApp,i18n("A directory or a file with this file already exists."));
		return;
	}	
	// as always - the vfs do the job
	panel->files->vfs_mkdir(dirName);
}

void TreePanelFunc::deleteFiles(){
  QStringList fileNames;
 	QDir dir;

  // check that the you have write perm
	krConfig->setGroup("Advanced");
	///////////////////////////////
	if(krConfig->readBoolEntry("Permission Check",_PermCheck) &&
	  !panel->files->isWritable){
    KMessageBox::sorry(krApp,i18n("You do not have write permission to this directory"));
	  return;
  }

  // now ask the user if he want to delete:
	krConfig->setGroup("Advanced");
	if( krConfig->readBoolEntry("Confirm Delete",_ConfirmDelete) ){
	  QString s,b;
	  krConfig->setGroup("General");
	  if( krConfig->readBoolEntry("Move To Trash",_MoveToTrash) ){
	    s = i18n("trash ")+panel->files->vfs_origin;
	    b = i18n("&Trash");
	  }
	  else {
	    s = i18n("delete ")+panel->files->vfs_origin;
	    b = i18n("&Delete");
	  }
	  // show message
	  if(KMessageBox::warningContinueCancel(krApp,i18n("Are you sure you want to ")+s
	    ,QString::null,b)
	    == KMessageBox::Cancel ) return;
  }

  //we want to warn the user about non empty dir
	krConfig->setGroup("Advanced");
	// verify non-empty dirs delete...
	dir.setPath( panel->files->vfs_origin );
	if ( dir.count() > 2 && krConfig->readBoolEntry("Confirm Unempty Dir",_ConfirmUnemptyDir) )
	  if( KMessageBox::warningContinueCancel( krApp,
		  i18n("Directory ")+(panel->files->vfs_origin)+i18n(" is not empty ! Are you sure ? "),
			QString::null,i18n("&Delete") ) == KMessageBox::Cancel) return;
				
  // let the vfs do the job...
	panel->files->vfs_delFiles(&fileNames);
}

void TreePanelFunc::rename(){
  krConfig->setGroup("Advanced");
  QString fileName = panel->getCurrentName();
  if (fileName.isNull()) return;

  if(krConfig->readBoolEntry("Permission Check",_PermCheck)){
	  if( !panel->files->isWritable ){
		  KMessageBox::sorry(krApp,i18n("You do not have permission to rename ")+fileName);
	    return;
    }
  } 			

	bool ok=false;
  QString newName =
		KLineEditDlg::getText(i18n("Rename ")+fileName+i18n(" to:"),fileName,&ok,krApp);

	// if the user canceled - quit
	if ( !ok || newName==fileName ) return;
  // if the name is already taken - quit
	QString path = panel->files->vfs_origin.left(panel->files->vfs_origin.findRev('/')-1);
	if ( QDir(path+"/"+newName).exists() ) {
		KMessageBox::sorry(krApp,i18n("A directory with this name already exists."));
		return;
	}	
	// as always - the vfs do the job
	panel->files->vfs_rename(fileName,newName);
}

void TreePanelFunc::copyFiles(){
	QStringList fileNames;
	
	panel->getSelectedNames(&fileNames);
	if (fileNames.isEmpty()) return;  // safety
  QString dest = panel->otherPanel->getPath();
	
	krConfig->setGroup("Advanced");
	if( krConfig->readBoolEntry("Confirm Copy",_ConfirmCopy) ){
    QString s = i18n("Copy ")+panel->files->vfs_origin+" "+i18n("to")+":";
	
	  // ask the user for the copy dest
	  KChooseDir *chooser = new KChooseDir( 0,s,panel->otherPanel->getPath() );
	  dest=chooser->dest;
	  if(dest==QString::null) return; // the user canceled
  }
  	
	// verify read permissions
	krConfig->setGroup("Advanced");
	if(krConfig->readBoolEntry("Permission Check",_PermCheck) &&
    !panel->files->supportMoveFrom){  // vfs_supportMoveFrom == readable
	  KMessageBox::sorry(krApp,i18n("You do not have permission to move ")+(panel->files->vfs_origin));
	  return; // stop operation
	}

  KURL url;
  url.setPath(panel->files->vfs_origin);
  KURL::List fileUrls;
  fileUrls.append(url);
	
	// if we are not copying to the other panel :
	if(panel->otherPanel->getPath() != dest){
    new KIO::CopyJob(fileUrls,dest, KIO::CopyJob::Copy,false,true );
  // let the other panel do the dirty job
	}else{
		//check if copy is supported
    if(!panel->otherPanel->files->supportCopyTo){
      KMessageBox::sorry(krApp,i18n("You can't copy files to this file system"));
	    return;
    }
    // check that the you have write perm
	  krConfig->setGroup("Advanced");
	  ///////////////////////////////
	  if(krConfig->readBoolEntry("Permission Check",_PermCheck) &&
	    !panel->otherPanel->files->isWritable){
      KMessageBox::sorry(krApp,i18n("You do not have write permission to this directory"));
	    return;
    }
    // finally..
    panel->otherPanel->files->vfs_addFiles(&fileUrls,KIO::CopyJob::Copy,0);
  }
}

void TreePanelFunc::moveFiles(){
  // check that the you have write (delete) perm
	krConfig->setGroup("Advanced");
	///////////////////////////////
	if(krConfig->readBoolEntry("Permission Check",_PermCheck) &&
	  !panel->files->isWritable){
    KMessageBox::sorry(krApp,i18n("You do not have write permission to this directory"));
	  return;
  }
	
	QString dest = panel->otherPanel->getPath();
	
	krConfig->setGroup("Advanced");
	if( krConfig->readBoolEntry("Confirm Move",_ConfirmMove) ){
	  QString s = i18n("Move ")+panel->files->vfs_origin+" "+i18n("to")+":";
	  // ask the user for the copy dest
	  KChooseDir *chooser = new KChooseDir( 0,s,panel->otherPanel->getPath() );
	  dest=chooser->dest;
	  if(dest==QString::null) return; // the usr canceled
  }
  // verify read permissions
	krConfig->setGroup("Advanced");
	if(krConfig->readBoolEntry("Permission Check",_PermCheck) &&
    !panel->files->supportMoveFrom){  // vfs_supportMoveFrom == readable
	  KMessageBox::sorry(krApp,i18n("You do not have permission to move ")+(panel->files->vfs_origin));
	  return; // stop operation
	}
	
	KURL url;
	url.setPath(panel->files->vfs_origin);
	KURL::List fileUrls;
	fileUrls.append(url);
	
	// if we are not copyning to the other panel :
	if(panel->otherPanel->getPath() != dest){
   new KIO::CopyJob(fileUrls,dest, KIO::CopyJob::Move,false,true );
  //else let the other panel do the dirty job
	}else{
		//check if copy is supported
    if(!panel->otherPanel->files->supportMoveTo){
      KMessageBox::sorry(krApp,i18n("You can't move files to this file system"));
	    return;
    }
    // check that the you have write perm
	  krConfig->setGroup("Advanced");
	  ///////////////////////////////
	  if(krConfig->readBoolEntry("Permission Check",_PermCheck) &&
	    !panel->otherPanel->files->isWritable){
      KMessageBox::sorry(krApp,i18n("You do not have write permission to this directory"));
	    return;
    }
    // finally..
    panel->otherPanel->files->vfs_addFiles(&fileUrls,KIO::CopyJob::Move,panel);
  }
}

void TreePanelFunc::pack() {

  // verify read  permissions
	krConfig->setGroup("Advanced");
	if(krConfig->readBoolEntry("Permission Check",_PermCheck) &&
	   !panel->files->supportMoveFrom){
	
	  KMessageBox::sorry(krApp,i18n("You do not have permission to pack ")+(panel->files->vfs_origin));
	  return; // stop operation
  }

  // choose the default name
  QString defaultName = panel->getCurrentName();
  if (defaultName.isNull()) defaultName = "archive";
  if (defaultName == "/" ) defaultName = "System";
  // ask the user for archive name and packer
  new PackGUI(defaultName,panel->otherPanel->virtualPath,1,panel->getCurrentName());
  QString packer;
  if (PackGUI::type == QString::null ) return; // the user canceled
  // set the right packer to do the job
  else if (PackGUI::type ==  "zip" ){
     packer = "zip -ry";
  }
  else if (PackGUI::type ==  "tar" ){
    packer = "tar -cvf";
  }
  else if (PackGUI::type ==  "tar.gz" ){
    packer = "tar -cvzf";
  }
  else if (PackGUI::type ==  "tar.bz2" ){
    packer = "tar -cvjf";
  }
  else if (PackGUI::type ==  "rar" ){
    packer = "rar -r a";
  }

  bool packToOtherPanel = (PackGUI::destination == panel->otherPanel->virtualPath );

  if ( PackGUI::destination.contains('\\')) // packing into archive
    if( !packToOtherPanel ){
      KMessageBox::sorry(krApp,i18n("When Packing into archive - you must use the active directory"));
	    return;
    }
    else
      PackGUI::destination = panel->otherPanel->files->vfs_workingDir();

  QString arcFile = PackGUI::destination+"/"+PackGUI::filename+"."+PackGUI::type;

  krConfig->setGroup("Advanced");
	if(krConfig->readBoolEntry("Permission Check",_PermCheck) &&
    (packToOtherPanel && !panel->otherPanel->files->isWritable) || !QFileInfo(PackGUI::destination).isWritable()){
    KMessageBox::sorry(krApp,i18n("You don't have write permissions to the destination directory"));
	  return;
  }

  if(PackGUI::type != "zip" && QFileInfo(arcFile).exists()){
    if( KMessageBox::warningContinueCancel(krApp,i18n("The Archive")+PackGUI::filename+"."+PackGUI::type+
        i18n(" already exists, Do you want to overwrite the archive ")+
        i18n("(all data in previous archive will be lost)"),QString::null,i18n("&Overwrite"))
        == KMessageBox::Cancel) return; // stop operation
	}
	// tell the user to wait
  krApp->startWaiting(i18n("Preparing to pack"),0);

  // prepare to pack
  KShellProcess proc;
  proc << packer << "\""+arcFile+"\"";

  long long totalSize=0;
  long totalDirs=0, totalFiles=0;
  proc << "*";
  panel->files->vfs_calcSpace(panel->files->vfs_origin,&totalSize,&totalFiles,&totalDirs);
  QString save = getcwd(0,0);
  chdir(panel->files->vfs_origin.local8Bit());

  // tell the user to wait
  krApp->startWaiting(i18n("Packing Directory"),totalFiles+totalDirs);
  connect(&proc,SIGNAL(receivedStdout(KProcess*,char*,int)),
          krApp, SLOT(incProgress(KProcess*,char*,int)) );

  // start the packing process
  proc.start(KProcess::NotifyOnExit,KProcess::AllOutput);
  while( proc.isRunning() ) qApp->processEvents(); // busy wait - need to find something better...
  krApp->stopWait();

  // check the return value
  if( !proc.normalExit() )
	   KMessageBox::error(krApp,i18n("Error","Failed To Pack Files !"));

  chdir(save.local8Bit());
  if(packToOtherPanel) panel->otherPanel->refresh();
}

void TreePanelFunc::calcSpace(){
  long long totalSize = 0;
  long totalFiles = 0, totalDirs = 0;
  QStringList names;

  // set the cursor to busy mode
  krApp->setCursor(KCursor::waitCursor());// tell the user to wait

  // ask the vfs to calculate the space for the current dir
  panel->files->vfs_calcSpace(panel->files->vfs_origin,&totalSize,&totalFiles,&totalDirs);

  // show the results to the user...
  krApp->setCursor(KCursor::arrowCursor());  // set the cursor to normal mode
	QString msg;
  QString fileName = i18n("Name: ")+panel->getCurrentName();
  msg=fileName+i18n("Total occupied space: %1\nin %2 directories and %3 files").
  arg(KIO::convertSize(totalSize)).arg(totalDirs).arg(totalFiles);
  KMessageBox::information(krApp,msg.local8Bit());
}

void TreePanelFunc::properties() {
  QStringList names;
  if ( panel->getCurrentName().isNull() ) return; // no dir...
	KFileItemList fi;
	KURL url;
  url.setPath( panel->files->vfs_getFile(panel->getCurrentName()) );
  fi.append(new KFileItem((mode_t)-1,(mode_t)-1,url));
	// create a new url and get the file's mode
  KPropertiesDialog *dlg=new KPropertiesDialog(fi);
  connect(dlg,SIGNAL(applied()),panel,SLOT(refresh()));
}

#include "panelfunc.moc"
