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
#include <qeventloop.h>
// KDE Includes
#include <klocale.h>
#include <kprocess.h>
#include <kpropertiesdialog.h>
#include <kopenwith.h>
#include <kmessagebox.h>
#include <kcursor.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kurl.h>
#include <krun.h>
#include <kinputdialog.h>
#include <kdebug.h>
// Krusader Includes
#include "panelfunc.h"
#include "krcalcspacedialog.h"
#include "krdetailedview.h"
#include "../krusader.h"
#include "../krslots.h"
#include "../defaults.h"
#include "../VFS/vfile.h"
#include "../VFS/vfs.h"
#include "../VFS/krarchandler.h"
#include "../VFS/krpermhandler.h"
#include "../VFS/krvfshandler.h"
#include "../Dialogs/packgui.h"
#include "../Dialogs/krdialogs.h"
#include "../Dialogs/krpleasewait.h"
#include "../Dialogs/krspwidgets.h"
#include "../KViewer/krviewer.h"
#include "../MountMan/kmountman.h"
#include "../resources.h"
#include "../krservices.h"
#include "../GUI/syncbrowsebutton.h"


#define OTHER_FUNC	(panel->otherPanel->func)


//////////////////////////////////////////////////////////
//////		----------	List Panel -------------		////////
//////////////////////////////////////////////////////////

ListPanelFunc::ListPanelFunc( ListPanel *parent ) :
panel( parent ), inRefresh( false ), vfsP(0){
  urlStack.push( "file:/" );
  connect( &delayTimer, SIGNAL(timeout()), this, SLOT(doOpenUrl()));
}

void ListPanelFunc::openUrl( const QString& url,const QString& nameToMakeCurrent) {
  openUrl( vfs::fromPathOrURL(url),nameToMakeCurrent);
}

void ListPanelFunc::openUrl( const KURL& url,const QString& nameToMakeCurrent) {
  //kdDebug() << "openUrl: " << url.url() << endl;
  
  //prevents that the sync-browsing circles itself to death
  static bool bMaster = true;
  
  // first the other dir, then the active! Else the focus changes and the other becomes active
  if ( panel->syncBrowseButton->state() == SYNCBROWSE_CD && bMaster) {
	bMaster = false;
	//do sync-browse stuff....
	//QString relative_path = KURL::relativeURL( panel->getPath()+"/", url.url() );
	//kdDebug() << "Sync: from: " << panel->getPath()+"/" << " to: " << url.url() << " -> relativeURL: " << relative_path << endl;
	//OTHER_FUNC->openUrl( relative_path );
	OTHER_FUNC->openUrl( KURL::relativeURL( panel->getPath()+"/", url.url() ) );	// the trailing slash is nessesary because krusader provides Dir's without it
	bMaster = true;
  }

	// check for special cases:
  if( !url.isValid() ){
		if( url.url() == "~" ){
			openUrl(QDir::homeDirPath());
    }
    else if( !url.url().startsWith("/") ){  // possible relative URL
			KURL u = files()->vfs_getOrigin();
			u.addPath(url.url());
			openUrl(u);
		}
		else panel->slotStartUpdate();  // refresh the panel
		return;
  }
	// change the cursor to busy
  krApp->setCursor( KCursor::waitCursor() );

  panel->slotFocusOnMe();

  // clear the view - to avoid a repaint crash
  panel->view->clear();

  if( !nameToMakeCurrent.isEmpty() ){
		panel->view->setNameToMakeCurrent( nameToMakeCurrent );
  }

  vfs* v = 0;
  if( !urlStack.top().equals(url) ) urlStack.push( url );
  while( true ){
		KURL u = urlStack.pop();
		//u.adjustPath(-1); // remove trailing "/"
		u.cleanPath(); // Resolves "." and ".." components in path.
		v = KrVfsHandler::getVfs(u,panel,files());
    if( !v ) continue; //this should not happen !
		if( v != vfsP ){
      delete vfsP;
			vfsP = v; // v != 0 so this is safe
		}
    if( vfsP->vfs_refresh(u) ) break; // we have a valid refreshed URL now
  }
	// update the urls stack
	if( !files()->vfs_getOrigin().equals(urlStack.top()) ){
		urlStack.push( files()->vfs_getOrigin() );
	}
	// disconnect older signals
	disconnect(files(), SIGNAL(addedVfile(vfile* )), 0, 0); 
	disconnect(files(), SIGNAL(updatedVfile(vfile* )), 0, 0);
	disconnect(files(), SIGNAL(deletedVfile(const QString& )), 0, 0);
		
	// connect to the vfs's dirwatch signals
	connect(files(), SIGNAL(addedVfile(vfile* )), 
		panel, SLOT(slotItemAdded(vfile* )));
	connect(files(), SIGNAL(updatedVfile(vfile* )), 
		panel, SLOT(slotItemUpdated(vfile* )));
	connect(files(), SIGNAL(deletedVfile(const QString& )), 
		panel, SLOT(slotItemDeleted(const QString&)));
	
	// on local file system change the working directory
	if( files()->vfs_getType() == vfs::NORMAL )
		chdir( files()->vfs_getOrigin().path().local8Bit() );
}

