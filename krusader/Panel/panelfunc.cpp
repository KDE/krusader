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
#include <qtimer.h>
#include <qtextstream.h>
// KDE Includes
#include <klocale.h>
#include <kio/jobclasses.h>
#include <kprocess.h>
#include <kpropertiesdialog.h>
#include <kopenwith.h>
#include <kmessagebox.h>
#include <kcursor.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kurl.h>
#include <krun.h>
#include <klineeditdlg.h>
#include <kdebug.h>
// Krusader Includes
#include "panelfunc.h"
#include "../krusader.h"
#include "../krslots.h"
#include "../defaults.h"
#include "../VFS/normal_vfs.h"
#include "../VFS/ftp_vfs.h"
#include "../VFS/temp_vfs.h"
#include "../VFS/vfile.h"
#include "../VFS/krarchandler.h"
#include "../VFS/krpermhandler.h"
#include "../Dialogs/packgui.h"
#include "../Dialogs/krdialogs.h"
#include "../Dialogs/krpleasewait.h"
#include "../Dialogs/krspwidgets.h"
#include "../KViewer/krviewer.h"
#include "../MountMan/kmountman.h"
#include "../resources.h"

/* --=={ Patch by Heiner <h.eichmann@gmx.de> }==-- */
KrCalcSpaceDialog::CalcThread::CalcThread(KrCalcSpaceDialog * parent, vfs * files, const QStringList & names)
	: m_totalSize(0), m_totalFiles(0), m_totalDirs(0), m_names(names), m_files(files), m_parent(parent)
        , m_threadInUse(true), m_stop(false) {}

void KrCalcSpaceDialog::CalcThread::cleanUp(){
	if (m_threadInUse || !finished())
		m_synchronizeUsageAccess.unlock();
	else{
		m_synchronizeUsageAccess.unlock(); // prevents a resource leak
		// otherwise: no one needs this instance any more: delete it
		delete this;
	}
}

void KrCalcSpaceDialog::CalcThread::deleteInstance(){
	// synchronize to avoid race condition.
	m_synchronizeUsageAccess.lock();
	m_threadInUse = false;
	cleanUp();
}

void KrCalcSpaceDialog::CalcThread::run(){
	if ( !m_names.isEmpty() ) // if something to do: do the calculation
	for ( QStringList::ConstIterator name = m_names.begin(); name != m_names.end(); ++name )
		m_files->vfs_calcSpace( *name, &m_totalSize, &m_totalFiles, &m_totalDirs , & m_stop);
	// synchronize to avoid race condition.
	m_synchronizeUsageAccess.lock();
	cleanUp(); // this does not need the instance any more
}

void KrCalcSpaceDialog::CalcThread::stop(){
	// cancel was pressed
	m_stop = true;
}

KrCalcSpaceDialog::KrCalcSpaceDialog(QWidget *parent, vfs * files, const QStringList & names, bool autoclose) :
	KDialogBase(parent, "KrCalcSpaceDialog", true, "Calculate Occupied Space", Ok|Cancel),
	m_autoClose(autoclose), m_canceled(false), m_timerCounter(0){
	// the dialog: The Ok button is hidden until it is needed
	showButtonOK(false);
	m_thread = new CalcThread(this, files, names);
	m_pollTimer = new QTimer(this);
	QWidget * mainWidget = new QWidget( this );
	setMainWidget(mainWidget);
	QVBoxLayout *topLayout = new QVBoxLayout( mainWidget, 0, spacingHint() );

	m_label = new QLabel( "", mainWidget, "caption" );
	showResult(); // fill m_label with something usefull
	topLayout->addWidget( m_label );
	topLayout->addStretch(10);
}

void KrCalcSpaceDialog::calculationFinished(){
	// close dialog if auto close is true
	if (m_autoClose){
		done(0);
		return;
	}
	// otherwise hide cancel and show ok button
	showButtonCancel(false);
	showButtonOK(true);
	showResult(); // and show final result
}

