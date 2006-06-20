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
#include <qclipboard.h> 
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
#include <kio/netaccess.h>
#include <kstandarddirs.h>
#include <ktempdir.h> 
#include <kurlrequester.h>
#include <kprocio.h>
#include <kdesktopfile.h>
// Krusader Includes
#include "panelfunc.h"
#include "krcalcspacedialog.h"
#include "krdetailedview.h"
#include "../krusader.h"
#include "../krslots.h"
#include "../defaults.h"
#include "../VFS/vfile.h"
#include "../VFS/vfs.h"
#include "../VFS/virt_vfs.h"
#include "../VFS/krarchandler.h"
#include "../VFS/krpermhandler.h"
#include "../VFS/krvfshandler.h"
#include "../VFS/preservingcopyjob.h"
#include "../VFS/virtualcopyjob.h"
#include "../Dialogs/packgui.h"
#include "../Dialogs/krdialogs.h"
#include "../Dialogs/krpleasewait.h"
#include "../Dialogs/krspwidgets.h"
#include "../Dialogs/checksumdlg.h"
#include "../KViewer/krviewer.h"
#include "../resources.h"
#include "../krservices.h"
#include "../GUI/syncbrowsebutton.h"
#include "../Queue/queue_mgr.h"
#include "krdrag.h"
#include <kurldrag.h>

//////////////////////////////////////////////////////////
//////          ----------      List Panel -------------                ////////
//////////////////////////////////////////////////////////

ListPanelFunc::ListPanelFunc( ListPanel *parent ) :
panel( parent ), inRefresh( false ), vfsP( 0 ) {
	urlStack.push( "file:/" );
	connect( &delayTimer, SIGNAL( timeout() ), this, SLOT( doOpenUrl() ) );
}

void ListPanelFunc::openUrl( const QString& url, const QString& nameToMakeCurrent ) {
	openUrl( vfs::fromPathOrURL( 
		// KURLRequester is buggy: it should return a string containing "/home/shie/downloads"
		// but it returns "~/downloads" which is parsed incorrectly by vfs::fromPathOrURL.
		// replacedPath should replace ONLY $HOME and environment variables
		panel->origin->completionObject()->replacedPath(url) )
		, nameToMakeCurrent );
}

void ListPanelFunc::immediateOpenUrl( const KURL& urlIn ) {
	KURL url = urlIn;
	url.cleanPath();

	// check for special cases first - don't refresh here !
	// you may call openUrl or vfs_refresh()
	if ( !url.isValid() ) {
		if ( url.url() == "~" ) {
			return openUrl( QDir::homeDirPath() );
		} else if ( !url.url().startsWith( "/" ) ) {
			// possible relative URL - translate to full URL
			url = files() ->vfs_getOrigin();
			url.addPath( urlIn.url() );
			//kdDebug()<< urlIn.url() << "," << url.url() <<endl;
		} else {
			panel->slotStartUpdate();  // refresh the panel
			return ;
		}
	}

	// if we are not refreshing to current URL
	bool is_equal_url = files() ->vfs_getOrigin().equals( url, true );
	
	if ( !is_equal_url ) {
		// change the cursor to busy
		panel->setCursor( KCursor::waitCursor() );
	}

	if ( !nameToMakeCurrent.isEmpty() ) {
		panel->view->setNameToMakeCurrent( nameToMakeCurrent );
		// if the url we're refreshing into is the current one, then the
		// partial url will not generate the needed signals to actually allow the
		// view to use nameToMakeCurrent. do it here instead (patch by Thomas Jarosch)
		if ( is_equal_url ) {
		    panel->view->setCurrentItem( nameToMakeCurrent );
		    panel->view->makeItemVisible( panel->view->getCurrentKrViewItem() );
		}		
	}

	vfs* v = 0;
	if ( !urlStack.top().equals( url ) )
		urlStack.push( url );
	while ( true ) {
		KURL u = urlStack.pop();
		//u.adjustPath(-1); // remove trailing "/"
		u.cleanPath(); // Resolves "." and ".." components in path.
		v = KrVfsHandler::getVfs( u, panel, files() );
		if ( !v )
			continue; //this should not happen !
		if ( v != vfsP ) {
			if( vfsP->vfs_canDelete() )                
				delete vfsP;
			else {
				connect( vfsP, SIGNAL( deleteAllowed() ), vfsP, SLOT( deleteLater() ) );
				vfsP->vfs_requestDelete();                                
			}
			vfsP = v; // v != 0 so this is safe
		}
		connect( files(), SIGNAL(startJob(KIO::Job* )),
				panel, SLOT(slotJobStarted(KIO::Job* )));
		if ( vfsP->vfs_refresh( u ) )
			break; // we have a valid refreshed URL now
		if ( vfsP == 0 )  // the object was deleted during vfs_refresh? Hoping the best...
			return;                
		// prevent repeated error messages
		if ( vfsP->vfs_isDeleting() )
			break;                
		vfsP->vfs_setQuiet( true );
	}
	vfsP->vfs_setQuiet( false );

	// update the urls stack
	if ( !files() ->vfs_getOrigin().equals( urlStack.top() ) ) {
		urlStack.push( files() ->vfs_getOrigin() );
	}
	// disconnect older signals
	disconnect( files(), SIGNAL( addedVfile( vfile* ) ), 0, 0 );
	disconnect( files(), SIGNAL( updatedVfile( vfile* ) ), 0, 0 );
	disconnect( files(), SIGNAL( deletedVfile( const QString& ) ), 0, 0 );
	disconnect( files(), SIGNAL( cleared() ), 0, 0 );
	// connect to the vfs's dirwatch signals
	connect( files(), SIGNAL( addedVfile( vfile* ) ),
	         panel, SLOT( slotItemAdded( vfile* ) ) );
	connect( files(), SIGNAL( updatedVfile( vfile* ) ),
	         panel, SLOT( slotItemUpdated( vfile* ) ) );
	connect( files(), SIGNAL( deletedVfile( const QString& ) ),
	         panel, SLOT( slotItemDeleted( const QString& ) ) );
	connect( files(), SIGNAL( cleared() ),
	         panel, SLOT( slotCleared() ) );
	
	// on local file system change the working directory
	if ( files() ->vfs_getType() == vfs::NORMAL )
		chdir( files() ->vfs_getOrigin().path().local8Bit() );
}

