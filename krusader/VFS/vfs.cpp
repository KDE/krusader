/***************************************************************************
	  	                        vfs.cpp
  	                    -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
  ------------------------------------------------------------------------
   the vfs class is an extendable class which by itself does (almost)
   nothing. other VFSs like the normal_vfs inherits from this class and
   make it possible to use a consistent API for all types of VFSs.

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

#include "vfs.h"
#include <kapplication.h>
#include <kdebug.h>
#include <time.h>
#include "../krusader.h"
#include "../defaults.h"

vfs::vfs(QObject* panel, bool quiet): quietMode(quiet),disableRefresh(false),vfileIterator(0), 
                                      mimeTypeMagicDisabled( false ) {
		if ( panel ){
	 		connect(this,SIGNAL(startUpdate()),panel,SLOT(slotStartUpdate()));
	 		connect(this,SIGNAL(incrementalRefreshFinished( QString )),panel,SLOT(slotGetStats( QString )));
		}
		else quietMode = true;
}

KIO::filesize_t vfs::vfs_totalSize(){
	KIO::filesize_t temp=0;
	class vfile* vf=vfs_getFirstFile();
		
	while (vf!=0){
		if ( (vf->vfile_getName() != ".") && ( vf->vfile_getName() != "..")  
		     && !(vf->vfile_isDir()) )
				temp+=vf->vfile_getSize();
		vf=vfs_getNextFile();
	}
	return temp;                                                     
}

bool vfs::vfs_refresh(KIO::Job* job){
  if(job && job->error()){
		job->showErrorDialog(krApp);
	}
	return vfs_refresh(vfs_origin);
}

KURL vfs::fromPathOrURL( const QString &originIn )
{
  QString password, loginName, origin = originIn;
  bool bugfix = false;
  
  if ( originIn.contains( ":/" ) && !originIn.startsWith( "/" ) )
  {
    // breakdown the url;
    /* FIXME: untill KDE fixes the bug we have to check for
       passwords and users with @ in them... */
    bugfix = origin.find("@") != origin.findRev("@");
    if(bugfix){
      if(origin.find(":") != origin.findRev(":", origin.findRev("@") )){
        int passStart = origin.find( ":",origin.find(":")+1 )+1;
        int passLen = origin.findRev("@")-passStart;
        password = origin.mid(passStart,passLen);
        origin = origin.remove(passStart-1,passLen+1);
      }
      if(origin.find("@") != origin.findRev("@")){
        int usrStart = origin.find( "/" )+1;
        if(origin.at(usrStart) == '/') ++usrStart;
        int usrLen = origin.findRev("@")-usrStart;
        loginName = origin.mid(usrStart,usrLen);
        origin = origin.remove(usrStart,usrLen+1);
      }
    }
  }
  KURL url = KURL::fromPathOrURL( origin );
  if(loginName.isEmpty()) loginName = url.user();
  if(password.isEmpty())  password  = url.pass();
  if(bugfix){
    url.setPass(password);
    url.setUser(loginName);
  }

  return url;
}

void vfs::setVfsFilesP(vfileDict* dict){
	vfs_filesP=dict;
	vfs_searchP = dict;
	if( vfileIterator ) delete vfileIterator;
	vfileIterator = new QDictIterator<vfile>(*dict);
}

bool vfs::vfs_refresh(){ 
	dirty = false;
	// point the vfs_filesP to a NEW (empty) dictionary
	vfs_filesP = new vfileDict();
	//vfs_filesP->setAutoDelete(true);
	
	// and populate it
	krConfig->setGroup("Look&Feel");
	bool showHidden = krConfig->readBoolEntry("Show Hidden",_ShowHidden);
	bool res = populateVfsList(vfs_getOrigin(),showHidden);

	QString name;
	if( res ){
		// compare the two list emiting signals when needed;;
		for( vfile* vf = vfs_getFirstFile(); vf ;  ){
			name = vf->vfile_getName();
			vfile* newVf = (*vfs_filesP)[name];
			if( !newVf ){
				// the file was deleted..
				emit deletedVfile(name);
				vfs_searchP->remove(name);
				// the remove() advance our iterator ! 
				vf = vfileIterator->current();
			} else {
				if( *vf != *newVf ){
					// the file was changed..
					*vf = *newVf;
					emit updatedVfile(vf);
				}
				vf=vfs_getNextFile();
			}
			removeFromList(name);
		} 
		// everything thats left is a new file
		QDictIterator<vfile> it(*vfs_filesP);
		for(vfile* vf=it.toFirst(); vf; vf=(++it)){
			// sanity checking
			if( !vf || (*vfs_searchP)[vf->vfile_getName()] ) continue;
			
			vfile* newVf = new vfile();
			*newVf = *vf;
			vfs_searchP->insert(newVf->vfile_getName(),newVf);
			emit addedVfile(newVf);
		}
	}
	
	// delete the needed temporary vfs_filesP
	// and make the vfs_searchP the primary list again 
	vfileDict *temp = vfs_filesP;
	vfs_filesP = vfs_searchP;
	delete temp;
	emit incrementalRefreshFinished( vfs_origin.isLocalFile() ? vfs_origin.path() : vfs_origin.prettyURL() );        
	
	return res; 
}

bool vfs::vfs_refresh(const KURL& origin){
	if( origin.equals(vfs_getOrigin(),true) ) return vfs_refresh();
	
	dirty = false;        
	krConfig->setGroup("Look&Feel");
	bool showHidden = krConfig->readBoolEntry("Show Hidden",_ShowHidden);

	// clear the the list
	clear();
	// and re-populate it
	if (!populateVfsList(origin,showHidden) ) return false;
	if (!disableRefresh) emit startUpdate();
	else dirty = true;   
	     
	return true;
}

void vfs::vfs_enableRefresh(bool enable){
	if (vfs_type != NORMAL) return;
	if (disableRefresh == !enable) return; // if gets called twice by mistake
	disableRefresh = quietMode = !enable;
  if( enable && dirty ) vfs_refresh();
	dirty = false;
}

void vfs::slotKdsResult(KIO::Job *job){


}

/// to be implemented
void vfs::vfs_calcSpace( QString /*name*/ , KIO::filesize_t* /*totalSize*/, unsigned long* /*totalFiles*/, unsigned long* /*totalDirs*/, bool* /*stop*/ ) {
#if 0
	if ( stop && *stop ) return ;
	busy = true;
	KDirSize* kds = KDirSize::dirSizeJob( vfs_getFile( name ) );
	krOut << "vfs_calcSpace: " << vfs_getFile( name ) << endl;
	connect( kds, SIGNAL( result( KIO::Job* ) ), this, SLOT( slotListResult( KIO::Job* ) ) );

	//while (busy && (!stop || !(*stop))) qApp->processEvents();

	*totalSize += kds->totalSize();
	*totalFiles += kds->totalFiles();
	*totalDirs += kds->totalSubdirs();

	kds->kill( true );
#endif
}

#include "vfs.moc"
