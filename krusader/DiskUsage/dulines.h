/***************************************************************************
                           dulines.h  -  description
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

#ifndef __DU_LINES_H__
#define __DU_LINES_H__

#include <qpixmap.h>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include "diskusage.h"
#include "../GUI/krtreewidget.h"

class DULinesToolTip;
class DULinesItemDelegate;

class DULines : public KrTreeWidget
{
  Q_OBJECT
  
public:
  DULines( DiskUsage *usage );
  ~DULines();

  File * getCurrentFile();
  
public slots:
  void slotDirChanged( Directory *dirEntry );
  void sectionResized( int );
  void slotRightClicked(QTreeWidgetItem *, const QPoint &);
  void slotChanged( File * );
  void slotDeleted( File * );
  void slotShowFileSizes();
  void slotRefresh();
  
protected:
  DiskUsage *diskUsage;  
  
  virtual bool event ( QEvent * event );
  virtual void mouseDoubleClickEvent ( QMouseEvent * e );
  virtual void keyPressEvent( QKeyEvent *e );
  virtual void resizeEvent( QResizeEvent * );
  
private:
  QPixmap createPixmap( int percent, int maxPercent, int maxWidth );
  
  bool doubleClicked( QTreeWidgetItem * item );
  
  bool refreshNeeded;
  bool started;
  
  bool showFileSize;
  
  DULinesToolTip *toolTip;
  DULinesItemDelegate *itemDelegate;
};

#endif /* __DU_LINES_H__ */

