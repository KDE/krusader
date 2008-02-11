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

#include "../GUI/krtreewidget.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include "diskusage.h"

class DUListViewItem : public QTreeWidgetItem
{
public:
  DUListViewItem( DiskUsage *diskUsageIn, File *fileIn, QTreeWidget * parent, QString label1, 
                  QString label2, QString label3, QString label4, QString label5, QString label6, 
                  QString label7, QString label8, QString label9 ) 
                  : QTreeWidgetItem( parent ), diskUsage( diskUsageIn ), file( fileIn )
                  {
                    setText( 0, label1 );
                    setText( 1, label2 );
                    setText( 2, label3 );
                    setText( 3, label4 );
                    setText( 4, label5 );
                    setText( 5, label6 );
                    setText( 6, label7 );
                    setText( 7, label8 );
                    setText( 8, label9 );

                    setTextAlignment( 1, Qt::AlignRight );
                    setTextAlignment( 2, Qt::AlignRight );
                    setTextAlignment( 3, Qt::AlignRight );

                    setChildIndicatorPolicy( QTreeWidgetItem::DontShowIndicatorWhenChildless );
                    diskUsage->addProperty( file, "ListView-Ref", this );
                  }
  DUListViewItem( DiskUsage *diskUsageIn, File *fileIn, QTreeWidgetItem * parent, QString label1, 
                  QString label2, QString label3, QString label4, QString label5, QString label6, 
                  QString label7, QString label8, QString label9 ) 
                  : QTreeWidgetItem( parent ), diskUsage( diskUsageIn ), file( fileIn ) 
                  {
                    setText( 0, label1 );
                    setText( 1, label2 );
                    setText( 2, label3 );
                    setText( 3, label4 );
                    setText( 4, label5 );
                    setText( 5, label6 );
                    setText( 6, label7 );
                    setText( 7, label8 );
                    setText( 8, label9 );

                    setTextAlignment( 1, Qt::AlignRight );
                    setTextAlignment( 2, Qt::AlignRight );
                    setTextAlignment( 3, Qt::AlignRight );

                    setChildIndicatorPolicy( QTreeWidgetItem::DontShowIndicatorWhenChildless );
                    diskUsage->addProperty( file, "ListView-Ref", this );
                  }
  DUListViewItem( DiskUsage *diskUsageIn, File *fileIn, QTreeWidget * parent, QTreeWidgetItem * after, 
                  QString label1, QString label2, QString label3, QString label4, QString label5, 
                  QString label6, QString label7, QString label8, QString label9 )
                  : QTreeWidgetItem( parent, after ), diskUsage( diskUsageIn ), file( fileIn ) 
                  {
                    setText( 0, label1 );
                    setText( 1, label2 );
                    setText( 2, label3 );
                    setText( 3, label4 );
                    setText( 4, label5 );
                    setText( 5, label6 );
                    setText( 6, label7 );
                    setText( 7, label8 );
                    setText( 8, label9 );

                    setTextAlignment( 1, Qt::AlignRight );
                    setTextAlignment( 2, Qt::AlignRight );
                    setTextAlignment( 3, Qt::AlignRight );

                    setChildIndicatorPolicy( QTreeWidgetItem::DontShowIndicatorWhenChildless );
                    diskUsage->addProperty( file, "ListView-Ref", this );
                  }
  DUListViewItem( DiskUsage *diskUsageIn, File *fileIn, QTreeWidgetItem * parent, QTreeWidgetItem * after, 
                  QString label1, QString label2, QString label3, QString label4, QString label5, 
                  QString label6, QString label7, QString label8, QString label9 )   
                  : QTreeWidgetItem( parent, after ), 
                  diskUsage( diskUsageIn ), file( fileIn ) 
                  {
                    setText( 0, label1 );
                    setText( 1, label2 );
                    setText( 2, label3 );
                    setText( 3, label4 );
                    setText( 4, label5 );
                    setText( 5, label6 );
                    setText( 6, label7 );
                    setText( 7, label8 );
                    setText( 8, label9 );

                    setTextAlignment( 1, Qt::AlignRight );
                    setTextAlignment( 2, Qt::AlignRight );
                    setTextAlignment( 3, Qt::AlignRight );

                    setChildIndicatorPolicy( QTreeWidgetItem::DontShowIndicatorWhenChildless );
                    diskUsage->addProperty( file, "ListView-Ref", this );
                  }
  ~DUListViewItem()
                  {
                    diskUsage->removeProperty( file, "ListView-Ref" );
                  }
  
  virtual bool operator<(const QTreeWidgetItem &other) const
  {
      int column = treeWidget() ? treeWidget()->sortColumn() : 0;

      if( text( 0 ) == ".." )
        return true;

      const DUListViewItem *compWith = dynamic_cast< const DUListViewItem * >( &other );
      if( compWith == 0 )
        return false;

      switch( column )
      {
      case 1:
      case 2:
        return file->size() > compWith->file->size();
      case 3:
        return file->ownSize() > compWith->file->ownSize();
      case 5:
        return file->time() < compWith->file->time();
      default:
        return text(column) < other.text(column);
      }
  }

  inline File * getFile() { return file; }
  
private:
  DiskUsage *diskUsage;
  File *file;
};

class DUListView : public KrTreeWidget
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
  void slotRightClicked(QTreeWidgetItem *, const QPoint &);
  void slotExpanded( QTreeWidgetItem * );
    
protected:
  DiskUsage *diskUsage;
  
  virtual void mouseDoubleClickEvent ( QMouseEvent * e );
  virtual void keyPressEvent( QKeyEvent *e );
    
private:
  void addDirectory( Directory *dirEntry, QTreeWidgetItem *parent );
  bool doubleClicked( QTreeWidgetItem * item );
};

#endif /* __DU_LISTVIEW_H__ */

