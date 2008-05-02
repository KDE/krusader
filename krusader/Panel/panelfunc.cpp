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
#include <qeventloop.h>
#include <qclipboard.h> 
#include <QList>
// KDE Includes
#include <klocale.h>
#include <kprocess.h>
#include <kpropertiesdialog.h>
#include <kmessagebox.h>
#include <kcursor.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kurl.h>
#include <krun.h>
#include <kinputdialog.h>
#include <kdebug.h>
#include <kio/netaccess.h>
#include <kio/jobuidelegate.h>
#include <kstandarddirs.h>
#include <ktempdir.h> 
#include <kurlrequester.h>
#include <kdesktopfile.h>
// Krusader Includes
#include "panelfunc.h"
#include "krcalcspacedialog.h"
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
#include <QDrag>
#include <QMimeData>

//////////////////////////////////////////////////////////
//////          ----------      List Panel -------------                ////////
//////////////////////////////////////////////////////////

ListPanelFunc::ListPanelFunc( ListPanel *parent ) :
panel( parent ), inRefresh( false ), vfsP( 0 ) {
	urlStack.push_back( KUrl( "file:/" ) );
	connect( &delayTimer, SIGNAL( timeout() ), this, SLOT( doOpenUrl() ) );
}

void ListPanelFunc::openUrl( const QString& url, const QString& nameToMakeCurrent ) {
	openUrl( KUrl( 
		// KUrlRequester is buggy: it should return a string containing "/home/shie/downloads"
		// but it returns "~/downloads" which is parsed incorrectly by KUrl.
		// replacedPath should replace ONLY $HOME and environment variables
		panel->origin->completionObject()->replacedPath(url) )
		, nameToMakeCurrent );
}