void ListPanelFunc::openUrl( const KURL& url, const QString& nameToMakeCurrent ) {
	panel->inlineRefreshCancel();
	// first the other dir, then the active! Else the focus changes and the other becomes active
	if ( panel->syncBrowseButton->state() == SYNCBROWSE_CD ) {
		// prevents that the sync-browsing circles itself to death
		static bool inSync = false;
		if( ! inSync ){
			inSync = true;
			//do sync-browse stuff....
			KURL otherDir = OTHER_PANEL->virtualPath();
			OTHER_FUNC->files() ->vfs_setQuiet( true );
			// the trailing slash is nessesary because krusader provides Dir's without it
			// we can't use openUrl because the delay don't allow a check if the panel has realy changed!
			OTHER_FUNC->immediateOpenUrl( KURL::relativeURL( panel->virtualPath().url() + "/", url.url() ) );
			OTHER_FUNC->files() ->vfs_setQuiet( false );
			// now we need to test ACTIVE_PANEL because the openURL has changed the active panel!!
			if ( ACTIVE_PANEL->virtualPath().equals( otherDir ) ) {
				// deactivating the sync-browse if syncbrowse not possible
				panel->syncBrowseButton->setOn( false );
			}
			inSync = false;
		}
	}
	this->nameToMakeCurrent = nameToMakeCurrent;
	delayURL = url;               /* this function is useful for FTP url-s and bookmarks */
	delayTimer.start( 0, true );  /* to avoid qApp->processEvents() deadlock situaltion */
}

void ListPanelFunc::refresh() {
	openUrl(panel->virtualPath()); // re-read the files
}

void ListPanelFunc::doOpenUrl() {
	immediateOpenUrl( delayURL );
}

void ListPanelFunc::goBack() {
	if ( urlStack.isEmpty() )
		return ;

	if ( urlStack.top().equals( files() ->vfs_getOrigin() ) )
		urlStack.pop();
	openUrl( urlStack.top(), files() ->vfs_getOrigin().fileName() );

	if ( urlStack.isEmpty() )
		krBack->setEnabled( false );
}

void ListPanelFunc::redirectLink() {
	if ( files() ->vfs_getType() != vfs::NORMAL ) {
		KMessageBox::sorry( krApp, i18n( "You can edit links only on local file systems" ) );
		return ;
	}

	vfile *vf = files() ->vfs_search( panel->getCurrentName() );
	if ( !vf )
		return ;

	QString file = files() ->vfs_getFile( vf->vfile_getName() ).path( -1 );
	QString currentLink = vf->vfile_getSymDest();
	if ( currentLink.isEmpty() ) {
		KMessageBox::sorry( krApp, i18n( "The current file is not a link, so I can't redirect it." ) );
		return ;
	}

	// ask the user for a new destination
	bool ok = false;
	QString newLink =
	    KInputDialog::getText( i18n( "Link Redirection" ),
	                           i18n( "Please enter the new link destination:" ), currentLink, &ok, krApp );

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
		KMessageBox:: /* --=={ Patch by Heiner <h.eichmann@gmx.de> }==-- */sorry( krApp, i18n( "Failed to create a new link: " ) + file );
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
	    KInputDialog::getText( i18n( "New link" ), i18n( "Create a new link to: " ) + name, name, &ok, krApp );

	// if the user canceled - quit
	if ( !ok || linkName == name )
		return ;

	// if the name is already taken - quit
	if ( files() ->vfs_search( linkName ) != 0 ) {
		KMessageBox::sorry( krApp, i18n( "A directory or a file with this name already exists." ) );
		return ;
	}

	if ( linkName.left( 1 ) != "/" )
		linkName = files() ->vfs_workingDir() + "/" + linkName;

	if ( linkName.contains( "/" ) )
		name = files() ->vfs_getFile( name ).path( -1 );

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
	vfile * vf = files() ->vfs_search( fileName );
	if ( !vf || vf->vfile_isDir() )
		return ;
	if ( !vf->vfile_isReadable() ) {
		KMessageBox::sorry( 0, i18n( "No permissions to view this file." ) );
		return ;
	}
	// call KViewer.
	KrViewer::view( files() ->vfs_getFile( fileName ) );
	// nothing more to it!
}

