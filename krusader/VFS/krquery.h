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
#include <qdatetime.h>
#include <time.h>
#include <kurl.h>
#include <kio/jobclasses.h>
#include "vfile.h"

class KFileItem;
class QTextCodec;

class KRQuery : public QObject {
  Q_OBJECT

public:
  // null query
  KRQuery();
  // query only with name filter
  KRQuery( const QString &name, bool matchCase = true );
  // copy constructor
  KRQuery( const KRQuery & );
  // let operator
  KRQuery& operator=(const KRQuery &);
  // destructor
  virtual ~KRQuery();

  // matching a file with the query
  bool match( vfile *file ) const;// checks if the given vfile object matches the conditions
  // matching a KIO file with the query
  bool match( KFileItem *file ) const;// checks if the given vfile object matches the conditions
  // matching a name with the query
  bool match( const QString &name ) const;// matching the filename only
  // matching the name of the directory
  bool matchDirName( const QString &name ) const;

  // sets the text for name filtering
  void setNameFilter( const QString &text, bool cs=true );
  // returns the current filter mask
  const QString& nameFilter() const { return origFilter; }
  // returns whether the filter is case sensitive
  bool isCaseSensitive() { return matchesCaseSensitive; }

  // returns if the filter is null (was cancelled)
  bool isNull() {return bNull;};

  // sets the content part of the query
  void setContent( const QString &content, bool cs=true, bool wholeWord=false, bool remoteSearch=false, QString encoding=QString(), bool regExp=false);

  // sets the minimum file size limit
  void setMinimumFileSize( KIO::filesize_t );
  // sets the maximum file size limit
  void setMaximumFileSize( KIO::filesize_t );

  // sets the time the file newer than
  void setNewerThan( time_t time );
  // sets the time the file older than
  void setOlderThan( time_t time );

  // sets the owner
  void setOwner( const QString &ownerIn );
  // sets the group
  void setGroup( const QString &groupIn );
  // sets the permissions
  void setPermissions( const QString &permIn );

  // sets the mimetype for the query
  // type, must be one of the following:
  // 1. a valid mime type name
  // 2. one of: i18n("Archives"),   i18n("Directories"), i18n("Image Files")
  //            i18n("Text Files"), i18n("Video Files"), i18n("Audio Files")
  // 3. i18n("Custom") in which case you must supply a list of valid mime-types
  //    in the member QStringList customType
  void setMimeType( const QString &typeIn, QStringList customList = QStringList() );
  // true if setMimeType was called
  bool hasMimeType()  { return type.isEmpty(); }

  // sets the search in archive flag
  void setSearchInArchives( bool flag ) { inArchive = flag; }
  // gets the search in archive flag
  bool searchInArchives() { return inArchive; }
  // sets the recursive flag
  void setRecursive( bool flag ) { recurse = flag; }
  // gets the recursive flag
  bool isRecursive() { return recurse; }
  // sets whether to follow symbolic links
  void setFollowLinks( bool flag ) { followLinksP = flag; }
  // gets whether to follow symbolic links
  bool followLinks() { return followLinksP; }

  // sets the folders where the searcher will search
  void setSearchInDirs( const KUrl::List &urls );
  // gets the folders where the searcher searches
  const KUrl::List & searchInDirs() { return whereToSearch; }
  // sets the folders where search is not permitted
  void setDontSearchInDirs( const KUrl::List &urls );
  // gets the folders where search is not permitted
  const KUrl::List & dontSearchInDirs() { return whereNotToSearch; }
  // checks if a URL is excluded
  bool isExcluded( const KUrl &url );
  // gives whether we search for content
  bool isContentSearched() const { return !contain.isEmpty(); }
  
  const QString& foundText() const { return lastSuccessfulGrep; }

protected:
  // important to know whether the event processor is connected
  virtual void connectNotify ( const char * signal );
  // important to know whether the event processor is connected
  virtual void disconnectNotify ( const char * signal );

protected:
  QStringList matches;           // what to search
  QStringList excludes;          // what to exclude
  QStringList includedDirs;      // what dirs to include
  QStringList excludedDirs;      // what dirs to exclude
  bool matchesCaseSensitive;

  bool bNull;                    // flag if the query is null

  QString contain;               // file must contain this string
  bool containCaseSensetive;
  bool containWholeWord;
  bool containRegExp;
  bool containOnRemote;

  KIO::filesize_t minSize;
  KIO::filesize_t maxSize;

  time_t newerThen;
  time_t olderThen;

  QString owner;
  QString group;
  QString perm;

  QString type;
  QStringList customType;

  bool inArchive;                // if true- search in archive.
  bool recurse;                  // if true recurse ob sub-dirs...
  bool followLinksP;

  KUrl::List whereToSearch;     // directorys to search
  KUrl::List whereNotToSearch;  // directorys NOT to search

signals:
  void status( const QString &name );
  void processEvents( bool & stopped );

private:
  bool matchCommon( const QString &, const QStringList &, const QStringList & ) const;
  bool checkPerm(QString perm) const;
  bool checkType(QString mime) const;
  bool containsContent( QString file ) const;
  bool containsContent( KUrl url ) const;
  bool checkBuffer( const char * data, int len ) const;
  bool checkLine( const QString &line ) const;
  bool checkTimer() const;
  QStringList split( QString );

private slots:
  void containsContentData(KIO::Job *, const QByteArray &);
  void containsContentFinished(KJob*);

private:
  QString                  origFilter;
  mutable bool             busy;
  mutable bool             containsContentResult;
  mutable char *           receivedBuffer;
  mutable int              receivedBufferLen;
  mutable QString          lastSuccessfulGrep;
  mutable QString          fileName;
  mutable KIO::filesize_t  receivedBytes;
  mutable KIO::filesize_t  totalBytes;
  mutable int              processEventsConnected;
  mutable QTime            timer;

  QTextCodec *             codec;

  const char *             encodedEnter;
  int                      encodedEnterLen;
  QByteArray               encodedEnterArray;
};

#endif