void ListPanelFunc::immediateOpenUrl( const KUrl& urlIn ) {
	KUrl url = urlIn;
	url.cleanPath();

	// check for special cases first - don't refresh here !
	// you may call openUrl or vfs_refresh()
	if ( !url.isValid() || url.isRelative() ) {
		if ( url.url() == "~" ) {
			return openUrl( QDir::homePath() );
		} else if ( !url.url().startsWith( "/" ) ) {
			// possible relative URL - translate to full URL
			url = files() ->vfs_getOrigin();
			url.addPath( urlIn.url() );
			//kDebug()<< urlIn.url() << "," << url.url() <<endl;
		} else {
			panel->slotStartUpdate();  // refresh the panel
			return ;
		}
	}

	// if we are not refreshing to current URL
	bool is_equal_url = files() ->vfs_getOrigin().equals( url, KUrl::CompareWithoutTrailingSlash );
	
	if ( !is_equal_url ) {
		// change the cursor to busy
		panel->setCursor( Qt::WaitCursor );
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
	if ( !urlStack.last().equals( url ) )
		urlStack.push_back( url );
	// count home many urls is in the stack, so later on, we'll know if the refresh was a success
	int stackSize = urlStack.count();
	bool refreshFailed = true; // assume the worst
	while ( true ) {
		KUrl u = urlStack.takeLast();
		//u.adjustPath(KUrl::RemoveTrailingSlash); // remove trailing "/"
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
		} else {
			if( vfsP->vfs_isBusy() )
			{
				delayURL = url;               /* this function is useful for FTP url-s and bookmarks */
				delayTimer.setSingleShot( true );
				delayTimer.start( 100 );  /* if vfs is busy try refreshing later */
				return;
			}
		}
		connect( files(), SIGNAL(startJob(KIO::Job* )),
				panel, SLOT(slotJobStarted(KIO::Job* )));
		if ( vfsP->vfs_refresh( u ) ) {
			break; // we have a valid refreshed URL now
		}
		if ( vfsP == 0 )  // the object was deleted during vfs_refresh? Hoping the best...
			return;                
		// prevent repeated error messages
		if ( vfsP->vfs_isDeleting() )
			break;                
		vfsP->vfs_setQuiet( true );
	}
	vfsP->vfs_setQuiet( false );

	// if we popped exactly 1 url from the stack, it means the url we were
	// given was refreshed successfully.
	if (stackSize == urlStack.count() + 1)
		refreshFailed = false;

	// update the urls stack
	if ( urlStack.isEmpty() || !files() ->vfs_getOrigin().equals( urlStack.last() ) ) {
		urlStack.push_back( files() ->vfs_getOrigin() );
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
		chdir( files() ->vfs_getOrigin().path().toLocal8Bit() );
	
	// see if the open url operation failed, and if so, 
	// put the attempted url in the origin bar and let the user change it
	if (refreshFailed) {
		panel->origin->setUrl(urlIn.prettyUrl());
		panel->origin->setFocus();
	}

	refreshActions();
}

void ListPanelFunc::openUrl( const KUrl& url, const QString& nameToMakeCurrent ) {
	panel->inlineRefreshCancel();
	// first the other dir, then the active! Else the focus changes and the other becomes active
	if ( panel->syncBrowseButton->state() == SYNCBROWSE_CD ) {
		// prevents that the sync-browsing circles itself to death
		static bool inSync = false;
		if( ! inSync ){
			inSync = true;
			//do sync-browse stuff....
			ListPanel *other_panel = OTHER_PANEL;
			KUrl otherDir = other_panel->virtualPath();
			QString otherText = other_panel->origin->lineEdit()->text();
			
			OTHER_FUNC->files() ->vfs_setQuiet( true );
			// the trailing slash is nessesary because krusader provides Dir's without it
			// we can't use openUrl because the delay don't allow a check if the panel has realy changed!
			OTHER_FUNC->immediateOpenUrl( KUrl::relativeUrl( panel->virtualPath().url() + "/", url.url() ) );
			OTHER_FUNC->files() ->vfs_setQuiet( false );
			// now we need to test ACTIVE_PANEL because the openURL has changed the active panel!!
			if ( other_panel->virtualPath().equals( otherDir ) ) {
				// deactivating the sync-browse if syncbrowse not possible
				panel->syncBrowseButton->setChecked( false );
				other_panel->origin->lineEdit()->setText( otherText );
			}
			inSync = false;
		}
	}
	this->nameToMakeCurrent = nameToMakeCurrent;
	delayURL = url;               /* this function is useful for FTP url-s and bookmarks */
	delayTimer.setSingleShot( true );
	delayTimer.start( 0 );  /* to avoid qApp->processEvents() deadlock situaltion */
}

void ListPanelFunc::refresh() {
	openUrl(panel->virtualPath()); // re-read the files
}

void ListPanelFunc::doOpenUrl() {
	immediateOpenUrl( delayURL );
}

void ListPanelFunc::goBack() {
	if ( !canGoBack())
		return ;

	if ( urlStack.last().equals( files() ->vfs_getOrigin() ) )
		urlStack.pop_back();
	openUrl( urlStack.last(), files() ->vfs_getOrigin().fileName() );

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

	QString file = files() ->vfs_getFile( vf->vfile_getName() ).path( KUrl::RemoveTrailingSlash );
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
	if ( unlink( file.toLocal8Bit() ) == -1 ) {
		KMessageBox::sorry( krApp, i18n( "Can't remove old link: " ) + file );
		return ;
	}
	// try to create a new symlink
	if ( symlink( newLink.toLocal8Bit(), file.toLocal8Bit() ) == -1 ) {
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
		name = files() ->vfs_getFile( name ).path( KUrl::RemoveTrailingSlash );

	if ( sym ) {
		if ( symlink( name.toLocal8Bit(), linkName.toLocal8Bit() ) == -1 )
			KMessageBox::sorry( krApp, i18n( "Failed to create a new symlink: " ) + linkName +
			                    i18n( " To: " ) + name );
	} else {
		if ( link( name.toLocal8Bit(), linkName.toLocal8Bit() ) == -1 )
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
	//FIXME nearly identical code is in krpopupmenu.cpp
	KProcess proc;
	KConfigGroup group( krConfig, "General" );
	QString term = group.readEntry( "Terminal", _Terminal );
	proc << KrServices::separateArgs( term );
	proc.setWorkingDirectory(panel->realPath());
#if 0 // I _think_ that this is no more nessesary since the process is now allways detached
	if ( term.contains( "konsole" ) )    /* KDE 3.2 bug (konsole is killed by pressing Ctrl+C) */
	{                                  /* Please remove the patch if the bug is corrected */
		proc << "&";
		proc.setUseShell( true );
	}
#endif
	if ( !proc.startDetached() )
		KMessageBox::sorry( krApp, i18n( "<qt>Can't open <b>%1</b></qt>", term) );
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

	KUrl dest = panel->otherPanel->virtualPath();
	KUrl virtualBaseURL;

	QString destProtocol = dest.protocol();
	if ( destProtocol == "krarc" || destProtocol == "tar" || destProtocol == "zip" ) {
		KMessageBox::sorry( krApp, i18n( "Moving into archive is disabled" ) );
		return ;
	}

	KConfigGroup group( krConfig, "Advanced" );
	if ( group.readEntry( "Confirm Move", _ConfirmMove ) ) {
		bool preserveAttrs = group.readEntry( "PreserveAttributes", _PreserveAttributes );
		QString s;

  if( fileNames.count() == 1 )
    s = i18n("Move %1 to:", fileNames.first());
  else
    s = i18np("Move %1 file to:", "Move %1 files to:", fileNames.count());

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

	KUrl::List* fileUrls = files() ->vfs_getFiles( &fileNames );

	// after the delete return the cursor to the first unmarked
	// file above the current item;
	panel->prepareToDelete();

	if( !virtualBaseURL.isEmpty() ) {
		// keep the directory structure for virtual paths
		VirtualCopyJob *vjob = new VirtualCopyJob( &fileNames, files(), dest, virtualBaseURL, pmode, KIO::CopyJob::Move, true );
		connect( vjob, SIGNAL( result( KJob* ) ), this, SLOT( refresh() ) );
		if ( dest.equals( panel->otherPanel->virtualPath(), KUrl::CompareWithoutTrailingSlash ) )
			connect( vjob, SIGNAL( result( KJob* ) ), panel->otherPanel->func, SLOT( refresh() ) );
	}
	// if we are not moving to the other panel :
	else if ( !dest.equals( panel->otherPanel->virtualPath(), KUrl::CompareWithoutTrailingSlash ) ) {
		// you can rename only *one* file not a batch,
		// so a batch dest must alwayes be a directory
		if ( fileNames.count() > 1 ) dest.adjustPath(KUrl::AddTrailingSlash);
		KIO::Job* job = PreservingCopyJob::createCopyJob( pmode, *fileUrls, dest, KIO::CopyJob::Move, false, true );
		job->ui()->setAutoErrorHandlingEnabled( true );
		// refresh our panel when done
		connect( job, SIGNAL( result( KJob* ) ), this, SLOT( refresh() ) );
		// and if needed the other panel as well
		if ( dest.equals( panel->otherPanel->virtualPath(), KUrl::CompareWithoutTrailingSlash ) )
			connect( job, SIGNAL( result( KJob* ) ), panel->otherPanel->func, SLOT( refresh() ) );

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

	QStringList dirTree = dirName.split( "/" );

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

KUrl ListPanelFunc::getVirtualBaseURL() {
	if( files()->vfs_getType() != vfs::VIRT || otherFunc()->files()->vfs_getType() == vfs::VIRT )
		return KUrl();
	
	QStringList fileNames;
	panel->getSelectedNames( &fileNames );
	
	KUrl::List* fileUrls = files() ->vfs_getFiles( &fileNames );
	if( fileUrls->count() == 0 )
		return KUrl();
	
	KUrl base = (*fileUrls)[ 0 ].upUrl();
	
	if( base.protocol() == "virt" ) // is it a virtual subfolder?
		return KUrl();          // --> cannot keep the directory structure
	
	for( int i=1; i < fileUrls->count(); i++ ) {
		if( base.isParentOf( (*fileUrls)[ i ] ) )
			continue;
		if( base.protocol() != (*fileUrls)[ i ].protocol() )
			return KUrl();
		
		do {
			KUrl oldBase = base;
			base = base.upUrl();
			if( oldBase.equals( base, KUrl::CompareWithoutTrailingSlash ) )
				return KUrl();
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

	KUrl dest = panel->otherPanel->virtualPath();
	KUrl virtualBaseURL;

	// confirm copy
	KConfigGroup group( krConfig, "Advanced" );
	if ( group.readEntry( "Confirm Copy", _ConfirmCopy ) ) {
		bool preserveAttrs = group.readEntry( "PreserveAttributes", _PreserveAttributes );
		QString s;

  if( fileNames.count() == 1 )
    s = i18n("Copy %1 to:", fileNames.first());
  else
    s = i18np("Copy %1 file to:", "Copy %1 files to:", fileNames.count());

		// ask the user for the copy dest
		virtualBaseURL = getVirtualBaseURL();
		dest = KChooseDir::getDir(s, dest, panel->virtualPath(), preserveAttrs, virtualBaseURL );
		if ( dest.isEmpty() ) return ; // the user canceled
		if( preserveAttrs )
			pmode = PM_PRESERVE_ATTR;
		else
			pmode = PM_NONE;
	}

	KUrl::List* fileUrls = files() ->vfs_getFiles( &fileNames );

	if( !virtualBaseURL.isEmpty() ) {
		// keep the directory structure for virtual paths
		VirtualCopyJob *vjob = new VirtualCopyJob( &fileNames, files(), dest, virtualBaseURL, pmode, KIO::CopyJob::Copy, true );
		connect( vjob, SIGNAL( result( KJob* ) ), this, SLOT( refresh() ) );
		if ( dest.equals( panel->otherPanel->virtualPath(), KUrl::CompareWithoutTrailingSlash ) )
			connect( vjob, SIGNAL( result( KJob* ) ), panel->otherPanel->func, SLOT( refresh() ) );
	}
	// if we are  not copying to the other panel :
	else if ( !dest.equals( panel->otherPanel->virtualPath(), KUrl::CompareWithoutTrailingSlash ) ) {
		// you can rename only *one* file not a batch,
		// so a batch dest must alwayes be a directory
		if ( fileNames.count() > 1 ) dest.adjustPath(KUrl::AddTrailingSlash);
		KIO::Job* job = PreservingCopyJob::createCopyJob( pmode, *fileUrls, dest, KIO::CopyJob::Copy, false, true );
		job->ui()->setAutoErrorHandlingEnabled( true );
		if ( dest.equals( panel->virtualPath(), KUrl::CompareWithoutTrailingSlash ) ||
			dest.upUrl().equals( panel->virtualPath(), KUrl::CompareWithoutTrailingSlash ) )
			// refresh our panel when done
			connect( job, SIGNAL( result( KJob* ) ), this, SLOT( refresh() ) );
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

	KConfigGroup gg( krConfig, "General" );
	bool trash = gg.readEntry( "Move To Trash", _MoveToTrash );
	// now ask the user if he want to delete:
	KConfigGroup group( krConfig, "Advanced" );
	if ( group.readEntry( "Confirm Delete", _ConfirmDelete ) ) {
		QString s, b;

		if ( !reallyDelete && trash && files() ->vfs_getType() == vfs::NORMAL ) {
			s = i18np( "Do you really want to move this item to the trash?", "Do you really want to move these %1 items to the trash?", fileNames.count() );
			b = i18n( "&Trash" );
		} else if( files() ->vfs_getType() == vfs::VIRT && files()->vfs_getOrigin().equals( KUrl("virt:/"), KUrl::CompareWithoutTrailingSlash ) ) {
			s = i18np( "Do you really want to delete this virtual item (physical files stay untouched)?", "Do you really want to delete these virtual items (physical files stay untouched)?", fileNames.count() );
			b = i18n( "&Delete" );
		} else if( files() ->vfs_getType() == vfs::VIRT ) {
			s = i18np( "<qt>Do you really want to delete this item <b>physically</b> (not just removing it from the virtual items)?</qt>", "<qt>Do you really want to delete these %1 items <b>physically</b> (not just removing them from the virtual items)?</qt>", fileNames.count() );
			b = i18n( "&Delete" );
		} else {
			s = i18np( "Do you really want to delete this item?", "Do you really want to delete these %1 items?", fileNames.count() );
			b = i18n( "&Delete" );
		}

		// show message
		// note: i'm using continue and not yes/no because the yes/no has cancel as default button
		if ( KMessageBox::warningContinueCancelList( krApp, s, fileNames,
                                                     i18n( "Warning" ), KGuiItem( b ) ) != KMessageBox::Continue )
			return ;
	}
	//we want to warn the user about non empty dir
	// and files he don't have permission to delete
	bool emptyDirVerify = group.readEntry( "Confirm Unempty Dir", _ConfirmUnemptyDir );
	emptyDirVerify = ( ( emptyDirVerify ) && ( files() ->vfs_getType() == vfs::NORMAL ) );

	QDir dir;
	for ( QStringList::Iterator name = fileNames.begin(); name != fileNames.end(); ) {
		vfile * vf = files() ->vfs_search( *name );

		// verify non-empty dirs delete... (only for normal vfs)
		if ( emptyDirVerify && vf->vfile_isDir() && !vf->vfile_isSymLink() ) {
			dir.setPath( panel->virtualPath().path() + "/" + ( *name ) );
			if ( dir.entryList(QDir::TypeMask | QDir::System | QDir::Hidden ).count() > 2 ) {
				switch ( KMessageBox::warningYesNoCancel( krApp,
																		i18n( "<qt><p>Directory <b>%1</b> is not empty!</p><p>Skip this one or Delete All?</p></qt>", *name),
																		QString(), KGuiItem( i18n( "&Skip" )), KGuiItem( i18n( "&Delete All" ) ) ) ) {
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
		group.writeEntry( "Move To Trash", false );
	}
	files() ->vfs_delFiles( &fileNames );
	if (reallyDelete) {
		group.writeEntry( "Move To Trash", trash);
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
	
	KUrl origin = files() ->vfs_getOrigin();

	QString protocol = origin.isLocalFile() ? KrServices::registerdProtocol( vf->vfile_getMime() ) : "";

	if ( protocol == "tar" || protocol == "krarc" ) {
		bool encrypted;
		QString type = KRarcHandler::getType( encrypted, vf->vfile_getUrl().path(), vf->vfile_getMime(), false );
		if ( !KRarcHandler::arcHandled( type ) )   // if the specified archive is disabled delete the protocol
			protocol = "";
	}

	if ( vf->vfile_isDir() ) {
		origin = files() ->vfs_getFile( name );
		panel->view->setNameToMakeCurrent( QString() );
		openUrl( origin );
	} else if ( !protocol.isEmpty() ) {
		KUrl path = files() ->vfs_getFile( vf->vfile_getName() );
		path.setProtocol( protocol );
		openUrl( path );
	} else {
		KUrl url = files() ->vfs_getFile( name );
		KFileItem kfi( vf->vfile_getEntry(), url,true );
		kfi.run();	
	}
}

void ListPanelFunc::dirUp() {
	openUrl( files() ->vfs_getOrigin().upUrl(), files() ->vfs_getOrigin().fileName() );
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
	new PackGUI( defaultName, vfs::pathOrUrl( panel->otherPanel->virtualPath(), KUrl::RemoveTrailingSlash ), fileNames.count(), fileNames.first() );
	if ( PackGUI::type == QString() )
		return ; // the user canceled

	// check for partial URLs	
	if( !PackGUI::destination.contains(":/") && !PackGUI::destination.startsWith("/") ){
		PackGUI::destination = panel->virtualPath().prettyUrl()+"/"+PackGUI::destination;
	}
	
	QString destDir = PackGUI::destination;
	if( !destDir.endsWith( "/" ) )
		destDir += "/";
	
	bool packToOtherPanel = ( destDir == panel->otherPanel->virtualPath().prettyUrl(KUrl::AddTrailingSlash) );

	// on remote URL-s first pack into a temp file then copy to its right place
	KUrl destURL = KUrl( destDir + PackGUI::filename + "." + PackGUI::type );
	KTemporaryFile *tempDestFile = 0;
	QString arcFile;
	if ( destURL.isLocalFile() )
		arcFile = destURL.path();
	else if( destURL.protocol() == "virt" ) {
		KMessageBox::error( krApp, i18n( "Cannot pack files onto a virtual destination!" ) );
		return;                
	}        
	else {
		tempDestFile = new KTemporaryFile();
		tempDestFile->setSuffix( QString(".") + PackGUI::type );
		tempDestFile->open(); tempDestFile->close(); // nessesary to create the filename
		arcFile = tempDestFile->fileName();
		QFile::remove
			( arcFile );
	}

	if (  QFileInfo( arcFile ).exists() ) {
		QString msg = i18n( "<qt><p>The archive <b>%1.%2</b> already exists. Do you want to overwrite it?</p><p>All data in the previous archive will be lost!</p></qt>", PackGUI::filename, PackGUI::type);
		if( PackGUI::type == "zip" ) {
			msg = i18n( "<qt><p>The archive <b>%1.%2</b> already exists. Do you want to overwrite it?</p><p>Zip will replace identically named entries in the zip archive or add entries for new names.</p></qt>", PackGUI::filename, PackGUI::type);
		}
		if ( KMessageBox::warningContinueCancel( krApp,msg,QString(),KGuiItem( i18n( "&Overwrite" )))
		        == KMessageBox::Cancel )
			return ; // stop operation
	}
	// tell the user to wait
	krApp->startWaiting( i18n( "Counting files to pack" ), 0, true );

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
		arcDir = tempDir->name();
		KUrl::List *urlList = files() ->vfs_getFiles( &fileNames );
		KIO::NetAccess::dircopy( *urlList, KUrl( arcDir ), 0 );
		delete urlList;
	}

	// pack the files
	// we must chdir() first because we supply *names* not URL's
	QString save = getcwd( 0, 0 );
	chdir( arcDir.toLocal8Bit() );
	KRarcHandler::pack( fileNames, PackGUI::type, arcFile, totalFiles + totalDirs, PackGUI::extraProps );
	chdir( save.toLocal8Bit() );

	// delete the temporary directory if created
	if ( tempDir )
		delete tempDir;

	// copy from the temp file to it's right place
	if ( tempDestFile ) {
		KIO::NetAccess::move( KUrl( arcFile ), destURL );
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

	KUrl arcURL = files() ->vfs_getFile( arcName );
	QString url = QString();

	// download the file if it's on a remote filesystem
	if ( !arcURL.isLocalFile() ) {
		url = KStandardDirs::locateLocal( "tmp", QString( arcName ) );
		if ( !KIO::NetAccess::download( arcURL, url, 0 ) ) {
			KMessageBox::sorry( krApp, i18n( "Krusader is unable to download: " ) + arcURL.fileName() );
			return ;
		}
	} else
		url = arcURL.path( KUrl::RemoveTrailingSlash );

	QString mime = files() ->vfs_search( arcName ) ->vfile_getMime();
	bool encrypted = false;
	QString type = KRarcHandler::getType( encrypted, url, mime );
	
	// check we that archive is supported
	if ( !KRarcHandler::arcSupported( type ) ) {
		KMessageBox::sorry( krApp, i18n( "%1, unknown archive type.", arcName ) );
		return ;
	}
	
	QString password = encrypted ? KRarcHandler::getPassword( url ) : QString();
	
	// test the archive
	if ( KRarcHandler::test( url, type, password ) )
		KMessageBox::information( krApp, i18n( "%1, test passed.", arcName ) );
	else
		KMessageBox::error( krApp, i18n( "%1, test failed!", arcName ) );

	// remove the downloaded file if necessary
	if ( url != arcURL.path( KUrl::RemoveTrailingSlash ) )
		QFile( url ).remove();
}

void ListPanelFunc::unpack() {

	QStringList fileNames;
	panel->getSelectedNames( &fileNames );
	if ( fileNames.isEmpty() )
		return ;  // safety

	QString s;
  if(fileNames.count() == 1)
    s = i18n("Unpack %1 to:", fileNames[0]);
  else
    s = i18np("Unpack %1 file to:", "Unpack %1 files to:", fileNames.count());

	// ask the user for the copy dest
	KUrl dest = KChooseDir::getDir(s, panel->otherPanel->virtualPath(), panel->virtualPath());
	if ( dest.isEmpty() ) return ; // the user canceled

	bool packToOtherPanel = ( dest.equals( panel->otherPanel->virtualPath(), KUrl::CompareWithoutTrailingSlash ) );

	for ( int i = 0; i < fileNames.count(); ++i ) {
		QString arcName = fileNames[ i ];
		if ( arcName.isNull() )
			return ;
		if ( arcName == ".." )
			return ; // safety

		// download the file if it's on a remote filesystem
		KUrl arcURL = files() ->vfs_getFile( arcName );
		QString url = QString();
		if ( !arcURL.isLocalFile() ) {
			url = KStandardDirs::locateLocal( "tmp", QString( arcName ) );
			if ( !KIO::NetAccess::download( arcURL, url, 0 ) ) {
				KMessageBox::sorry( krApp, i18n( "Krusader is unable to download: " ) + arcURL.fileName() );
				continue;
			}
		} else
			url = arcURL.path( KUrl::RemoveTrailingSlash );

		// if the destination is in remote directory use temporary one instead
		dest.adjustPath(KUrl::AddTrailingSlash);
		KUrl originalDestURL;
		KTempDir *tempDir = 0;

		if ( !dest.isLocalFile() ) {
			originalDestURL = dest;
			tempDir = new KTempDir();
			dest = tempDir->name();
		}

		// determining the type
		QString mime = files() ->vfs_search( arcName ) ->vfile_getMime();
		bool encrypted = false;
		QString type = KRarcHandler::getType( encrypted, url, mime );

		// check we that archive is supported
		if ( !KRarcHandler::arcSupported( type ) ) {
			KMessageBox::sorry( krApp, i18n( "%1, unknown archive type", arcName ) );
			continue;
		}
		
		QString password = encrypted ? KRarcHandler::getPassword( url ) : QString();
		
		// unpack the files
		KRarcHandler::unpack( url, type, password, dest.path( KUrl::RemoveTrailingSlash ) );

		// remove the downloaded file if necessary
		if ( url != arcURL.path( KUrl::RemoveTrailingSlash ) )
			QFile( url ).remove();

		// copy files to the destination directory at remote files
		if ( tempDir ) {
			QStringList nameList = QDir( dest.path( KUrl::RemoveTrailingSlash ) ).entryList();
			KUrl::List urlList;
			for ( int i = 0; i != nameList.count(); i++ )
				if ( nameList[ i ] != "." && nameList[ i ] != ".." )
					urlList.append( KUrl( dest.path( KUrl::AddTrailingSlash ) + nameList[ i ] ) );
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
	QList<vfile*> checksumFiles = files()->vfs_search(
		KRQuery(MatchChecksumDlg::checksumTypesFilter)
	);
	MatchChecksumDlg dlg(args, folders, panel->realPath(), 
		(checksumFiles.size()==1 ? checksumFiles[0]->vfile_getUrl().prettyUrl() : QString()));
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
		panel->view->setNameToMakeCurrent( QString() );
		openUrl( panel->realPath() ); // open the last local URL
	}
}

void ListPanelFunc::newFTPconnection() {
	KUrl url = KRSpWidgets::newFTP();
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

	for ( int i = 0 ; i < names.count() ; ++i ) {
		vfile* vf = files() ->vfs_search( names[ i ] );
		if ( !vf )
			continue;
		KUrl url = files()->vfs_getFile( names[i] );
		fi.push_back( KFileItem( vf->vfile_getEntry(), url ) );
	}

	if ( fi.isEmpty() )
		return ;

	// Show the properties dialog
	KPropertiesDialog *dlg = new KPropertiesDialog( fi, krApp );
	connect( dlg, SIGNAL( applied() ), SLOTS, SLOT( refresh() ) );
	dlg->show();
}

bool ListPanelFunc::canGoBack() {
	return (urlStack.count() > 1);
}

void ListPanelFunc::refreshActions() {
	vfs::VFS_TYPE vfsType = files() ->vfs_getType();

	//  set up actions
	krMultiRename->setEnabled( vfsType == vfs::NORMAL );  // batch rename
	//krProperties ->setEnabled( vfsType == vfs::NORMAL || vfsType == vfs::FTP ); // file properties
	krFTPDiss ->setEnabled( vfsType == vfs::FTP );     // disconnect an FTP session
	krCreateCS->setEnabled( vfsType == vfs::NORMAL );
	
	QString protocol = files()->vfs_getOrigin().protocol();
	krRemoteEncoding->setEnabled( protocol == "ftp" || protocol == "sftp" || protocol == "fish" );
	
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
	  krRoot->setEnabled(true);                              // go all the way up
	      krExecFiles->setEnabled(true);                         // show only executables
	*/
	krBack->setEnabled(canGoBack());	                 // go back
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
		vfsP = KrVfsHandler::getVfs( KUrl( "/" ), panel, 0 );
	return vfsP;
}


void ListPanelFunc::copyToClipboard( bool move ) {
	if( files()->vfs_getOrigin().equals( KUrl("virt:/"), KUrl::CompareWithoutTrailingSlash ) ) {
		if( move )
			KMessageBox::error( krApp, i18n( "Cannot cut a virtual URL collection to the clipboard!" ) );
		else
			KMessageBox::error( krApp, i18n( "Cannot copy a virtual URL collection onto the clipboard!" ) );
		return;
	}
	
	QStringList fileNames;

	panel->getSelectedNames( &fileNames );
	if ( fileNames.isEmpty() )
		return ;  // safety

	KUrl::List* fileUrls = files() ->vfs_getFiles( &fileNames );
	if ( fileUrls ) {
		QMimeData *mimeData = new QMimeData;
		mimeData->setData( "application/x-kde-cutselection", move ? "1" : "0");
		fileUrls->populateMimeData(mimeData);
		
		QApplication::clipboard()->setMimeData( mimeData, QClipboard::Clipboard );
		
		if( move && files()->vfs_getType() == vfs::VIRT )
			( static_cast<virt_vfs*>( files() ) )->vfs_removeFiles( &fileNames );
		
		delete fileUrls;
	}
}

void ListPanelFunc::pasteFromClipboard() {
	QClipboard * cb = QApplication::clipboard();
	
	bool move = false;
	const QMimeData *data = cb->mimeData();
	if ( data->hasFormat( "application/x-kde-cutselection" ) ) {
		QByteArray a = data->data( "application/x-kde-cutselection" );
		if ( !a.isEmpty() )
			move = (a.at(0) == '1'); // true if 1
	}
	
	KUrl::List urls = KUrl::List::fromMimeData( data );
	if ( urls.isEmpty() )
		return ;
	
	KUrl destUrl = panel->virtualPath();
	
	files()->vfs_addFiles( &urls, move ? KIO::CopyJob::Move : KIO::CopyJob::Copy, otherFunc()->files(),
		"", PM_DEFAULT );
}

#include "panelfunc.moc"
