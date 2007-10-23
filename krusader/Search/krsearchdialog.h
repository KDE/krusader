/***************************************************************************
                                 krsearchdialog.h
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



#ifndef KRSEARCHDIALOG_H
#define KRSEARCHDIALOG_H

#include "../Filter/filtertabs.h"
#include "../Filter/generalfilter.h"
#include "../VFS/krquery.h"
#include "../VFS/krpermhandler.h"
#include "krsearchmod.h"
#include "../GUI/profilemanager.h"

#include <qwidget.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QGridLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QCloseEvent>
#include <ksqueezedtextlabel.h>
#include <qstringlist.h>
#include <sys/types.h>
#include <time.h>
#include <qstring.h>
#include <qtabwidget.h>
#include <q3listview.h>
#include <qstringlist.h>
#include <kglobal.h>
#include <klocale.h>

class KrSearchDialog : public QDialog  {
   Q_OBJECT
public: 
  KrSearchDialog(QString profile = 0, QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0 );

  void prepareGUI();
    
  static KrSearchDialog *SearchDialog;
  
public slots:
  void startSearch();
  void stopSearch();
  void feedToListBox();
  void copyToClipBoard();
  void found(QString what, QString where, KIO::filesize_t size, time_t mtime, QString perm, QString foundText);
  void closeDialog( bool isAccept = true );
  void resultDoubleClicked(Q3ListViewItem*);
  void resultClicked(Q3ListViewItem*);

  virtual void keyPressEvent(QKeyEvent *e);
  virtual void closeEvent(QCloseEvent *e);
  virtual void rightClickMenu(Q3ListViewItem*, const QPoint&, int);
  virtual void resizeEvent( QResizeEvent *e );

protected slots:
  void reject();

private:
  bool gui2query();
  void editCurrent();
  void viewCurrent();

private:
  ProfileManager *profileManager;

  FilterTabs * filterTabs;
  GeneralFilter * generalFilter;
      
  QPushButton* mainHelpBtn;
  QPushButton* mainSearchBtn;
  QPushButton* mainStopBtn;
  QPushButton* mainCloseBtn;
  QPushButton* mainFeedToListBoxBtn;
  
  QTabWidget* searcherTabs;  
  QWidget* resultTab;
  QGridLayout* resultLayout;
  QLabel* foundLabel;
  KrSqueezedTextLabel *foundTextLabel;
  KSqueezedTextLabel *searchingLabel;
  
  Q3ListView* resultsList;

  KRQuery *query;
  KRSearchMod *searcher;
  QStringList savedSearches;
  bool isSearching;
  bool closed;
  
  static QString lastSearchText;
  static int     lastSearchType;
  static bool    lastSearchForCase;
  static bool    lastRemoteContentSearch;
  static bool    lastContainsWholeWord;
  static bool    lastContainsWithCase;
  static bool    lastSearchInSubDirs;
  static bool    lastSearchInArchives;
  static bool    lastFollowSymLinks;
  
  int            sizeX;
  int            sizeY;
};

class ResultListViewItem : public Q3ListViewItem
{
public:
  ResultListViewItem( Q3ListView *resultsList, QString name, QString where, KIO::filesize_t size, 
                      QDateTime date, QString perm ) : Q3ListViewItem( resultsList, name, where, 
                      KRpermHandler::parseSize(size), 
                      KGlobal::locale()->formatDateTime( date ), perm )
  {
    fileSize = size;
    fileDate = date;
    setDragEnabled( true );
  }  

  void setFoundText(QString text) { _foundText=text; }
  const QString& foundText() const { return _foundText; }
  
  virtual int compare(Q3ListViewItem *i,int col,bool ascending ) const
  {
    if( col == 2 ) {
      ResultListViewItem *other = (ResultListViewItem *)i;
      KIO::filesize_t otherSize = other->getSize();
      
      if( fileSize == otherSize )
        return 0;
      if( fileSize > otherSize )
        return 1;
      return -1;
    }
    if( col == 3 ) {
      ResultListViewItem *other = (ResultListViewItem *)i;
      QDateTime otherDate = other->getDate();
      
      if( fileDate == otherDate )
        return 0;
      if( fileDate > otherDate )
        return 1;
      return -1;
    }
    return Q3ListViewItem::compare( i, col, ascending );
  }

  KIO::filesize_t getSize() {
    return fileSize;
  }

  QDateTime getDate() {
    return fileDate;
  }
  
private:
  KIO::filesize_t fileSize;
  QDateTime       fileDate;
  QString _foundText;
};

#endif
