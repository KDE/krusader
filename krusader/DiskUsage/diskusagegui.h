/***************************************************************************
                        diskusagegui.h  -  description
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

#ifndef __DISK_USAGE_GUI_H__
#define __DISK_USAGE_GUI_H__

#include <qdialog.h>
#include <qlayout.h>
#include <qwidgetstack.h>
#include <qtoolbutton.h>
#include <kurl.h>
#include <ksqueezedtextlabel.h>

#include "diskusage.h"
#include "dulistview.h"
#include "dulines.h"

class DiskUsageGUI : public QDialog
{
  Q_OBJECT
  
public:
  DiskUsageGUI( QString openDir, QWidget* parent=0, char *name = 0 );
  ~DiskUsageGUI();
  
  void                       selectView( int viewNum );

public slots:
  void                       loadUsageInfo();
  bool                       newSearch();
  void                       setStatus( QString );
  
  void                       selectLinesView() { selectView( 0 ); }
  void                       selectListView()  { selectView( 1 ); }
  
protected slots:
  virtual void               reject();
  
protected:
  virtual void               resizeEvent( QResizeEvent *e );
  
  
  DiskUsage                  diskUsage;
  KURL                       baseDirectory;
  
  QWidgetStack              *viewStack;
  KSqueezedTextLabel        *status;
  
  DUListView                *listView;
  DULines                   *lineView;

  QToolButton               *btnLines;
  QToolButton               *btnDetailed;
  
  int                        sizeX;
  int                        sizeY;
};

#endif /* __DISK_USAGE_GUI_H__ */

