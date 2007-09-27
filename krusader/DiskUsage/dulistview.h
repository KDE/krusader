/***************************************************************************
                          dulistview.h  -  description
                             -------------------
    copyright            : (C) 2004 by Csaba Karai
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

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __DU_LISTVIEW_H__
#define __DU_LISTVIEW_H__

#include <q3listview.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>
#include "diskusage.h"

class DUListViewItem : public Q3ListViewItem
{
public:
  DUListViewItem( DiskUsage *diskUsageIn, File *fileIn, Q3ListView * parent, QString label1, 
                  QString label2, QString label3, QString label4, QString label5, QString label6, 
                  QString label7, QString label8, QString label9 ) 
                  : Q3ListViewItem( parent, label1, label2, label3, label4, label5, label6, label7, label8), 
                  diskUsage( diskUsageIn ), file( fileIn ) 
                  {
                    setText( 8, label9 );
                    diskUsage->addProperty( file, "ListView-Ref", this );
                  }
  DUListViewItem( DiskUsage *diskUsageIn, File *fileIn, Q3ListViewItem * parent, QString label1, 
                  QString label2, QString label3, QString label4, QString label5, QString label6, 
                  QString label7, QString label8, QString label9 ) 
                  : Q3ListViewItem( parent, label1, label2, label3, label4, label5, label6, label7, label8), 
                  diskUsage( diskUsageIn ), file( fileIn ) 
                  {
                    setText( 8, label9 );
                    diskUsage->addProperty( file, "ListView-Ref", this );
                  }
  DUListViewItem( DiskUsage *diskUsageIn, File *fileIn, Q3ListView * parent, Q3ListViewItem * after, 
                  QString label1, QString label2, QString label3, QString label4, QString label5, 
                  QString label6, QString label7, QString label8, QString label9 )   
                  : Q3ListViewItem( parent, after, label1, label2, label3, label4, label5, label6, label7, label8), 
                  diskUsage( diskUsageIn ), file( fileIn ) 
                  {
                    setText( 8, label9 );
                    diskUsage->addProperty( file, "ListView-Ref", this );
                  }
  DUListViewItem( DiskUsage *diskUsageIn, File *fileIn, Q3ListViewItem * parent, Q3ListViewItem * after, 
                  QString label1, QString label2, QString label3, QString label4, QString label5, 
                  QString label6, QString label7, QString label8, QString label9 )   
                  : Q3ListViewItem( parent, after, label1, label2, label3, label4, label5, label6, label7, label8), 
                  diskUsage( diskUsageIn ), file( fileIn ) 
                  {
                    setText( 8, label9 );
                    diskUsage->addProperty( file, "ListView-Ref", this );
                  }
  ~DUListViewItem()
                  {
                    diskUsage->removeProperty( file, "ListView-Ref" );
                  }
  
  virtual int compare ( Q3ListViewItem * i, int col, bool ascending ) const 
  {
    if( text(0) == ".." ) return ascending ? -1 : 1;
    if( i->text(0) == "..") return ascending ? 1 : -1;
    
    DUListViewItem *compWith = dynamic_cast< DUListViewItem * >( i );
        
    QString buf1,buf2;
    
    switch( col )
    {
    case 1:    
    case 2:
      buf1.sprintf("%025llu",file->size());
      buf2.sprintf("%025llu",compWith->file->size());
      return -QString::compare( buf1, buf2 );
    case 3:
      buf1.sprintf("%025llu",file->ownSize());
      buf2.sprintf("%025llu",compWith->file->ownSize());
      return -QString::compare( buf1, buf2 );
    case 5:
      return Q3ListViewItem::compare( i, col, !ascending );
    default:    
      return Q3ListViewItem::compare( i, col, ascending );
    }
  }
  
  inline File * getFile() { return file; }
  
private:
  DiskUsage *diskUsage;
  File *file;                  
};

class DUListView : public Q3ListView
{
  Q_OBJECT
  
public:
  DUListView( DiskUsage *usage );
  ~DUListView();

  File * getCurrentFile();
      
public slots:
  void slotDirChanged( Directory * );
  void slotChanged( File * );
  void slotDeleted( File * );
  void slotRightClicked(Q3ListViewItem *);
  void slotExpanded( Q3ListViewItem * );
    
protected:
  DiskUsage *diskUsage;
  
  virtual void contentsMouseDoubleClickEvent ( QMouseEvent * e );
  virtual void keyPressEvent( QKeyEvent *e );
    
private:
  void addDirectory( Directory *dirEntry, Q3ListViewItem *parent );
  bool doubleClicked( Q3ListViewItem * item );
};

#endif /* __DU_LISTVIEW_H__ */

