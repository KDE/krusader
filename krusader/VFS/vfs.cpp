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
#include <kapp.h>
#include <time.h>

vfs::vfs(QObject* panel, bool quiet): error(false),quietMode(quiet){
		if ( panel ){
	 		connect(this,SIGNAL(startUpdate()),panel,SLOT(slotStartUpdate()));
    	connect(this,SIGNAL(endUpdate()),panel,SLOT(slotEndUpdate()));
		}
		else quietMode = true;
}

long vfs::vfs_totalSize(){
	long temp=0;
	class vfile* vf=vfs_getFirstFile();
		
	while (vf!=0){
		if ( (vf->vfile_getName() != ".") && ( vf->vfile_getName() != "..")
		     && !(vf->vfile_isDir()) )
				temp+=vf->vfile_getSize();
		vf=vfs_getNextFile();
	}
	return temp;
}

vfile* vfs::vfs_search(QString name){	
	vfile* temp = vfs_getFirstFile();
		
	while (temp!=0){
		if (temp->vfile_getName()==name) return temp;
		temp=vfs_getNextFile();
	}
	return 0;
}

QString vfs::round(int i){
	QString t;
	t.sprintf("%d",i);
	if(i<10) t=("0"+t);
	return t;
}

QString vfs::month2Qstring(QString month){
	if(month.lower() == "jan" ) return QString("01");
	if(month.lower() == "feb" ) return QString("02");
	if(month.lower() == "mar" ) return QString("03");
	if(month.lower() == "apr" ) return QString("04");
	if(month.lower() == "may" ) return QString("05");
	if(month.lower() == "jun" ) return QString("06");
	if(month.lower() == "jul" ) return QString("07");
	if(month.lower() == "aug" ) return QString("08");
	if(month.lower() == "sep" ) return QString("09");
	if(month.lower() == "oct" ) return QString("10");
	if(month.lower() == "nov" ) return QString("11");
	if(month.lower() == "dec" ) return QString("12");
	return QString("00");
}

// create a easy to read date-time format
QString vfs::dateTime2QString(const QDateTime& datetime){
	QString dateTime;
	QDate date = datetime.date();
	QTime time = datetime.time();
	
	// construct the string
	dateTime=round(date.day())+"/"+round(date.month())+
	        "/"+round(date.year()%100)+
		     +" "+round(time.hour())+":"+round(time.minute());
	return dateTime;
}

#include "vfs.moc"