/* This timer has two jobs: it polls the thread if it is finished. Polling is
  better here as it might finish while the dialog builds up. Secondly it refreshes
  the displayed result.
 */
void KrCalcSpaceDialog::timer(){
	// thread finished?
	if (m_thread->finished()){
		// close dialog or switch buttons
		calculationFinished();
		m_pollTimer->stop(); // stop the polling. No longer needed
		return;
	}

	// Every 10 pollings (1 second) refresh the displayed result
	if (++m_timerCounter > 10){
		m_timerCounter = 0;
		showResult();
	}
}

void KrCalcSpaceDialog::showResult(){
	if (!m_thread) return;
	QString msg;
	QString fileName = ( ( m_thread->getNames().count() == 1 ) ? ( i18n( "Name: " ) + m_thread->getNames().first() + "\n" ) : QString( "" ) );
	msg = fileName + i18n( "Total occupied space: %1\nin %2 directories and %3 files" ).
		arg( KIO::convertSize( m_thread->getTotalSize() ) ).arg( m_thread->getTotalDirs() ).arg( m_thread->getTotalFiles() );
	m_label->setText(msg);
}

void KrCalcSpaceDialog::slotCancel(){
	m_thread->stop(); // notify teh thread to stop
	m_canceled = true; // set the cancel flag
	KDialogBase::slotCancel(); // close the dialog
}

KrCalcSpaceDialog::~KrCalcSpaceDialog(){
	CalcThread * tmp = m_thread;
	m_thread = 0; // do not access the thread anymore or core dump if smoe piece of code wrongly does
	tmp->deleteInstance(); // Notify the thread, that the dialog does not need anymore.
}

void KrCalcSpaceDialog::exec(){
	m_thread->start(); // start the thread
	if (m_autoClose){ // autoclose
		// set the cursor to busy mode and wait 3 seconds or until the thread finishes
		krApp->setCursor( KCursor::waitCursor() );
		bool result = m_thread->wait(3000);
		krApp->setCursor( KCursor::arrowCursor() );  // return the cursor to normal mode
		if (result) return;// thread finished: do not show the dialog
		showResult(); // fill the invisible dialog with usefull data
	}
	// prepare and start the poll timer
	connect(m_pollTimer, SIGNAL(timeout()), this, SLOT(timer()));
	m_pollTimer->start(100);
	KDialogBase::exec(); // show the dialog
}
/* --=={ End of patch by Heiner <h.eichmann@gmx.de> }==-- */

//////////////////////////////////////////////////////////
//////		----------	List Panel -------------		////////
//////////////////////////////////////////////////////////

ListPanelFunc::ListPanelFunc( ListPanel *parent ) :
panel( parent ), inRefresh( false ) {
  vfsStack.setAutoDelete( true );
  vfsStack.push( new normal_vfs( "/", panel ) );
  files() ->vfs_refresh();
}

void ListPanelFunc::openUrl( const QString& path,const QString& nameToMakeCurrent) {
  panel->slotFocusOnMe();

  QString mypath = path;

  // clear the view - to avoid a repaint crash
  panel->view->clear();
  if( !nameToMakeCurrent.isEmpty() ){
		panel->view->setNameToMakeCurrent( nameToMakeCurrent );
  }
  // make sure local urls are handles ok
  if ( mypath.lower().startsWith( "file:" ) )
    mypath = mypath.mid( 5 );

  // remote file systems
  if ( mypath.contains( ":/" ) ) {
    // first close all open archives / remote connections
    while ( files() ->vfs_getType() != "normal" ) vfsStack.remove();
    vfs* v = new ftp_vfs(mypath,panel);
    if ( v->vfs_error() ) {
      kdWarning() << "Failed to create vfs: " << mypath.local8Bit() << endl;
      delete v;
      refresh();
      return ;
    }
    // save the current vfs
    files() ->blockSignals( true );
    vfsStack.push( v );
  } else { // local directories
    // first close all open archives / remote connections
    while ( files() ->vfs_getType() != "normal" )
      vfsStack.remove();
    // now we have a normal vfs- refresh it.
    files() ->blockSignals( false );

    if( mypath == "~" ) mypath = QDir::homeDirPath();
    if( !mypath.startsWith("/") ) mypath = files()->vfs_getOrigin()+"/"+mypath;

    mypath = QDir::cleanDirPath(mypath);

    while ( !KRpermHandler::dirExist(mypath) ) {
      panel->view->setNameToMakeCurrent( mypath.mid(mypath.findRev('/')+1 ) );
      mypath = mypath.left( mypath.findRev( '/' ) );
      if ( mypath.isEmpty() )
        mypath = "/";
    }

    chdir( mypath.latin1() );
    refresh( mypath );
  }
}