void ListPanelFunc::delayedOpenUrl( const KURL& url ) {
   delayURL = url;               /* this function is useful for FTP url-s and bookmarks */
   delayTimer.start( 0, true );  /* to avoid qApp->processEvents() deadlock situaltion */
}

void ListPanelFunc::doOpenUrl() {
   openUrl( delayURL );
}

void ListPanelFunc::goBack() {
  if ( urlStack.isEmpty() ) return ;

  urlStack.pop();
  openUrl( urlStack.top(), files()->vfs_getOrigin().fileName());

  if ( urlStack.isEmpty() ) krBack->setEnabled( false );
}

void ListPanelFunc::redirectLink() {
  if ( files() ->vfs_getType() != vfs::NORMAL ) {
    KMessageBox::sorry( krApp, i18n( "You can edit links only on local file systems" ) );
    return ;
  }

  vfile *vf = files() ->vfs_search( panel->getCurrentName() );
  if ( !vf )
    return ;

  QString file = files() ->vfs_getFile( vf->vfile_getName() ).path(-1);
  QString currentLink = vf->vfile_getSymDest();
  if ( currentLink.isEmpty() ) {
    KMessageBox::sorry( krApp, i18n( "The current file is not a link, so i can't redirect it." ) );
    return ;
  }

  // ask the user for a new destination
  bool ok = false;
  QString newLink =
    KInputDialog::getText( i18n("Link Redirection"),
		                       i18n("Please enter the new link destination:"), currentLink, &ok, krApp );

  // if the user canceled - quit
  if ( !ok || newLink == currentLink )
    return ;
  // delete the current link
  if ( unlink( file.local8Bit() ) == -1 ) {
    KMessageBox::sorry( krApp, i18n( "Can't remove old link: " ) + file );
    return ;
  }
  // try to create a new symlink
  if ( symlink( newLink.local8Bit(), file.local8Bit() ) == -1 ) {
    KMessageBox::/* --=={ Patch by Heiner <h.eichmann@gmx.de> }==-- */sorry( krApp, i18n( "Failed to create a new link: " ) + file );
    return ;
  }
}

