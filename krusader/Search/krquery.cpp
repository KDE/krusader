/***************************************************************************
                                 krquery.cpp
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



#include "krquery.h"
#include "../krusader.h"
#include "../resources.h"

// set the defaults
KRQuery::KRQuery(): matches(""),matchesCaseSensitive(true),
                    contain(QString::null),containCaseSensetive(true),
                    inArchive(false),recurse(true),followLinks(true),
                    minSize(0),maxSize(0),newerThen(0),olderThen(0),
                    owner(QString::null),group(QString::null),
                    perm(QString::null),type(QString::null){}

KRQuery::KRQuery(QString name){
  KRQuery(); // fill in the default values
  load(name);
}


KRQuery::~KRQuery(){}

void KRQuery::normalize(){
	// remove the trailing "/" from the directories lists
	for( QStringList::Iterator it = whereNotToSearch.begin();
    it != whereNotToSearch.end(); ++it ){

		if( (*it).right(1) != "/" && !((*it).isEmpty()) ){
  		(*it) = (*it)+"/";
		}
	}
}

void KRQuery::save(QString name){
  krConfig->setGroup("KRquery-"+name);

  krConfig->writeEntry("matches",matches);
  krConfig->writeEntry("matchesCaseSensitive",matchesCaseSensitive);
  krConfig->writeEntry("contain",contain);
  krConfig->writeEntry("containCaseSensetive",containCaseSensetive);
  krConfig->writeEntry("inArchive",inArchive);
  krConfig->writeEntry("recurse",recurse);
  krConfig->writeEntry("followLinks",followLinks);
	krConfig->writeEntry("whereToSearch",whereToSearch);
  krConfig->writeEntry("whereNotToSearch",whereNotToSearch);
	// size
	krConfig->writeEntry("minSize",minSize);
  krConfig->writeEntry("maxSize",maxSize);
  //date
  krConfig->writeEntry("newerThen",newerThen);
  krConfig->writeEntry("olderThen",olderThen);
  //permissions
  krConfig->writeEntry("owner",owner);
  krConfig->writeEntry("group",group);
  krConfig->writeEntry("perm", perm);
  //type
  krConfig->writeEntry("type", type);
  krConfig->sync();
}

void KRQuery::load(QString name){
  krConfig->setGroup("KRquery-"+name);

  matches              = krConfig->readListEntry("matches");
  //matches(amatches);
  exit(0);
/*  matchesCaseSensitive = krConfig->readBoolEntry("matchesCaseSensitive",true);
  contain              = krConfig->readEntry("contain");
  containCaseSensetive = krConfig->readBoolEntry("containCaseSensetive",true);
  inArchive            = krConfig->readBoolEntry("inArchive",false);
  recurse              = krConfig->readBoolEntry("recurse",true);
  followLinks          = krConfig->readBoolEntry("followLinks",false);
	whereToSearch        = krConfig->readListEntry("whereToSearch");
  whereNotToSearch     = krConfig->readListEntry("whereNotToSearch");
	// size
	minSize = krConfig->readUnsignedLongNumEntry("minSize",0);
  maxSize = krConfig->readUnsignedLongNumEntry("maxSize",0);
  //date
  newerThen = krConfig->readUnsignedLongNumEntry("newerThen",0);
  olderThen = krConfig->readUnsignedLongNumEntry("olderThen",0);
  //permissions
  owner = krConfig->readEntry("owner");
  group = krConfig->readEntry("group");
  perm  = krConfig->readEntry("perm");
  //type
  type  = krConfig->readEntry("type");*/
  kdWarning() << "finished" << endl;
}