void ListPanelFunc::terminal() {
	QString save = getcwd( 0, 0 );
	chdir( panel->realPath().local8Bit() );

	KProcess proc;
	krConfig->setGroup( "General" );
	QString term = krConfig->readEntry( "Terminal", _Terminal );
	proc << KrServices::separateArgs( term );

	if ( term.contains( "konsole" ) )    /* KDE 3.2 bug (konsole is killed by pressing Ctrl+C) */
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

	if ( files() ->vfs_search( name ) ->vfile_isDir() ) {
		KMessageBox::sorry( krApp, i18n( "You can't edit a directory" ) );
		return ;
	}

	if ( !files() ->vfs_search( name ) ->vfile_isReadable() ) {
		KMessageBox::sorry( 0, i18n( "No permissions to edit this file." ) );
		return ;
	}

	KrViewer::edit( files() ->vfs_getFile( name ) );
}

void ListPanelFunc::moveFiles() {
	PreserveMode pmode = PM_DEFAULT;

	QStringList fileNames;
	panel->getSelectedNames( &fileNames );
	if ( fileNames.isEmpty() )
		return ;  // safety

	KURL dest = panel->otherPanel->virtualPath();
	KURL virtualBaseURL;

	QString destProtocol = dest.protocol();
	if ( destProtocol == "krarc" || destProtocol == "tar" || destProtocol == "zip" ) {
		KMessageBox::sorry( krApp, i18n( "Moving into archive is disabled" ) );
		return ;
	}

	krConfig->setGroup( "Advanced" );
	if ( krConfig->readBoolEntry( "Confirm Move", _ConfirmMove ) ) {
		bool preserveAttrs = krConfig->readBoolEntry( "PreserveAttributes", _PreserveAttributes );
		QString s;

  if( fileNames.count() == 1 )
    s = i18n("Move %1 to:").arg(fileNames.first());
  else
    s = i18n("Move %n file to:", "Move %n files to:", fileNames.count());

		// ask the user for the copy dest
		virtualBaseURL = getVirtualBaseURL();
		dest = KChooseDir::getDir(s, dest, panel->virtualPath(), preserveAttrs, virtualBaseURL);
		if ( dest.isEmpty() ) return ; // the user canceled
		if( preserveAttrs )
			pmode = PM_PRESERVE_ATTR;
		else
			pmode = PM_NONE;
	}

	if ( fileNames.isEmpty() )
		return ; // nothing to copy

	KURL::List* fileUrls = files() ->vfs_getFiles( &fileNames );

	// after the delete return the cursor to the first unmarked
	// file above the current item;
	panel->prepareToDelete();

	if( !virtualBaseURL.isEmpty() ) {
		// keep the directory structure for virtual paths
		VirtualCopyJob *vjob = new VirtualCopyJob( &fileNames, files(), dest, virtualBaseURL, pmode, KIO::CopyJob::Move, false, true );
		connect( vjob, SIGNAL( result( KIO::Job* ) ), this, SLOT( refresh() ) );
		if ( dest.equals( panel->otherPanel->virtualPath(), true ) )
			connect( vjob, SIGNAL( result( KIO::Job* ) ), panel->otherPanel->func, SLOT( refresh() ) );
	}
	// if we are not moving to the other panel :
	else if ( !dest.equals( panel->otherPanel->virtualPath(), true ) ) {
		// you can rename only *one* file not a batch,
		// so a batch dest must alwayes be a directory
		if ( fileNames.count() > 1 ) dest.adjustPath(1);
		KIO::Job* job = PreservingCopyJob::createCopyJob( pmode, *fileUrls, dest, KIO::CopyJob::Move, false, true );
		job->setAutoErrorHandlingEnabled( true );
		// refresh our panel when done
		connect( job, SIGNAL( result( KIO::Job* ) ), this, SLOT( refresh() ) );
		// and if needed the other panel as well
		if ( dest.equals( panel->otherPanel->virtualPath(), true ) )
			connect( job, SIGNAL( result( KIO::Job* ) ), panel->otherPanel->func, SLOT( refresh() ) );

	} else { // let the other panel do the dirty job
		//check if copy is supported
		if ( !otherFunc() ->files() ->vfs_isWritable() ) {
			KMessageBox::sorry( krApp, i18n( "You can't move files to this file system" ) );
			return ;
		}
		// finally..
		otherFunc() ->files() ->vfs_addFiles( fileUrls, KIO::CopyJob::Move, files(), "", pmode );
	}
}