void ListPanelFunc::refresh( const QString path ) {
  // change the cursor to busy
  krApp->setCursor( KCursor::waitCursor() );

  // if we could not refresh try to dir up
  QString origin = path;
  if ( !files() ->vfs_refresh( origin ) ) {
    panel->virtualPath = origin;
    dirUp();
    return ; // dirUp() calls refresh again...
  }
  // update the backStack
  if ( backStack.last() != panel->realPath ) {
    krBack->setEnabled( true );
    backStack.append( panel->realPath );
    //the size of the backStack is hard coded - 30
    if ( backStack.count() > 30 )
      backStack.remove( backStack.begin() );
  }
}

void ListPanelFunc::goBack() {
  if ( backStack.isEmpty() )
    return ;

  if ( backStack.last() == "//WARNING//" ) {
    KMessageBox::information( 0, i18n( "Can't re-enter archive. Going to the nearest path" ), QString::null, "BackArchiveWarning" );
    backStack.remove( backStack.fromLast() ); //remove the //WARNING// entry
  }
  // avoid going back to the same place
  while ( !backStack.isEmpty() && backStack.last() == panel->realPath )
    backStack.remove( backStack.fromLast() );

  if ( backStack.isEmpty() )
    return ;
  QString path = backStack.last();

  refresh( path );
  backStack.remove( backStack.fromLast() );

  if ( backStack.isEmpty() )
    krBack->setEnabled( false );
}

void ListPanelFunc::redirectLink() {
  if ( files() ->vfs_getType() == "ftp" ) {
    KMessageBox::sorry( krApp, i18n( "You can edit links only on local file systems" ) );
    return ;
  }

  vfile *vf = files() ->vfs_search( panel->getCurrentName() );
  if ( !vf )
    return ;

  QString file = files() ->vfs_getFile( vf->vfile_getName() );
  QString currentLink = vf->vfile_getSymDest();
  if ( currentLink.isEmpty() ) {
    KMessageBox::sorry( krApp, i18n( "The current file is not a link, so i can't redirect it." ) );
    return ;
  }

  // ask the user for a new destination
  bool ok = false;
  QString newLink =
    KLineEditDlg::getText( i18n( "Please enter the new link destination:" ), currentLink, &ok, krApp );

  // if the user canceled - quit
  if ( !ok || newLink == currentLink )
    return ;
  // delete the current link
  if ( unlink( file.local8Bit() ) == -1 ) {
    KMessageBox::sorry( krApp, i18n( "Can't remove old link: " ) + file.latin1() );
    return ;
  }
  // try to create a new symlink
  if ( symlink( newLink.local8Bit(), file.local8Bit() ) == -1 ) {
    KMessageBox::/* --=={ Patch by Heiner <h.eichmann@gmx.de> }==-- */sorry( krApp, i18n( "Failed to create a new link: " ) + file.latin1() );
    return ;
  }
}