void ListPanelFunc::krlink( bool sym ) {
  if ( files() ->vfs_getType() != vfs::NORMAL ) {
    KMessageBox::sorry( krApp, i18n( "You can create links only on local file systems" ) );
    return ;
  }

  QString name = panel->getCurrentName();

  // ask the new link name..
  bool ok = false;
  QString linkName =
    KInputDialog::getText( i18n("New link"),i18n( "Create a new link to: " ) + name, name, &ok, krApp );

  // if the user canceled - quit
  if ( !ok || linkName == name )
    return ;

  // if the name is already taken - quit
  if ( files() ->vfs_search( linkName ) != 0 ) {
    KMessageBox::sorry( krApp, i18n( "A directory or a file with this name already exists." ) );
    return ;
  }

  if ( linkName.left( 1 ) != "/" )
    linkName = files()->vfs_workingDir() + "/" + linkName;

  if ( linkName.contains( "/" ) )
    name = files()->vfs_getFile( name ).path(-1);

  if ( sym ) {
    if ( symlink( name.local8Bit(), linkName.local8Bit() ) == -1 )
      KMessageBox::sorry( krApp, i18n( "Failed to create a new symlink: " ) + linkName +
                          i18n( " To: " ) + name );
  } else {
    if ( link( name.local8Bit(), linkName.local8Bit() ) == -1 )
      KMessageBox::sorry( krApp, i18n( "Failed to create a new link: " ) + linkName +
                          i18n( " To: " ) + name );
  }
}

void ListPanelFunc::view() {
  QString fileName = panel->getCurrentName();
  if ( fileName.isNull() )
    return ;

  // if we're trying to view a directory, just exit
  vfile * vf = files()->vfs_search( fileName );
  if ( !vf || vf->vfile_isDir() )
    return ;
  if ( !vf->vfile_isReadable() ) {
    KMessageBox::sorry(0,i18n("No permissions to view this file."));
    return;
  }
  // call KViewer.
  KrViewer::view( files()->vfs_getFile( fileName ) );
  // nothing more to it!
}

void ListPanelFunc::terminal() {
  QString save = getcwd( 0, 0 );
  chdir( panel->realPath.local8Bit() );

  KProcess proc;
  krConfig->setGroup( "General" );
  QString term = krConfig->readEntry( "Terminal", _Terminal );
  proc << KrServices::separateArgs( term );
  
  if( term.contains( "konsole" ) )   /* KDE 3.2 bug (konsole is killed by pressing Ctrl+C) */
  {                                  /* Please remove the patch if the bug is corrected */
    proc << "&";
    proc.setUseShell( true );
  }
  
  if ( !proc.start( KProcess::DontCare ) )
    KMessageBox::sorry( krApp, i18n( "Can't open " ) + "\"" + term + "\"" );

  chdir( save.local8Bit() );
}

void ListPanelFunc::editFile() {
  QString name = panel->getCurrentName();
  if ( name.isNull() )
    return ;

  if ( files()->vfs_search( name )->vfile_isDir() ) {
    KMessageBox::sorry( krApp, i18n( "You can't edit a directory" ) );
    return ;
  }

  if ( !files()->vfs_search( name )->vfile_isReadable() ) {
    KMessageBox::sorry(0,i18n("No permissions to edit this file."));
    return;
  }

  KrViewer::edit( files() ->vfs_getFile(name) );
}

