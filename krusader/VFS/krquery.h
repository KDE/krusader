/***************************************************************************
                                 krquery.h
                             -------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
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

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KRQUERY_H
#define KRQUERY_H

#include <qstringlist.h>
#include <time.h>
#include <kurl.h>
#include "vfile.h"



class KRQuery {
public: 
  KRQuery();
  KRQuery( QString name );
  ~KRQuery() {};

  bool match( vfile *file ) const;// checks if the given vfile object matches the conditions
  bool match( QString name ) const;// matching the filename only
  void normalize();               // make sure KRSearchMod can use the data
  void setFilter( QString text ); // sets the text for filtering
  QString filter() { return origFilter; } // returns the current filter mask
  bool isNull() {return bNull;};          

  QStringList matches;           // what to search
  QStringList excludes;          // what to exclude
  bool matchesCaseSensitive;
  QString contain;               // file must contain this string
  bool containCaseSensetive;
  bool containWholeWord;
  bool inArchive;                // if true- search in archive.
  bool recurse;                  // if true recurse ob sub-dirs...
  bool followLinks;
  bool bNull;                   // flag if the query is null
  
  KURL::List whereToSearch;     // directorys to search
  KURL::List whereNotToSearch;  // directorys NOT to search
  // size
  unsigned long minSize;
  unsigned long maxSize;
  //date
  time_t newerThen;
  time_t olderThen;
  //permissions
  QString owner;
  QString group;
  QString perm;
  // type, must be one of the following:
  // 1. a valid mime type name
  // 2. one of: i18n("Archives"),   i18n("Directories"), i18n("Image Files")
  //            i18n("Text Files"), i18n("Video Files"), i18n("Audio Files")
  // 3. i18n("Custom") in which case you must supply a list of valid mime-types
  //    in the member QStringList customType
  QString type;
  QStringList customType;

private:
  bool checkPerm(QString perm) const;
  bool checkType(QString mime) const;
  bool containsContent( QString file ) const;
  
  QString origFilter;
};

#endif