void ListPanelFunc::krlink( bool sym ) {
  if ( files() ->vfs_getType() == "ftp" ) {
    KMessageBox::sorry( krApp, i18n( "You can create links only on local file systems" ) );
    return ;
  }

  QString name = panel->getCurrentName();

  // ask the new link name..
  bool ok = false;
  QString linkName =
    KLineEditDlg::getText( i18n( "Create a new link to: " ) + name.latin1(), name.latin1(), &ok, krApp );

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
    name = files()->vfs_getFile( name );

  if ( sym ) {
    if ( symlink( name.latin1(), linkName.latin1() ) == -1 )
      KMessageBox::sorry( krApp, i18n( "Failed to create a new symlink: " ) + linkName +
                          i18n( " To: " ) + name );
  } else {
    if ( link( name.latin1(), linkName.latin1() ) == -1 )
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

  // at this point, let's take the full path
  fileName = files()->vfs_getFile( fileName );
  // and call KViewer.
  KrViewer::view( KURL::fromPathOrURL(fileName) );
  // nothing more to it!
}

void ListPanelFunc::terminal() {
  QString save = getcwd( 0, 0 );
  chdir( panel->realPath.local8Bit() );

  KProcess proc;
  krConfig->setGroup( "General" );
  QString term = krConfig->readEntry( "Terminal", _Terminal );
  proc << term;
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

  KProcess proc;
  krConfig->setGroup( "General" );
  QString edit = krConfig->readEntry( "Editor", _Editor );
  if ( edit == "internal editor" )
    KrViewer::edit( KURL::fromPathOrURL(files() ->vfs_getFile( name )) );
  else {
    proc << edit << files() ->vfs_getFile( name );
    if ( !proc.start( KProcess::DontCare ) )
      KMessageBox::sorry( krApp, i18n( "Can't open " ) + "\"" + edit + "\"" );
  }
}

void ListPanelFunc::moveFiles() {
  QStringList fileNames;
  panel->getSelectedNames( &fileNames );
  if ( fileNames.isEmpty() )
    return ;  // safety

  QString dest = panel->otherPanel->getPath();
                 /* --=={ Patch by Heiner <h.eichmann@gmx.de> }==-- */
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
    KLineEditDlg::getText( i18n( "Directory's name:" ), "", &ok, krApp );

  // if the user canceled - quit
  if ( !ok || dirName.isEmpty() )
    return ;

  // if the name is already taken - quit
  if ( QDir( files() ->vfs_getOrigin() + "/" + dirName ).exists() ) {
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
         files() ->vfs_getType() == "normal" ) {
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
  emptyDirVerify = ( ( emptyDirVerify ) && ( files() ->vfs_getType() == "normal" ) );

  QDir dir;
  for ( QStringList::Iterator name = fileNames.begin(); name != fileNames.end(); ) {
    vfile * vf = files() ->vfs_search( *name );

    // verify non-empty dirs delete... (only for norml vfs)
    if ( emptyDirVerify && vf->vfile_isDir() && !vf->vfile_isSymLink() ) {
      dir.setPath( panel->getPath() + "/" + ( *name ) );
      if ( dir.count() > 2 ) {
        switch ( KMessageBox::warningYesNoCancel( krApp,
                                                  i18n( "Directory " ) + ( *name ).latin1() + i18n( " is not empty !\nSkip this one or Delete All ?" ),
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
  QString origin = files() ->vfs_getOrigin();

  QString type = vf->vfile_getMime().right( 4 );
  if ( vf->vfile_getMime().contains( "-rar" ) )
    type = "-rar";

  if ( vf->vfile_isDir() ) {
    origin.right(1) == "/" ? origin += name : origin += "/" + name;
    panel->view->setNameToMakeCurrent( QString::null );
    refresh( origin );
  } else if ( KRarcHandler::arcHandled( type ) && !origin.contains(":/")) {
    QString path = files()->vfs_getFile(vf->vfile_getName());
    if ( type == "-tbz" || type == "-tgz" || type == "tarz" || type == "-tar" ){
			path = "tar:"+path;
		} else {
			path = "krarc:"+path;
    }
    openUrl( path );
	} else {
    KURL url;
    url.setPath( files() ->vfs_getFile( name ) );
    KRun::runURL( url, vf->vfile_getMime() );
  }
}

void ListPanelFunc::dirUp() {
  QString origin = panel->virtualPath;
	QString newOrigin = origin;
  // remove one dir from the new path
  newOrigin.truncate( newOrigin.findRev('/') );

  // Do we need to change vfs ?
  bool changeVFS = files()->vfs_error();

	if( origin.contains(":/") ){ // ftp_vfs
    if( QFileInfo(origin.mid(origin.find(":/")+1)).exists() ){
			changeVFS = true;
			// make the current archive the current item on the new list
			panel->view->setNameToMakeCurrent(origin.mid(origin.findRev('/')+1) );
	  }
	}

  // clear the view - to avoid a repaint crash
  panel->view->clear();
  
	if( changeVFS ){
    vfsStack.remove();
    files()->blockSignals( false );
    files()->vfs_refresh();
    return ;
	}

  // make the current dir the current item on the new list
  panel->view->setNameToMakeCurrent(origin.mid(origin.findRev("/")+1));


  // check the '/' case
  if ( newOrigin == "" ) newOrigin = "/";
  // and the '/' case for urls
  if ( newOrigin.contains( ":/" ) && newOrigin.find( "/", newOrigin.find( ":/" ) + 3 ) == -1 )
    newOrigin = newOrigin + "/";

  // change dir..
  refresh( newOrigin );
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
  if ( KRarcHandler::test( files() ->vfs_getFile( arcName ), type ) )
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
    KRarcHandler::unpack( files() ->vfs_getFile( arcName ), type, dest );
  }
  if ( packToOtherPanel )
    panel->otherPanel->func->refresh();
}

void ListPanelFunc::calcSpace() {
	QStringList names;
	panel->getSelectedNames( &names );
	if ( names.isEmpty() ) return ; // nothing to do

	KrCalcSpaceDialog calc(krApp, files(), names, false);
	calc.exec();
}

bool ListPanelFunc::calcSpace(QStringList & names,KIO::filesize_t & totalSize,unsigned long & totalFiles,unsigned long & totalDirs) {
	KrCalcSpaceDialog calc(krApp, files(), names, true);
	calc.exec();
	totalSize = calc.getTotalSize();
	totalFiles = calc.getTotalFiles();
	totalDirs = calc.getTotalDirs();
	return !calc.wasCanceled();
}

void ListPanelFunc::FTPDisconnect() {
  // clear the view - to avoid a repaint crash
  panel->view->clear();

  // you can disconnect only if connected !
  if ( files() ->vfs_getType() == "ftp" ) {
    vfsStack.remove();
    files() ->blockSignals( false );
    if ( files() ->vfs_getType() != "ftp" )
      krFTPDiss->setEnabled( false );
    krFTPNew->setEnabled( true );
    panel->view->setNameToMakeCurrent( QString::null );
    files() ->vfs_refresh();
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

	KURL::List* urls = files() ->vfs_getFiles( &names );
  for ( unsigned int i = 0 ; i < urls->count() ; ++i ) {
    KURL url = (*urls->at(i));
    vfile* vf = files()->vfs_search( url.fileName() );
    if( !vf ) continue;
    fi.append( new KFileItem(vf->vfile_getEntry(),files()->vfs_getOrigin(),false,true) );
  }

  // Show the properties dialog
  KPropertiesDialog *dlg = new KPropertiesDialog( fi );
  connect( dlg, SIGNAL( applied() ), SLOTS, SLOT( refresh() ) );
}

void ListPanelFunc::refreshActions() {
  QString vfsType = files() ->vfs_getType();
  //  set up actions
  krMultiRename->setEnabled( vfsType == "normal" );  // batch rename
  krProperties ->setEnabled( vfsType == "normal" || vfsType == "ftp" ); // file properties
  krFTPDiss ->setEnabled( vfsType == "ftp" );     // disconnect an FTP session
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
  while ( vfsStack.remove() ); // delete all vfs objects
}

vfs* ListPanelFunc::files() {
  // make sure we return a valid VFS*
  if ( vfsStack.isEmpty() ) {
    vfsStack.push( new normal_vfs( "/", panel ) );
  }
  return vfsStack.top();
}

#include "panelfunc.moc"