void ListPanelFunc::moveFiles() {
  QStringList fileNames;
  panel->getSelectedNames( &fileNames );
  if ( fileNames.isEmpty() )
    return ;  // safety

  QString dest = panel->otherPanel->getPath();
                 /* --=={ Patch by Heiner <h.eichmann@gmx.de> }==-- */

  //krConfig->setGroup( "Archives" );
  //if ( !krConfig->readBoolEntry( "Allow Move Into Archive", _MoveIntoArchive ) )
  //{  
    QString destProtocol = panel->otherPanel->func->files()->vfs_getOrigin().protocol();
    if( destProtocol == "krarc" || destProtocol == "tar" || destProtocol == "zip" )
    {
      KMessageBox::sorry( krApp, i18n( "Moving into archive is disabled" ) );
      return ;
    }
  //}
                 
  krConfig->setGroup( "Advanced" );
  if ( krConfig->readBoolEntry( "Confirm Move", _ConfirmMove ) ) {
    QString s;
    if ( fileNames.count() == 1 )
      s = i18n( "Move " ) + fileNames.first() + " " + i18n( "to" ) + ":";
    else
      s.sprintf( i18n( "Move %d files to:" ).local8Bit(), fileNames.count() );

    // ask the user for the copy dest
    KChooseDir *chooser = new KChooseDir( 0, s, panel->otherPanel->getPath() );
    dest = chooser->dest;
    if ( dest == QString::null )
      return ; // the usr canceled
  }

  if ( fileNames.isEmpty() )
    return ; // nothing to copy

  KURL::List* fileUrls = files() ->vfs_getFiles( &fileNames );

  // after the delete return the cursor to the first unmarked
  // file above the current item;
  panel->prepareToDelete();

  // if we are not moving to the other panel :
  if ( panel->otherPanel->getPath() != dest ) {
    if ( dest.left( 1 ) != "/" ) {
      dest = files() ->vfs_workingDir() + "/" + dest;
    }
    // you can rename only *one* file not a batch,
    // so a batch dest must alwayes be a directory
    if ( ( fileNames.count() > 1 ) && ( dest.right( 1 ) != "/" ) )
      dest = dest + "/";
    QDir().mkdir( dest.left( dest.findRev( "/" ) ) );
    KURL destUrl;
    destUrl.setPath( dest );
    KIO::Job* job = new KIO::CopyJob( *fileUrls, destUrl, KIO::CopyJob::Move, false, true );
    // refresh our panel when done
		connect(job,SIGNAL(result(KIO::Job*)),this,SLOT( refresh() ) );
    // and if needed the other panel as well
    if( dest.startsWith(panel->otherPanel->getPath()) )
			connect(job,SIGNAL(result(KIO::Job*)),panel->otherPanel->func,SLOT( refresh() ) );	

  } else { // let the other panel do the dirty job
    //check if copy is supported
    if ( !otherFunc() ->files() ->vfs_isWritable() ) {
      KMessageBox::sorry( krApp, i18n( "You can't move files to this file system" ) );
      return ;
    }
    // finally..
    otherFunc() ->files() ->vfs_addFiles( fileUrls, KIO::CopyJob::Move, files() );
  }
}

// called from SLOTS to begin the renaming process
void ListPanelFunc::rename() {
  panel->view->renameCurrentItem();
}

// called by signal itemRenamed() from the view to complete the renaming process
void ListPanelFunc::rename(const QString &oldname, const QString &newname) {
  if (oldname == newname) return; // do nothing
  panel->view->setNameToMakeCurrent( newname );
  // as always - the vfs do the job
  files() ->vfs_rename( oldname, newname );  
}

void ListPanelFunc::mkdir() {
  // ask the new dir name..
  bool ok = false;
  QString dirName =
    KInputDialog::getText(i18n("New directory"),i18n( "Directory's name:" ), "", &ok, krApp );

  // if the user canceled - quit
  if ( !ok || dirName.isEmpty() )
    return ;

  // if the name is already taken - quit
  if ( files()->vfs_search(dirName) ) {
    KMessageBox::sorry( krApp, i18n( "A directory or a file with this name already exists." ) );
    return ;
  }

  panel->view->setNameToMakeCurrent( dirName );
  // as always - the vfs do the job
  files() ->vfs_mkdir( dirName );
}