// called from SLOTS to begin the renaming process
void ListPanelFunc::rename() {
	panel->view->renameCurrentItem();
}

// called by signal itemRenamed() from the view to complete the renaming process
void ListPanelFunc::rename( const QString &oldname, const QString &newname ) {
	if ( oldname == newname )
		return ; // do nothing
	panel->view->setNameToMakeCurrentIfAdded( newname );
	// as always - the vfs do the job
	files() ->vfs_rename( oldname, newname );
}

void ListPanelFunc::mkdir() {
	// ask the new dir name..
	bool ok = false;
	QString dirName =
	    KInputDialog::getText( i18n( "New directory" ), i18n( "Directory's name:" ), "", &ok, krApp );

	// if the user canceled - quit
	if ( !ok || dirName.isEmpty() )
		return ;

	QStringList dirTree = QStringList::split( "/", dirName );

	for ( QStringList::Iterator it = dirTree.begin(); it != dirTree.end(); ++it ) {
		// check if the name is already taken
		if ( files() ->vfs_search( *it ) ) {
			// if it is the last dir to be created - quit
			if ( *it == dirTree.last() ) {
				KMessageBox::sorry( krApp, i18n( "A directory or a file with this name already exists." ) );
				return ;
			}
			// else go into this dir
			else {
				immediateOpenUrl( *it );
				continue;
			}
		}

		panel->view->setNameToMakeCurrent( *it );
		// as always - the vfs do the job
		files() ->vfs_mkdir( *it );
		if ( dirTree.count() > 1 )
			immediateOpenUrl( *it );
	} // for
}

KURL ListPanelFunc::getVirtualBaseURL() {
	if( files()->vfs_getType() != vfs::VIRT || otherFunc()->files()->vfs_getType() == vfs::VIRT )
		return KURL();
	
	QStringList fileNames;
	panel->getSelectedNames( &fileNames );
	
	KURL::List* fileUrls = files() ->vfs_getFiles( &fileNames );
	if( fileUrls->count() == 0 )
		return KURL();
	
	KURL base = (*fileUrls)[ 0 ].upURL();
	
	if( base.protocol() == "virt" ) // is it a virtual subfolder?
		return KURL();          // --> cannot keep the directory structure
	
	for( unsigned i=1; i < fileUrls->count(); i++ ) {
		if( base.isParentOf( (*fileUrls)[ i ] ) )
			continue;
		if( base.protocol() != (*fileUrls)[ i ].protocol() )
			return KURL();
		
		do {
			KURL oldBase = base;
			base = base.upURL();
			if( oldBase.equals( base, true ) )
				return KURL();
			if( base.isParentOf( (*fileUrls)[ i ] ) )
				break;
		}while( true );
	}
	return base;
}

void ListPanelFunc::copyFiles() {
	PreserveMode pmode = PM_DEFAULT;

	QStringList fileNames;
	panel->getSelectedNames( &fileNames );
	if ( fileNames.isEmpty() )
		return ;  // safety

	KURL dest = panel->otherPanel->virtualPath();
	KURL virtualBaseURL;

	// confirm copy
	krConfig->setGroup( "Advanced" );
	if ( krConfig->readBoolEntry( "Confirm Copy", _ConfirmCopy ) ) {
		bool preserveAttrs = krConfig->readBoolEntry( "PreserveAttributes", _PreserveAttributes );
		QString s;

  if( fileNames.count() == 1 )
    s = i18n("Copy %1 to:").arg(fileNames.first());
  else
    s = i18n("Copy %n file to:", "Copy %n files to:", fileNames.count());

		// ask the user for the copy dest
		virtualBaseURL = getVirtualBaseURL();
		dest = KChooseDir::getDir(s, dest, panel->virtualPath(), preserveAttrs, virtualBaseURL );
		if ( dest.isEmpty() ) return ; // the user canceled
		if( preserveAttrs )
			pmode = PM_PRESERVE_ATTR;
		else
			pmode = PM_NONE;
	}

	KURL::List* fileUrls = files() ->vfs_getFiles( &fileNames );

	if( !virtualBaseURL.isEmpty() ) {
		// keep the directory structure for virtual paths
		VirtualCopyJob *vjob = new VirtualCopyJob( &fileNames, files(), dest, virtualBaseURL, pmode, KIO::CopyJob::Copy, false, true );
		connect( vjob, SIGNAL( result( KIO::Job* ) ), this, SLOT( refresh() ) );
		if ( dest.equals( panel->otherPanel->virtualPath(), true ) )
			connect( vjob, SIGNAL( result( KIO::Job* ) ), panel->otherPanel->func, SLOT( refresh() ) );
	}
	// if we are  not copying to the other panel :
	else if ( !dest.equals( panel->otherPanel->virtualPath(), true ) ) {
		// you can rename only *one* file not a batch,
		// so a batch dest must alwayes be a directory
		if ( fileNames.count() > 1 ) dest.adjustPath(1);
		KIO::Job* job = PreservingCopyJob::createCopyJob( pmode, *fileUrls, dest, KIO::CopyJob::Copy, false, true );
		job->setAutoErrorHandlingEnabled( true );
		if ( dest.equals( panel->virtualPath(), true ) ||
			dest.upURL().equals( panel->virtualPath(), true ) )
			// refresh our panel when done
			connect( job, SIGNAL( result( KIO::Job* ) ), this, SLOT( refresh() ) );
	// let the other panel do the dirty job
	} else {
		//check if copy is supported
		if ( !otherFunc() ->files() ->vfs_isWritable() ) {
			KMessageBox::sorry( krApp, i18n( "You can't copy files to this file system" ) );
			return ;
		}
		// finally..
		otherFunc() ->files() ->vfs_addFiles( fileUrls, KIO::CopyJob::Copy, 0, "", pmode );
	}
}

