/***************************************************************************
                                 krslots.cpp
                             -------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
    web site		 : http://krusader.sourceforge.net
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

#include <qdir.h>
#include <qpoint.h>
#include <qstringlist.h>
#include <qprogressdialog.h>
#include <qlistview.h>
// KDE includes
#include <klocale.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <klineeditdlg.h>
#include <kkeydialog.h>
#include <kdirnotify_stub.h>
#include <kio/netaccess.h>
#include <kedittoolbar.h>
#include <kdeversion.h>
#include <KViewer/krviewer.h>
// Krusader includes
#include "krslots.h"
#include "krusader.h"
#include "krusaderview.h"
#include "Panel/listpanel.h"
#include "Dialogs/krdialogs.h"
#include "Dialogs/krspwidgets.h"
#include "GUI/krusaderstatus.h"
#include "RemoteMan/remoteman.h"
#include "Panel/panelfunc.h"
#include "Konfigurator/konfigurator.h"
#include "MountMan/kmountman.h"
#include "defaults.h"
#include "resources.h"
#include "GUI/kfnkeys.h"
#include "GUI/kcmdline.h"
#include "Search/krquery.h"
#include "Search/krsearchmod.h"
#include "Search/krsearchdialog.h"
#include "VFS/vfs.h"
#include "panelmanager.h"

#define ACTIVE_PANEL        (krApp->mainView->activePanel)
#define ACTIVE_FUNC         (krApp->mainView->activePanel->func)
#define MAIN_VIEW           (krApp->mainView)
#define REFRESH_BOTH_PANELS { ListPanel *p=ACTIVE_PANEL;        \
                              MAIN_VIEW->left->func->refresh(); \
	                            MAIN_VIEW->right->func->refresh();\
                              p->slotFocusOnMe(); }


void KRslots::selectCompareMask() {
  int left=0,right=0;
  KRSpWidgets::selectCompareColorMask(&left, &right);
  if (left>0 && right>0) { // safeguard
    MAIN_VIEW->left->colorMask = left;
    MAIN_VIEW->right->colorMask = right;
    if (krCompareDirs->isChecked() ) {
      ((ListPanel*)MAIN_VIEW->left)->view->clear();
      ((ListPanel*)MAIN_VIEW->left)->slotUpdate();
      ((ListPanel*)MAIN_VIEW->right)->view->clear();
      ((ListPanel*)MAIN_VIEW->right)->slotUpdate();
    }
  }
}

void KRslots::compareDirectories() {
  KMessageBox::sorry(0, i18n("Compare mode is temporarily disabled. sorry"));

  if (!krCompareDirs->isChecked()) {
    ACTIVE_PANEL->compareMode = false;
    ACTIVE_PANEL->otherPanel->compareMode = false;
    REFRESH_BOTH_PANELS;
    return;
  } // else is implied

  ACTIVE_PANEL->compareMode = true;
  ACTIVE_PANEL->otherPanel->compareMode = true;
  REFRESH_BOTH_PANELS;
}

void KRslots::sendFileByEmail(QString filename) {
  QString mailProg;
  QStringList lst = Krusader::supportedTools();
  if (lst.contains("MAIL")) mailProg=lst[lst.findIndex("MAIL") + 1];
  else {
    KMessageBox::error(0,i18n("Krusader can't find any of the supported mail clients. Please install one to your path. Hint: Krusader supports kmail."));
    return;
  }

  KShellProcess proc;

  if (mailProg == "kmail") {
    proc << "kmail" << "--subject \""+i18n("Sending file: ")+
            filename.mid(filename.findRev('/')+1)+"\"" << QString::null +
            "--attach "+"\"" + filename + "\"";
  }

	if (!proc.start(KProcess::DontCare))
    KMessageBox::error(0,i18n("Error executing ")+mailProg+" !");
  else proc.detach();
}

void KRslots::compareContent() {
  QString diffProg;
  QStringList lst = Krusader::supportedTools();
  if (lst.contains("DIFF")) diffProg=lst[lst.findIndex("DIFF") + 1];
  else {
    KMessageBox::error(0,i18n("Krusader can't find any of the supported diff-frontends. Please install one to your path. Hint: Krusader supports kdiff and xxdiff."));
    return;
  }

  QStringList lst1, lst2;
  QString name1, name2;

  ((ListPanel*)ACTIVE_PANEL)->getSelectedNames(&lst1);
  ((ListPanel*)ACTIVE_PANEL->otherPanel)->getSelectedNames(&lst2);

  // first, see if we've got exactly 1 selected file, if not, try the current one
  if (lst1.count() == 1) name1 = lst1[0];
  if (lst2.count() == 1) name2 = lst2[0];

  if ( name1.isEmpty() || name2.isEmpty() ) {
    // if we got here, then one of the panel can't be sure what file to diff
		KMessageBox::detailedError(0,i18n("Don't know which files to compare."),
      i18n("To compare 2 files by content, select (mark) a file in the left panel, and select another one in the right panel."));
    return;
  }

  // else implied: all ok, let's call kdiff
	// but if one of the files isn't local, download them first
	KURL url1 = ACTIVE_FUNC->files()->vfs_getFile(name1);
	KURL url2 = ACTIVE_PANEL->otherPanel->func->files()->vfs_getFile(name2);
	
	QString tmp1 = QString::null, tmp2 = QString::null;
  if (!url1.isLocalFile()) {
 		if( !KIO::NetAccess::download( url1, tmp1 ) ){
      KMessageBox::sorry(krApp,i18n("Krusader is unable to download: ")+name1);
      return;
    }
	} else tmp1 = url1.url();
  if (!url2.isLocalFile()) {
 		if( !KIO::NetAccess::download( url2, tmp2 ) ){
      KMessageBox::sorry(krApp,i18n("Krusader is unable to download: ")+name1);
      return;
    }
	} else tmp2 = url2.url();

  KShellProcess p;
  p << diffProg << tmp1.mid(tmp1.find('/')) << tmp2.mid(tmp2.find('/'));
	if (!p.start(KProcess::DontCare))
    KMessageBox::error(0,i18n("Error executing ")+diffProg+" !");
  else
    p.detach();
  sleep(2);	

	if( tmp1 != url1.url() ) KIO::NetAccess::removeTempFile( tmp1 );
	if( tmp2 != url2.url() ) KIO::NetAccess::removeTempFile( tmp2 );
}

void KRslots::rightclickMenu() {
  ACTIVE_PANEL->popRightClickMenu(
   ACTIVE_PANEL->mapToGlobal(
   dynamic_cast<KListView*>(ACTIVE_PANEL->view)->itemRect(dynamic_cast<QListViewItem*>
   (ACTIVE_PANEL->view->getCurrentKrViewItem())).topLeft()
   )
  );
}

void KRslots::addBookmark(){
  QString path=ACTIVE_PANEL->virtualPath;
  if (path.contains('\\')>0)
    KMessageBox::information(0, i18n("In order to save the time needed to enter an archive, Krusader will not allow bookmarks which point into an archive, as it is usually a mistake to do so!"),
                             QString::null, "BookmarkArchives");
}

// GUI toggle slots
void KRslots::toggleFnkeys(){
  if( MAIN_VIEW->fnKeys->isVisible() ) MAIN_VIEW->fnKeys->hide();
  else MAIN_VIEW->fnKeys->show();
}

void KRslots::toggleCmdline(){
  if( MAIN_VIEW->cmdLine->isVisible() ) MAIN_VIEW->cmdLine->hide();
  else MAIN_VIEW->cmdLine->show();
}

void KRslots::toggleToolbar() {
  if (krApp->toolBar()->isVisible())
		krApp->toolBar()->hide();
	else krApp->toolBar()->show();
}

void KRslots::toggleStatusbar() {
  if (krApp->statusBar()->isVisible())
		krApp->statusBar()->hide();
	else krApp->statusBar()->show();
}

void KRslots::toggleTerminal() {
  if( MAIN_VIEW->terminal_dock->isVisible() ) MAIN_VIEW->slotTerminalEmulator(false);
  else MAIN_VIEW->slotTerminalEmulator(true);
}

void KRslots::showAboutApplication() {
  KRAbout *q=new KRAbout();
  q->exec();
}

void KRslots::insertFileName(bool full_path)
{
  QString filename = ACTIVE_PANEL->view->getCurrentItem();
  if( filename == QString::null )
    return;
  
  if( full_path )
  {
    QString path=ACTIVE_FUNC->files()->vfs_getOrigin();
    if( !path.endsWith( "/" ))
      path+="/";
    filename = path+filename;
  }

  QString current = MAIN_VIEW->cmdLine->text();
  if( current.length() != 0 && !current.endsWith( " " ) )
    current += " ";
  MAIN_VIEW->cmdLine->setText( current + filename );
}

// directory list functions
void KRslots::allFilter()			  {	ACTIVE_PANEL->setFilter(ListPanel::ALL);	 }
void KRslots::execFilter()			{	ACTIVE_PANEL->setFilter(ListPanel::EXEC);	 }
void KRslots::customFilter()		{	ACTIVE_PANEL->setFilter(ListPanel::CUSTOM);}
void KRslots::markAll()         { ACTIVE_PANEL->select(true,true);           }
void KRslots::unmarkAll()       { ACTIVE_PANEL->select(false,true);          }
void KRslots::markGroup()       { ACTIVE_PANEL->select(true,false);          }
void KRslots::unmarkGroup()     { ACTIVE_PANEL->select(false,false);         }
void KRslots::invert()          { ACTIVE_PANEL->invertSelection();           }

void KRslots::root()            { ACTIVE_FUNC->openUrl("/");                 }
void KRslots::refresh(QString p){ ACTIVE_FUNC->openUrl(p);                   }
void KRslots::home()            { ACTIVE_FUNC->openUrl(QDir::homeDirPath()); }
void KRslots::refresh()         { ACTIVE_FUNC->refresh();                    }
void KRslots::properties()      { ACTIVE_FUNC->properties();                 }
void KRslots::dirUp()           { ACTIVE_FUNC->dirUp();                      }
void KRslots::back()            { ACTIVE_FUNC->goBack();                     }
void KRslots::slotPack()        { ACTIVE_FUNC->pack();                       }
void KRslots::slotUnpack()      { ACTIVE_FUNC->unpack();                     }
void KRslots::testArchive()     { ACTIVE_FUNC->testArchive();                }
void KRslots::calcSpace()       { ACTIVE_FUNC->calcSpace();                  }
void KRslots::FTPDisconnect()   { ACTIVE_FUNC->FTPDisconnect();              }
void KRslots::newFTPconnection(){ newFTPconnection(QString::null); 					 }
void KRslots::newFTPconnection(QString host)
               					        { ACTIVE_FUNC->newFTPconnection(host);       }
// run external modules / programs
void KRslots::runKonfigurator(bool firstTime) { delete new Konfigurator(firstTime); }

void KRslots::toggleSortByExt() {
  krConfig->setGroup("Look&Feel");
	bool ext = !krConfig->readBoolEntry("Sort By Extention",_SortByExt);
	krApp->actToggleSortByExt->setChecked(ext);
  krConfig->writeEntry("Sort By Extention",ext);
  REFRESH_BOTH_PANELS;
}

void KRslots::toggleHidden(){
  krConfig->setGroup("Look&Feel");
	bool show = !krConfig->readBoolEntry("Show Hidden",_ShowHidden);
	krApp->actToggleHidden->setChecked(show);
  krConfig->writeEntry("Show Hidden",show);
	REFRESH_BOTH_PANELS;
}

void KRslots::toggleSwapPanels(){
  PanelManager *currentFirst = panel_swap ? MAIN_VIEW->rightMng : MAIN_VIEW->leftMng;
	krApp->actToggleSwapPanels->setChecked( panel_swap = !panel_swap );
  MAIN_VIEW->horiz_splitter->moveToLast( currentFirst );
  MAIN_VIEW->left->updateGeometry();
  MAIN_VIEW->right->updateGeometry();
}

void KRslots::search() {
  new KrSearchDialog();
}

void KRslots::runRemoteMan() {
	QString host=remoteMan::getHost();
	if (host==QString::null) return;
	// otherwise, attempt a connection
	newFTPconnection(host);
}

void KRslots::runMountMan() {
  if (krApp->mountMan->operational()) {
    while (!krApp->mountMan->ready());
    krApp->mountMan->mainWindow();
  }
}

void KRslots::homeTerminal(){
  QString save = getcwd(0,0);
	chdir (QDir::homeDirPath().local8Bit());

  KProcess proc;
	krConfig->setGroup("General");
	QString term = krConfig->readEntry("Terminal",_Terminal);
	proc <<  term;
	if(!proc.start(KProcess::DontCare))
	  KMessageBox::sorry(krApp,i18n("Can't open ")+"\""+term+"\"");

	chdir(save.local8Bit());
}

void KRslots::sysInfo(){
  KProcess proc;
  proc << "kcmshell" << "System/ksysctrl";
  if (!proc.start(KProcess::DontCare)){
    KMessageBox::sorry(krApp,i18n("Can't find \"KsysCtrl\". Please install KDE admin package"));
  }
}

void KRslots::multiRename(){
	QStringList lst = Krusader::supportedTools();
	int i = lst.findIndex("RENAME");
	if (i == -1){
  	KMessageBox::sorry(krApp,i18n("Can't find a batch renamer tool.\nYou can get Krename at http://krename.sf.net"));
  	return;
	}
	QString pathToRename = lst[i+1];
	
  QStringList names;
  ((ListPanel*)ACTIVE_PANEL)->getSelectedNames(&names);
	KURL::List* urls = ACTIVE_FUNC->files()->vfs_getFiles(&names);

	if( urls->isEmpty() ){
		delete urls;
		return;
	}

	KShellProcess proc;
	proc << pathToRename;

	for( KURL::List::iterator u=urls->begin(); u != urls->end(); ++u){
    if( QFileInfo((*u).path()).isDir() ) proc << "-r";
		proc << "\"" + (*u).path() + "\""; // patch thanks to Tobias Vogele
	}

	proc.start(KProcess::DontCare);
	delete urls;
}

// settings slots
void KRslots::configToolbar(){
  KEditToolbar dlg(krApp->factory());//actionCollection());
  if (dlg.exec())
    krApp->updateGUI();
}

void KRslots::configKeys(){
  KKeyDialog::configureKeys(krApp->actionCollection(),"krusaderui.rc");
}

// misc
void KRslots::changeTrashIcon(){
	// update trash bin icon - this is "stolen" konqi code
	// Copyright (C) 2000  David Faure <faure@kde.org>
	KURL trash;
	trash.setPath(KGlobalSettings::trashPath());
	KURL::List lst;
	lst.append(trash);
	KDirNotify_stub allDirNotify("*","KDirNotify*");
	allDirNotify.FilesChanged( lst );
	// end of konqi code
}

// F2
void KRslots::terminal()       { ACTIVE_FUNC->terminal();   }
// F3
void KRslots::view()	         { ACTIVE_FUNC->view();       }
// F4
void KRslots::edit()           { ACTIVE_FUNC->editFile();   }
// F5
void KRslots::copyFiles()      { ACTIVE_FUNC->copyFiles();  }
// F6
void KRslots::moveFiles()      { ACTIVE_FUNC->moveFiles();  }	
// F7
void KRslots::mkdir()          { ACTIVE_FUNC->mkdir();      }
// F8
void KRslots::deleteFiles()    { ACTIVE_FUNC->deleteFiles();}     	
// F9
void KRslots::rename()         { ACTIVE_FUNC->rename();     }

// Shift F3
void KRslots::viewDlg(){
  // ask the user for a url to view
  KChooseDir *chooser = new KChooseDir( 0,i18n("Enter a URL to view:"), QString::null);
  QString dest = chooser->dest;
  if ( dest == QString::null ) return ; // the usr canceled
  else
  {
    /* KURL::fromPathOrURL requires fullpath, so we check whether it is relative  */
    if ( !dest.contains( ":/" ) && !dest.startsWith( "/" ) )
      dest = ACTIVE_FUNC->files()->vfs_getOrigin() + "/" + dest; /* it's full path now */
    
    KrViewer::view( KURL::fromPathOrURL(dest) ); // view the file
  }
  // nothing more to it!
}

// Shift F4
void KRslots::editDlg(){
  // ask the user for the filename to edit
  KChooseDir *chooser = new KChooseDir( 0,i18n("Enter the filename to edit:"), QString::null);
  QString dest = chooser->dest;
  if ( dest == QString::null ) return ; // the usr canceled
  else
  {
    /* KURL::fromPathOrURL requires fullpath, so we check whether it is relative  */
    if ( !dest.contains( ":/" ) && !dest.startsWith( "/" ) )
      dest = ACTIVE_FUNC->files()->vfs_getOrigin() + "/" + dest; /* it's full path now */
    
    krConfig->setGroup( "General" );
    QString edit = krConfig->readEntry( "Editor", _Editor );

    if ( edit == "internal editor" )
      KrViewer::edit( KURL::fromPathOrURL( dest ), true );
    else {
      KProcess proc;
      proc << edit << dest;
      if ( !proc.start( KProcess::DontCare ) )
        KMessageBox::sorry( krApp, i18n( "Can't open " ) + "\"" + edit + "\"" );
    }
  }
  // nothing more to it!
}

#include "krslots.moc"
