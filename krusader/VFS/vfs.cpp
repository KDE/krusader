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

#include <unistd.h>
#include <time.h>

#include <QtCore/QEventLoop>
#include <QList>
#include <QtCore/QDir>

#include <kapplication.h>
#include <kio/directorysizejob.h>
#include <kio/jobuidelegate.h>
#include <kde_file.h>

#include "../krusader.h"
#include "../defaults.h"
#include "kiojobwrapper.h"

vfs::vfs(QObject* panel, bool quiet): vfs_busy(false), quietMode(quiet),disableRefresh(false),postponedRefreshURL(),
                                      invalidated(true),panelConnected(false),vfs_tempFilesP(0),vfileIterator(0),deletePossible( true ),
                                      deleteRequested( false ) {
		

	setVfsFilesP( new vfileDict() );
	if ( panel ){
		panelConnected = true;
		connect(this,SIGNAL(startUpdate()),panel,SLOT(slotStartUpdate()));
		connect(this,SIGNAL(incrementalRefreshFinished( const KUrl& )),panel,SLOT(slotGetStats( const KUrl& )));
	}
	else quietMode = true;
}

vfs::~vfs() {
	if( !deletePossible )
		fprintf( stderr, "INTERNAL ERROR: trying to delete vfs while it is used! This may cause crash. Hoping the best...\n" );
	clear(); // please don't remove this line. This informs the view about deleting the references
	delete vfs_filesP;
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

bool vfs::vfs_refresh(KJob* job){
  if( job && job->error() ){
		if( job->uiDelegate() )
			job->uiDelegate()->showErrorMessage();
	}
	return vfs_refresh(vfs_origin);
}

QString vfs::pathOrUrl( const KUrl &originIn, KUrl::AdjustPathOption trailingSlash )
{
  if( originIn.isLocalFile() )
    return originIn.path( trailingSlash );
  return originIn.prettyUrl( trailingSlash );
}  

void vfs::setVfsFilesP(vfileDict* dict){
	vfs_filesP=dict;
	vfs_tempFilesP = new vfileDict();
	vfileIterator = vfs_filesP->begin();
}

bool vfs::vfs_refresh(){ 
	if( vfs_busy )
		return false;
	
	if( invalidated ) // invalidated fs requires total refresh
		return vfs_refresh( vfs_getOrigin() );
	
	if( disableRefresh )
	{
		postponedRefreshURL = vfs_getOrigin();
		return false;
	}
	
	vfs_busy = true;
	// and populate it
	KConfigGroup ga( krConfig, "Advanced");
	int maxIncrementalRefreshFileNr = ga.readEntry("Max Incremental Refresh File Nr", 50);
	KConfigGroup gl( krConfig, "Look&Feel");
	bool showHidden = gl.readEntry("Show Hidden",_ShowHidden);
	bool res = populateVfsList(vfs_getOrigin(),showHidden);

	QString name;
	if( res ){
		// check if the maximum incremental refresh number is achieved
		int diff = vfs_filesP->count() - vfs_tempFilesP->count();
		if( diff < 0 )
			diff = -diff;
		if( diff >= maxIncrementalRefreshFileNr )
		{
			// total filesystem refresh is cheaper than incremental refresh for many files
			clear();
			delete vfs_filesP;
			setVfsFilesP( vfs_tempFilesP );
			vfs_busy = false;
			
			emit startUpdate();
			return true;
		}

		// compare the two list emiting signals when needed;;
		for( vfile* vf = vfs_getFirstFile(); vf ;  ){
			name = vf->vfile_getName();
			vfile* newVf = (*vfs_tempFilesP)[name];
			if( !newVf ){
				// the file was deleted..
				emit deletedVfile(name);
				delete (*vfs_filesP)[ name ]; 
				vfileIterator  = vfs_filesP->erase( vfs_filesP->find( name ) );
				// the remove() advance our iterator !
				if( vfileIterator ==  vfs_filesP->end() )
					vf = 0;
				else
					vf = *vfileIterator;
			} else {
				if( *vf != *newVf ){
					// the file was changed..
					*vf = *newVf;
					emit updatedVfile(vf);
				}
				vf=vfs_getNextFile();
			}
			delete (*vfs_tempFilesP)[ name ]; 
			vfs_tempFilesP->remove(name);
		} 
		// everything thats left is a new file
		QHashIterator<QString, vfile *> it(*vfs_tempFilesP);
		while( it.hasNext() )
		{
			vfile *vf = it.next().value();
			
			// sanity checking
			if( !vf || (*vfs_filesP)[vf->vfile_getName()] ) continue;
			
			vfile* newVf = new vfile();
			*newVf = *vf;
			vfs_filesP->insert(newVf->vfile_getName(),newVf);
			emit addedVfile(newVf);
		}
	}
	
	// delete the temporary vfiles
	vfs_tempFilesP->clear();
	vfs_busy = false;
	
	emit incrementalRefreshFinished( vfs_origin );
	
	return res; 
}

bool vfs::vfs_refresh(const KUrl& origin){
	if( vfs_busy )
		return false;
	
	if( disableRefresh )
	{
		postponedRefreshURL = origin;
		if( !origin.equals(vfs_getOrigin(),KUrl::CompareWithoutTrailingSlash) )
			invalidated = true;
		vfs_origin = origin;
		return true;
	}

	if( !invalidated && origin.equals(vfs_getOrigin(),KUrl::CompareWithoutTrailingSlash) ) return vfs_refresh();
	
	vfs_busy = true;
	
	KConfigGroup group( krConfig, "Look&Feel");
	bool showHidden = group.readEntry("Show Hidden",_ShowHidden);

	vfs_tempFilesP->clear();
	// and re-populate it
	if (!populateVfsList(origin,showHidden) ) 
	{
		vfs_busy = false;
		return false;
	}
	
	clear();
	delete vfs_filesP;
	setVfsFilesP( vfs_tempFilesP );
	vfs_busy = false;
	
	emit startUpdate();
	
	invalidated = false;
	return true;
}

bool vfs::vfs_enableRefresh(bool enable){
	if (vfs_type != VFS_NORMAL) return true;
	if (disableRefresh == !enable) return true; // if gets called twice by mistake
	disableRefresh = quietMode = !enable;
	bool res = true;
	if( enable && !postponedRefreshURL.isEmpty() ) res = vfs_refresh( postponedRefreshURL );
	postponedRefreshURL = KUrl();
	return res;
}

void vfs::clear()
{
	emit cleared();
	QHashIterator< QString, vfile *> lit( *vfs_filesP );
	while( lit.hasNext() )
		delete lit.next().value();
	vfs_filesP->clear();
}

bool vfs::vfs_processEvents() {
	if( deleteRequested )
		return false;        
	deletePossible = false;
	qApp->processEvents( QEventLoop::AllEvents | QEventLoop::WaitForMoreEvents );
	deletePossible = true;        
	if( deleteRequested ) {
		emit deleteAllowed();
		return false;
	}        
	return true;                
}

void vfs::vfs_requestDelete() {
	if( deletePossible )
		emit deleteAllowed();
	deleteRequested = true;
}

/// to be implemented
void vfs::slotKdsResult( KJob* job){
	if( job && !job->error() ){
		KIO::DirectorySizeJob* kds = static_cast<KIO::DirectorySizeJob*>(job);
		*kds_totalSize += kds->totalSize();
		*kds_totalFiles += kds->totalFiles();
		*kds_totalDirs += kds->totalSubdirs();
	}
	*kds_busy = true;
}

void vfs::vfs_calcSpace( QString name , KIO::filesize_t* totalSize, unsigned long* totalFiles, unsigned long* totalDirs, bool* stop ) {
	calculateURLSize( vfs_getFile( name ), totalSize, totalFiles, totalDirs, stop );
}        
        
void vfs::calculateURLSize( KUrl url,  KIO::filesize_t* totalSize, unsigned long* totalFiles, unsigned long* totalDirs, bool* stop ) {
	if ( stop && *stop ) return ;        
	kds_busy = stop;
	kds_totalSize  = totalSize ;
	kds_totalFiles = totalFiles;
	kds_totalDirs  = totalDirs;

	if( url.isLocalFile() ) {
		vfs_calcSpaceLocal( url.path(KUrl::RemoveTrailingSlash), totalSize, totalFiles, totalDirs, stop );
		return;
	} else {
		stat_busy = true;
		KIOJobWrapper * statJob = KIOJobWrapper::stat( url );
		statJob->connectTo( SIGNAL( result( KJob* ) ), this, SLOT( slotStatResultArrived( KJob* ) ) );
		statJob->start();
		while ( !(*stop) && stat_busy ) {usleep(1000);}
		
		if( entry.count() == 0 ) return; // statJob failed
		KFileItem kfi(entry, url, true );        
		if( kfi.isFile() || kfi.isLink() ) {
			*totalFiles++;
			*totalSize += kfi.size();
			return;
		}
	}
	
	KIOJobWrapper* kds  = KIOJobWrapper::directorySize( url );
	kds->connectTo( SIGNAL( result( KJob* ) ), this, SLOT( slotKdsResult( KJob* ) ) );
	kds->start();
	while ( !(*stop) ){ 
		// we are in a sepetate thread - so sleeping is OK
		usleep(1000);
	}
}

void vfs::vfs_calcSpaceLocal(QString name ,KIO::filesize_t *totalSize,unsigned long *totalFiles,unsigned long *totalDirs, bool* stop){
  if ( *stop ) return;
  if (!name.contains("/")) name = vfs_workingDir() + '/' + name;
  if (name == "/proc") return;

  KDE_struct_stat stat_p;                // KDE lstat is necessary as QFileInfo and KFileItem 
  // if the name is wrongly encoded, then we zero the size out
  stat_p.st_size = 0;
  stat_p.st_mode = 0;
  KDE_lstat(name.toLocal8Bit(),&stat_p);   //         reports wrong size for a symbolic link
  
  if( S_ISLNK(stat_p.st_mode) || !S_ISDIR(stat_p.st_mode) ) { // single files are easy : )
    ++(*totalFiles);
    (*totalSize) += stat_p.st_size;
  }
  else{  // handle directories
    // avoid a nasty crash on un-readable dirs
    bool readable = ::access( name.toLocal8Bit(), R_OK | X_OK ) == 0;
    if( !readable )
      return;
      
    QDir dir(name);    
    if ( !dir.exists() ) return;
    
    ++(*totalDirs);
    dir.setFilter(QDir::TypeMask | QDir::System | QDir::Hidden);
    dir.setSorting(QDir::Name | QDir::DirsFirst);

    // recurse on all the files in the directory
    QFileInfoList fileList = dir.entryInfoList();
    for (int k=0; k != fileList.size(); k++){
      if ( *stop ) return;
      QFileInfo qfiP = fileList[ k ];
      if (qfiP.fileName() != "." && qfiP.fileName() != "..")
        vfs_calcSpaceLocal(name + '/' + qfiP.fileName(), totalSize, totalFiles, totalDirs, stop);
    }
  }
}

        
void vfs::slotStatResultArrived( KJob* job ) {
	if( !job || job->error() ) entry = KIO::UDSEntry();
	else entry = static_cast<KIO::StatJob*>(job)->statResult();
	stat_busy = false;
}

QList<vfile*> vfs::vfs_search(const KRQuery& filter) {
	QList<vfile*> result;
	for ( vfile *vf = vfs_getFirstFile(); vf != 0 ; vf = vfs_getNextFile() )
		if (filter.match(vf)) 
			result.append(vf);
	return result;
}

bool vfs::vfs_showHidden() {
	KConfigGroup gl( krConfig, "Look&Feel");
	return gl.readEntry("Show Hidden",_ShowHidden);
}

#include "vfs.moc"