void ListPanelFunc::deleteFiles(bool reallyDelete) {
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

	krConfig->setGroup( "General" );
	bool trash = krConfig->readBoolEntry( "Move To Trash", _MoveToTrash );
	// now ask the user if he want to delete:
	krConfig->setGroup( "Advanced" );
	if ( krConfig->readBoolEntry( "Confirm Delete", _ConfirmDelete ) ) {
		QString s, b;

		if ( !reallyDelete && trash && files() ->vfs_getType() == vfs::NORMAL ) {
			s = i18n( "Do you really want to move this item to the trash?", "Do you really want to move these %n items to the trash?", fileNames.count() );
			b = i18n( "&Trash" );
		} else if( files() ->vfs_getType() == vfs::VIRT && files()->vfs_getOrigin().equals( KURL("virt:/"), true ) ) {
			s = i18n( "Do you really want to delete this URL collection (URLs inside won't be deleted)?", "Do you really want to delete these URL collections (URLs inside won't be deleted)?", fileNames.count() );
			b = i18n( "&Delete" );
		} else if( files() ->vfs_getType() == vfs::VIRT ) {
			s = i18n( "Do you really want to delete this item (physically, not just removing from the URL collection)?", "Do you really want to delete these %n items (physically, not just removing from the URL collection)?", fileNames.count() );
			b = i18n( "&Delete" );
		} else {
			s = i18n( "Do you really want to delete this item?", "Do you really want to delete these %n items?", fileNames.count() );
			b = i18n( "&Delete" );
		}

		// show message
		// note: i'm using continue and not yes/no because the yes/no has cancel as default button
		if ( KMessageBox::warningContinueCancelList( krApp, s, fileNames,
                                                     i18n( "Warning" ), b ) != KMessageBox::Continue )
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

		// verify non-empty dirs delete... (only for normal vfs)
		if ( emptyDirVerify && vf->vfile_isDir() && !vf->vfile_isSymLink() ) {
			dir.setPath( panel->virtualPath().path() + "/" + ( *name ) );
			if ( dir.entryList(QDir::All | QDir::AccessMask).count() > 2 ) {
				switch ( KMessageBox::warningYesNoCancel( krApp,
				                                          i18n( "Directory " ) + ( *name ) + i18n( " is not empty!\nSkip this one or Delete All?" ),
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
	if (reallyDelete) {
		// if reallyDelete, then make sure nothing gets moved to trash
		krConfig->setGroup("General");
		krConfig->writeEntry( "Move To Trash", false );
	}
	files() ->vfs_delFiles( &fileNames );
	if (reallyDelete) {
		krConfig->setGroup("General");
		krConfig->writeEntry( "Move To Trash", trash);
	}
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

	QString protocol = origin.isLocalFile() ? KrServices::registerdProtocol( vf->vfile_getMime() ) : "";

	if ( protocol == "tar" || protocol == "krarc" ) {
		bool encrypted;
		QString type = KRarcHandler::getType( encrypted, vf->vfile_getUrl().path(), vf->vfile_getMime(), false );
		if ( !KRarcHandler::arcHandled( type ) )   // if the specified archive is disabled delete the protocol
			protocol = "";
	}

	if ( vf->vfile_isDir() ) {
		origin = files() ->vfs_getFile( name );
		panel->view->setNameToMakeCurrent( QString::null );
		openUrl( origin );
	} else if ( !protocol.isEmpty() ) {
		KURL path = files() ->vfs_getFile( vf->vfile_getName() );
		path.setProtocol( protocol );
		openUrl( path );
	} else {
		KURL url = files() ->vfs_getFile( name );
		KFileItem kfi( vf->vfile_getEntry(), url,true );
		kfi.run();	
	}
}

void ListPanelFunc::dirUp() {
	openUrl( files() ->vfs_getOrigin().upURL(), files() ->vfs_getOrigin().fileName() );
}

void ListPanelFunc::pack() {
	QStringList fileNames;
	panel->getSelectedNames( &fileNames );
	if ( fileNames.isEmpty() )
		return ;  // safety

	if ( fileNames.count() == 0 )
		return ; // nothing to pack

	// choose the default name
	QString defaultName = panel->virtualPath().fileName();
	if ( defaultName == "" )
		defaultName = "pack";
	if ( fileNames.count() == 1 )
		defaultName = fileNames.first();
	// ask the user for archive name and packer
	new PackGUI( defaultName, panel->otherPanel->virtualPath().prettyURL(-1, KURL::StripFileProtocol), fileNames.count(), fileNames.first() );
	if ( PackGUI::type == QString::null )
		return ; // the user canceled

	// check for partial URLs	
	if( !PackGUI::destination.contains(":/") && !PackGUI::destination.startsWith("/") ){
		PackGUI::destination = panel->virtualPath().prettyURL()+"/"+PackGUI::destination;
	}
	
	QString destDir = PackGUI::destination;
	if( !destDir.endsWith( "/" ) )
		destDir += "/";
	
	bool packToOtherPanel = ( destDir == panel->otherPanel->virtualPath().prettyURL(1) );

	// on remote URL-s first pack into a temp file then copy to its right place
	KURL destURL = vfs::fromPathOrURL( destDir + PackGUI::filename + "." + PackGUI::type );
	KTempFile *tempDestFile = 0;
	QString arcFile;
	if ( destURL.isLocalFile() )
		arcFile = destURL.path();
	else if( destURL.protocol() == "virt" ) {
		KMessageBox::error( krApp, i18n( "Cannot pack files onto a virtual destination!" ) );
		return;                
	}        
	else {
		tempDestFile = new KTempFile( QString::null, "." + PackGUI::type );
		tempDestFile->setAutoDelete( true );
		arcFile = tempDestFile->name();
		QFile::remove
			( arcFile );
	}

	if (  QFileInfo( arcFile ).exists() ) {
		QString msg = i18n( "The Archive " ) + PackGUI::filename + "." + PackGUI::type+
		              i18n(" already exists. Do you want to overwrite the archive ?\n");
		if( PackGUI::type == "zip" ){
			msg = msg + i18n("(ZIP will replace identically named entries in the zip archive or add entries for new names)"); 
		} else {
			msg = msg+ ("(all data in previous archive will be lost)");
		}
		if ( KMessageBox::warningContinueCancel( krApp,msg,QString::null,i18n( "&Overwrite" ))
		        == KMessageBox::Cancel )
			return ; // stop operation
	}
	// tell the user to wait
	krApp->startWaiting( i18n( "Counting files to pack" ), 0 );

	// get the files to be packed:
	files() ->vfs_getFiles( &fileNames );

	KIO::filesize_t totalSize = 0;
	unsigned long totalDirs = 0, totalFiles = 0;
	if( !calcSpace( fileNames, totalSize, totalFiles, totalDirs ) )
		return;

	// download remote URL-s if necessary
	QString arcDir;
	KTempDir *tempDir = 0;

	if ( files() ->vfs_getOrigin().isLocalFile() )
		arcDir = files() ->vfs_workingDir();
	else {
		tempDir = new KTempDir();
		tempDir->setAutoDelete( true );
		arcDir = tempDir->name();
		KURL::List *urlList = files() ->vfs_getFiles( &fileNames );
		KIO::NetAccess::dircopy( *urlList, vfs::fromPathOrURL( arcDir ), 0 );
		delete urlList;
	}

	// pack the files
	// we must chdir() first because we supply *names* not URL's
	QString save = getcwd( 0, 0 );
	chdir( arcDir.local8Bit() );
	KRarcHandler::pack( fileNames, PackGUI::type, arcFile, totalFiles + totalDirs, PackGUI::extraProps );
	chdir( save.local8Bit() );

	// delete the temporary directory if created
	if ( tempDir )
		delete tempDir;

	// copy from the temp file to it's right place
	if ( tempDestFile ) {
		KIO::NetAccess::file_move( vfs::fromPathOrURL( arcFile ), destURL );
		delete tempDestFile;
	}

	if ( packToOtherPanel )
		panel->otherPanel->func->refresh();
}

void ListPanelFunc::testArchive() {
	QString arcName = panel->getCurrentName();
	if ( arcName.isNull() )
		return ;
	if ( arcName == ".." )
		return ; // safety

	KURL arcURL = files() ->vfs_getFile( arcName );
	QString url = QString::null;

	// download the file if it's on a remote filesystem
	if ( !arcURL.isLocalFile() ) {
		url = locateLocal( "tmp", QString( arcName ) );
		if ( !KIO::NetAccess::download( arcURL, url, 0 ) ) {
			KMessageBox::sorry( krApp, i18n( "Krusader is unable to download: " ) + arcURL.fileName() );
			return ;
		}
	} else
		url = arcURL.path( -1 );

	QString mime = files() ->vfs_search( arcName ) ->vfile_getMime();
	bool encrypted = false;
	QString type = KRarcHandler::getType( encrypted, url, mime );
	
	// check we that archive is supported
	if ( !KRarcHandler::arcSupported( type ) ) {
		KMessageBox::sorry( krApp, i18n( "%1, unknown archive type." ).arg( arcName ) );
		return ;
	}
	
	QString password = encrypted ? KRarcHandler::getPassword( url ) : QString::null;
	
	// test the archive
	if ( KRarcHandler::test( url, type, password ) )
		KMessageBox::information( krApp, i18n( "%1, test passed." ).arg( arcName ) );
	else
		KMessageBox::error( krApp, i18n( "%1, test failed!" ).arg( arcName ) );

	// remove the downloaded file if necessary
	if ( url != arcURL.path( -1 ) )
		QFile( url ).remove();
}

void ListPanelFunc::unpack() {

	QStringList fileNames;
	panel->getSelectedNames( &fileNames );
	if ( fileNames.isEmpty() )
		return ;  // safety

	QString s;
  if(fileNames.count() == 1)
    s = i18n("Unpack %1 to:").arg(fileNames[0]);
  else
    s = i18n("Unpack %n file to:", "Unpack %n files to:", fileNames.count());

	// ask the user for the copy dest
	KURL dest = KChooseDir::getDir(s, panel->otherPanel->virtualPath(), panel->virtualPath());
	if ( dest.isEmpty() ) return ; // the user canceled

	bool packToOtherPanel = ( dest.equals( panel->otherPanel->virtualPath(), true ) );

	for ( unsigned int i = 0; i < fileNames.count(); ++i ) {
		QString arcName = fileNames[ i ];
		if ( arcName.isNull() )
			return ;
		if ( arcName == ".." )
			return ; // safety

		// download the file if it's on a remote filesystem
		KURL arcURL = files() ->vfs_getFile( arcName );
		QString url = QString::null;
		if ( !arcURL.isLocalFile() ) {
			url = locateLocal( "tmp", QString( arcName ) );
			if ( !KIO::NetAccess::download( arcURL, url, 0 ) ) {
				KMessageBox::sorry( krApp, i18n( "Krusader is unable to download: " ) + arcURL.fileName() );
				continue;
			}
		} else
			url = arcURL.path( -1 );

		// if the destination is in remote directory use temporary one instead
		dest.adjustPath(1);
		KURL originalDestURL;
		KTempDir *tempDir = 0;

		if ( !dest.isLocalFile() ) {
			originalDestURL = dest;
			tempDir = new KTempDir();
			tempDir->setAutoDelete( true );
			dest = tempDir->name();
		}

		// determining the type
		QString mime = files() ->vfs_search( arcName ) ->vfile_getMime();
		bool encrypted = false;
		QString type = KRarcHandler::getType( encrypted, url, mime );

		// check we that archive is supported
		if ( !KRarcHandler::arcSupported( type ) ) {
			KMessageBox::sorry( krApp, i18n( "%1, unknown archive type" ).arg( arcName ) );
			continue;
		}
		
		QString password = encrypted ? KRarcHandler::getPassword( url ) : QString::null;
		
		// unpack the files
		KRarcHandler::unpack( url, type, password, dest.path( -1 ) );

		// remove the downloaded file if necessary
		if ( url != arcURL.path( -1 ) )
			QFile( url ).remove();

		// copy files to the destination directory at remote files
		if ( tempDir ) {
			QStringList nameList = QDir( dest.path( -1 ) ).entryList();
			KURL::List urlList;
			for ( unsigned int i = 0; i != nameList.count(); i++ )
				if ( nameList[ i ] != "." && nameList[ i ] != ".." )
					urlList.append( vfs::fromPathOrURL( dest.path( 1 ) + nameList[ i ] ) );
			if ( urlList.count() > 0 )
				KIO::NetAccess::dircopy( urlList, originalDestURL, 0 );
			delete tempDir;
		}
	}
	if ( packToOtherPanel )
		panel->otherPanel->func->refresh();
}

// a small ugly function, used to prevent duplication of EVERY line of
// code (maybe except 3) from createChecksum and matchChecksum
static void checksum_wrapper(ListPanel *panel, QStringList& args, bool &folders) {
	KrViewItemList items;
	panel->view->getSelectedKrViewItems( &items );
	if ( items.isEmpty() ) return ; // nothing to do
	// determine if we need recursive mode (md5deep)
	folders=false;
	for ( KrViewItemList::Iterator it = items.begin(); it != items.end(); ++it ) {
		if (panel->func->getVFile(*it)->vfile_isDir()) {
			folders = true;
			args << (*it)->name();
		} else args << (*it)->name();
	}
}

void ListPanelFunc::createChecksum() {
	QStringList args;
	bool folders;
	checksum_wrapper(panel, args, folders);
	CreateChecksumDlg dlg(args, folders, panel->realPath());
}

void ListPanelFunc::matchChecksum() {
	QStringList args;
	bool folders;
	checksum_wrapper(panel, args, folders);
	QValueList<vfile*> checksumFiles = files()->vfs_search(
		KRQuery(MatchChecksumDlg::checksumTypesFilter)
	);
	MatchChecksumDlg dlg(args, folders, panel->realPath(), 
		(checksumFiles.size()==1 ? checksumFiles[0]->vfile_getUrl().prettyURL() : QString::null));
}

void ListPanelFunc::calcSpace() {
	QStringList items;
	panel->view->getSelectedItems( &items );
	if ( items.isEmpty() ) {
		panel->view->selectAllIncludingDirs();
		panel->view->getSelectedItems( &items );
		if ( items.isEmpty() )
			return ; // nothing to do
	}

	KrCalcSpaceDialog calc( krApp, panel, items, false );
	calc.exec();
	for ( QStringList::ConstIterator it = items.begin(); it != items.end(); ++it ) {
		KrViewItem *viewItem = panel->view->findItemByName( *it );
		if ( viewItem )
			panel->view->updateItem(viewItem);
	}
	panel->slotUpdateTotals();
}

bool ListPanelFunc::calcSpace( const QStringList & items, KIO::filesize_t & totalSize, unsigned long & totalFiles, unsigned long & totalDirs ) {
	KrCalcSpaceDialog calc( krApp, panel, items, true );
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
		openUrl( panel->realPath() ); // open the last local URL
	}
}

void ListPanelFunc::newFTPconnection() {
	KURL url = KRSpWidgets::newFTP();
	// if the user canceled - quit
	if ( url.isEmpty() )
		return ;

	krFTPDiss->setEnabled( true );
	openUrl( url );
}

void ListPanelFunc::properties() {
	QStringList names;
	panel->getSelectedNames( &names );
	if ( names.isEmpty() )
		return ;  // no names...
	KFileItemList fi;
	fi.setAutoDelete( true );

	for ( unsigned int i = 0 ; i < names.count() ; ++i ) {
		vfile* vf = files() ->vfs_search( names[ i ] );
		if ( !vf )
			continue;
		KURL url = files()->vfs_getFile( names[i] );
		fi.append( new KFileItem( vf->vfile_getEntry(), url ) );
	}

	if ( fi.isEmpty() )
		return ;

	// Show the properties dialog
	KPropertiesDialog *dlg = new KPropertiesDialog( fi );
	connect( dlg, SIGNAL( applied() ), SLOTS, SLOT( refresh() ) );
}

void ListPanelFunc::refreshActions() {
	vfs::VFS_TYPE vfsType = files() ->vfs_getType();

	//  set up actions
	krMultiRename->setEnabled( vfsType == vfs::NORMAL );  // batch rename
	//krProperties ->setEnabled( vfsType == vfs::NORMAL || vfsType == vfs::FTP ); // file properties
	krFTPDiss ->setEnabled( vfsType == vfs::FTP );     // disconnect an FTP session
	krCreateCS->setEnabled( vfsType == vfs::NORMAL );
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
	if( !vfsP ) {
		if( vfsP->vfs_canDelete() )
			delete vfsP;
		else {
			connect( vfsP, SIGNAL( deleteAllowed() ), vfsP, SLOT( deleteLater() ) );
			vfsP->vfs_requestDelete();
		}
	}
	vfsP = 0;
}

vfs* ListPanelFunc::files() {
	if ( !vfsP )
		vfsP = KrVfsHandler::getVfs( "/", panel, 0 );
	return vfsP;
}


void ListPanelFunc::copyToClipboard( bool move ) {
	QStringList fileNames;

	panel->getSelectedNames( &fileNames );
	if ( fileNames.isEmpty() )
		return ;  // safety

	KURL::List* fileUrls = files() ->vfs_getFiles( &fileNames );
	if ( fileUrls ) {
		KRDrag * urlData = KRDrag::newDrag( *fileUrls, move, krApp->mainView, "krusader" );
		QApplication::clipboard() ->setData( urlData );
		delete fileUrls;
	}
}

void ListPanelFunc::pasteFromClipboard() {
	QClipboard * cb = QApplication::clipboard();
	QMimeSource * data = cb->data();
	KURL::List urls;
	if ( KURLDrag::canDecode( data ) ) {
		KURLDrag::decode( data, urls );
		bool cutSelection = KRDrag::decodeIsCutSelection( data );

		KURL destUrl = panel->virtualPath();

		KIO::Job* job = PreservingCopyJob::createCopyJob( PM_DEFAULT, urls,
			 destUrl, cutSelection ? KIO::CopyJob::Move : KIO::CopyJob::Copy, false, true );
		job->setAutoErrorHandlingEnabled( true );
		connect( job, SIGNAL( result( KIO::Job* ) ), SLOTS, SLOT( refresh() ) );
	}
}

#include "panelfunc.moc"