void ListPanelFunc::copyFiles() {
  QStringList fileNames;
  panel->getSelectedNames( &fileNames );
  if ( fileNames.isEmpty() )
    return ;  // safety

  QString dest = panel->otherPanel->getPath();

  // confirm copy
  krConfig->setGroup( "Advanced" );
  if ( krConfig->readBoolEntry( "Confirm Copy", _ConfirmCopy ) ) {
    QString s;
    if ( fileNames.count() == 1 )
      s = i18n( "Copy " ) + fileNames.first() + " " + i18n( "to" ) + ":";
    else
      s.sprintf( i18n( "Copy %d files to:" ).local8Bit(), fileNames.count() );
    // ask the user for the copy dest
    KChooseDir *chooser = new KChooseDir( 0, s, panel->otherPanel->getPath() );
    dest = chooser->dest;
    if ( dest == QString::null )
      return ; // the usr canceled
  }

  KURL::List* fileUrls = files() ->vfs_getFiles( &fileNames );

  // if we are  not copying to the other panel :
  if ( panel->otherPanel->getPath() != dest ) {
    bool refresh = false;
    if ( dest.left( 1 ) != "/" ) {
      dest = files() ->vfs_workingDir() + "/" + dest;
      if ( !dest.contains( "/" ) )
        refresh = true;
    }
    // you can rename only *one* file not a batch,
    // so a batch dest must alwayes be a directory
    if ( ( fileNames.count() > 1 ) && ( dest.right( 1 ) != "/" ) )
      dest = dest + "/";
    QDir().mkdir( dest.left( dest.findRev( "/" ) ) );
    KURL destUrl;
    destUrl.setPath( dest );
    KIO::Job* job = new KIO::CopyJob( *fileUrls, destUrl, KIO::CopyJob::Copy, false, true );
    if ( refresh )
      connect( job, SIGNAL( result( KIO::Job* ) ), SLOTS, SLOT( refresh() ) );
    // let the other panel do the dirty job
  } else {
    //check if copy is supported
    if ( !otherFunc() ->files() ->vfs_isWritable() ) {
      KMessageBox::sorry( krApp, i18n( "You can't copy files to this file system" ) );
      return ;
    }
    // finally..
    otherFunc() ->files() ->vfs_addFiles( fileUrls, KIO::CopyJob::Copy, 0 );
  }
}

void ListPanelFunc::deleteFiles() {
  // check that the you have write perm
  if ( !files() ->vfs_isWritable() ) {
    KMessageBox::sorry( krApp, i18n( "You do not have write permission to this directory" ) );
    return ;
  }

  // first get the selected file names list
  QStringList fileNames;
  panel->getSelectedNames( &fileNames );
  if ( fileNames.isEmpty() )
    return ;

  // now ask the user if he want to delete:
  krConfig->setGroup( "Advanced" );
  if ( krConfig->readBoolEntry( "Confirm Delete", _ConfirmDelete ) ) {
    QString s, b;
    if ( fileNames.count() == 1 )
      s = " " + fileNames.first() + " ?";
    else
      s.sprintf( i18n( " %d files ?" ).local8Bit(), fileNames.count() );
    krConfig->setGroup( "General" );
    if ( krConfig->readBoolEntry( "Move To Trash", _MoveToTrash ) &&
         files() ->vfs_getType() == vfs::NORMAL ) {
      s = i18n( "trash" ) + s;
      b = i18n( "&Trash" );
    } else {
      s = i18n( "delete" ) + s;
      b = i18n( "&Delete" );
    }
    // show message
    if ( KMessageBox::warningContinueCancel( krApp, i18n( "Are you sure you want to " ) + s
                                             , QString::null, b )
         == KMessageBox::Cancel )
      return ;
  }
  //we want to warn the user about non empty dir
  // and files he don't have permission to delete
  krConfig->setGroup( "Advanced" );
  bool emptyDirVerify = krConfig->readBoolEntry( "Confirm Unempty Dir", _ConfirmUnemptyDir );
  emptyDirVerify = ( ( emptyDirVerify ) && ( files() ->vfs_getType() == vfs::NORMAL ) );

  QDir dir;
  for ( QStringList::Iterator name = fileNames.begin(); name != fileNames.end(); ) {
    vfile * vf = files() ->vfs_search( *name );

    // verify non-empty dirs delete... (only for norml vfs)
    if ( emptyDirVerify && vf->vfile_isDir() && !vf->vfile_isSymLink() ) {
      dir.setPath( panel->getPath() + "/" + ( *name ) );
      if ( dir.count() > 2 ) {
        switch ( KMessageBox::warningYesNoCancel( krApp,
                                                  i18n( "Directory " ) + ( *name ) + i18n( " is not empty !\nSkip this one or Delete All ?" ),
                                                  QString::null, i18n( "&Skip" ), i18n( "&Delete All" ) ) ) {
            case KMessageBox::Cancel :
            return ;
            case KMessageBox::No :
            emptyDirVerify = false;
            break;
            case KMessageBox::Yes :
            name = fileNames.remove( name );
            continue;
        }
      }
    }
    ++name;
  }

  if ( fileNames.count() == 0 )
    return ;  // nothing to delete

  // after the delete return the cursor to the first unmarked
  // file above the current item;
  panel->prepareToDelete();

  // let the vfs do the job...
  files() ->vfs_delFiles( &fileNames );
}

// this is done when you double click on a file
void ListPanelFunc::execute( QString& name ) {
  if ( name == ".." ) {
    dirUp();
    return ;
  }

  vfile *vf = files() ->vfs_search( name );
  if ( vf == 0 )
    return ;
  KURL origin = files() ->vfs_getOrigin();

  QString type = vf->vfile_getMime().right( 4 );
  if ( vf->vfile_getMime().contains( "-rar" ) )
    type = "-rar";

  if ( vf->vfile_isDir() ) {
    //origin.addPath(name);
    origin = files()->vfs_getFile( name );
    panel->view->setNameToMakeCurrent( QString::null );
    openUrl( origin );
  } else if ( KRarcHandler::arcHandled( type ) && origin.isLocalFile() ) {
    KURL path = files()->vfs_getFile(vf->vfile_getName());
    if ( type == "-tbz" || type == "-tgz" || type == "tarz" || type == "-tar" ){
			path.setProtocol("tar");
		} else {
			path.setProtocol("krarc");
    }
    openUrl( path );
	} else {
    KURL url = files()->vfs_getFile( name );
    KRun::runURL( url, vf->vfile_getMime() );
  }
}

void ListPanelFunc::dirUp() {
	openUrl(files()->vfs_getOrigin().upURL(), files()->vfs_getOrigin().fileName());
}

void ListPanelFunc::pack() {
  QStringList fileNames;
  panel->getSelectedNames( &fileNames );
  if ( fileNames.isEmpty() )
    return ;  // safety

  if ( fileNames.count() == 0 )
    return ; // nothing to pack

  // choose the default name
  QString defaultName = panel->virtualPath.right( panel->virtualPath.length() - panel->virtualPath.findRev( '/' ) - 1 );
  if ( defaultName == "" )
    defaultName = "pack";
  if ( fileNames.count() == 1 )
    defaultName = fileNames.first();
  // ask the user for archive name and packer
  new PackGUI( defaultName, panel->otherPanel->virtualPath, fileNames.count(), fileNames.first() );
  if ( PackGUI::type == QString::null )
    return ; // the user canceled

  bool packToOtherPanel = ( PackGUI::destination == panel->otherPanel->virtualPath );

  QString arcFile = PackGUI::destination + "/" + PackGUI::filename + "." + PackGUI::type;

  if ( PackGUI::type != "zip" && QFileInfo( arcFile ).exists() ) {
    if ( KMessageBox::warningContinueCancel( krApp,
                                             i18n( "The Archive " ) + PackGUI::filename + "." + PackGUI::type +
                                             i18n( " already exists, Do you want to overwrite the archive " ) +
                                             i18n( "(all data in previous archive will be lost)" ), QString::null, i18n( "&Overwite" ) )
         == KMessageBox::Cancel )
      return ; // stop operation
  }
  // tell the user to wait
  krApp->startWaiting( i18n( "Counting files to pack" ), 0 );

  // get the files to be packed:
  files() ->vfs_getFiles( &fileNames );

  KIO::filesize_t totalSize = 0;
  unsigned long totalDirs = 0, totalFiles = 0;
  for ( QStringList::Iterator file = fileNames.begin(); file != fileNames.end(); ++file ) {
    files() ->vfs_calcSpace( ( *file ), &totalSize, &totalFiles, &totalDirs );
  }

  // pack the files
  // we must chdir() first because we supply *names* not URL's
  QString save = getcwd( 0, 0 );
  chdir( files() ->vfs_workingDir().local8Bit() );
  KRarcHandler::pack( fileNames, PackGUI::type, arcFile, totalFiles + totalDirs );
  chdir( save.local8Bit() );

  if ( packToOtherPanel )
    panel->otherPanel->func->refresh();
}

void ListPanelFunc::testArchive() {
  QString arcName = panel->getCurrentName();
  if ( arcName.isNull() )
    return ;
  if ( arcName == ".." )
    return ; // safety
  QString type = files() ->vfs_search( arcName ) ->vfile_getMime().right( 4 );

  // check we that archive is supported
  if ( !KRarcHandler::arcSupported( type ) ) {
    KMessageBox::sorry( krApp, i18n( "%1, unknown archive type." ).arg( arcName ) );
    return ;
  }

  // test the archive
  if ( KRarcHandler::test( files()->vfs_getFile(arcName).path(-1), type ) )
    KMessageBox::information( krApp, i18n( "%1, test passed." ).arg( arcName ) );
  else
    KMessageBox::error( krApp, i18n( "%1, test failed !" ).arg( arcName ) );
}

void ListPanelFunc::unpack() {

  QStringList fileNames;
  panel->getSelectedNames( &fileNames );
  if ( fileNames.isEmpty() )
    return ;  // safety

  QString s;
  if ( fileNames.count() == 1 )
    s = i18n( "Unpack " ) + fileNames[ 0 ] + " " + i18n( "to" ) + ":";
  else
    s = i18n( "Unpack " ) + i18n( "%1 files" ).arg( fileNames.count() ) + i18n( "to" ) + ":";

  // ask the user for the copy dest
  KChooseDir *chooser = new KChooseDir( 0, s, panel->otherPanel->getPath() );
  QString dest = chooser->dest;
  if ( dest == QString::null )
    return ; // the usr canceled

  bool packToOtherPanel = ( dest == panel->otherPanel->virtualPath );

  for ( unsigned int i = 0; i < fileNames.count(); ++i ) {
    QString arcName = fileNames[ i ];
    if ( arcName.isNull() )
      return ;
    if ( arcName == ".." )
      return ; // safety

    QString mime = files() ->vfs_search( arcName ) ->vfile_getMime();
    QString type = mime.right( 4 );
    if ( mime.contains( "-rar" ) )
      type = "-rar";

    // check we that archive is supported
    if ( !KRarcHandler::arcSupported( type ) ) {
      KMessageBox::sorry( krApp, i18n( "%1, unknown archive type" ).arg( arcName ) );
      continue;
    }
    // unpack the files
    KRarcHandler::unpack( files()->vfs_getFile(arcName).path(-1), type, dest );
  }
  if ( packToOtherPanel )
    panel->otherPanel->func->refresh();
}

void ListPanelFunc::calcSpace() {
	QStringList items;
	panel->view->getSelectedItems(&items);
	if ( items.isEmpty() )
	{
	  panel->view->selectAllIncludingDirs();
	  panel->view->getSelectedItems(&items);
	  if ( items.isEmpty() ) return ; // nothing to do
	}

	KrCalcSpaceDialog calc(krApp, panel, items, false);
	calc.exec();
        KrDetailedView * view = dynamic_cast<KrDetailedView *> ( panel->view );
        if (!view)
                return;
        for ( QStringList::ConstIterator it = items.begin(); it != items.end(); ++it )
        {
                KrDetailedViewItem * viewItem = dynamic_cast<KrDetailedViewItem *> ( view->findItemByName(*it) );
                if (viewItem)
                     viewItem->repaintItem();
        }
  panel->slotUpdateTotals();
}

bool ListPanelFunc::calcSpace(const QStringList & items,KIO::filesize_t & totalSize,unsigned long & totalFiles,unsigned long & totalDirs) {
	KrCalcSpaceDialog calc(krApp, panel, items, true);
	calc.exec();
	totalSize = calc.getTotalSize();
	totalFiles = calc.getTotalFiles();
	totalDirs = calc.getTotalDirs();
	return !calc.wasCanceled();
}

void ListPanelFunc::FTPDisconnect() {
  // you can disconnect only if connected !
  if ( files() ->vfs_getType() == vfs::FTP ) {
    krFTPDiss->setEnabled( false );
    panel->view->setNameToMakeCurrent( QString::null );
    openUrl(panel->realPath); // open the last local URL
  }
}

void ListPanelFunc::newFTPconnection() {
  QString url;
  url = KRSpWidgets::newFTP();
  // if the user canceled - quit
  if ( url.isEmpty() ) return ;
   
  krFTPDiss->setEnabled( true );
  openUrl(url );
}

void ListPanelFunc::properties() {
  QStringList names;
  panel->getSelectedNames( &names );
  if ( names.isEmpty() )
    return ;  // no names...
  KFileItemList fi;
  fi.setAutoDelete(true);

  for ( unsigned int i = 0 ; i < names.count() ; ++i ) {
    vfile* vf = files()->vfs_search( names[i] );
    if( !vf ) continue;    
    //KURL url = files()->vfs_getFile( names[i] );
    fi.append( new KFileItem(vf->vfile_getEntry(),files()->vfs_getOrigin(),false,true) );
  }
  
  if( fi.isEmpty() ) return;
  
  // Show the properties dialog
  KPropertiesDialog *dlg = new KPropertiesDialog( fi );
  connect( dlg, SIGNAL( applied() ), SLOTS, SLOT( refresh() ) );
}

void ListPanelFunc::refreshActions() {
  vfs::VFS_TYPE vfsType = files() ->vfs_getType();
  //  set up actions
  krMultiRename->setEnabled( vfsType == vfs::NORMAL );  // batch rename
  krProperties ->setEnabled( vfsType == vfs::NORMAL || vfsType == vfs::FTP ); // file properties
  krFTPDiss    ->setEnabled( vfsType == vfs::FTP );     // disconnect an FTP session
  /*
    krUnpack->setEnabled(true);                            // unpack archive
    krTest->setEnabled(true);                              // test archive
    krSelect->setEnabled(true);                            // select a group by filter
    krSelectAll->setEnabled(true);                         // select all files
    krUnselect->setEnabled(true);                          // unselect by filter
    krUnselectAll->setEnabled( true);                      // remove all selections
    krInvert->setEnabled(true);                            // invert the selection
    krFTPConnect->setEnabled(true);                        // connect to an ftp
    krFTPNew->setEnabled(true);                            // create a new connection
    krAllFiles->setEnabled(true);                          // show all files in list
    krCustomFiles->setEnabled(true);                       // show a custom set of files
    krBack->setEnabled(func->canGoBack());                 // go back
    krRoot->setEnabled(true);                              // go all the way up
  	krExecFiles->setEnabled(true);                         // show only executables
  */
}

ListPanelFunc::~ListPanelFunc() {
  // clear the view - to avoid a repaint crash
  panel->view->clear();
  delete files(); // delete all vfs objects
}

vfs* ListPanelFunc::files() {
	if( !vfsP ) vfsP = KrVfsHandler::getVfs("/",panel,0);
	return vfsP;
}

#include "panelfunc.moc"
