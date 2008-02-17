/***************************************************************************
                     kurllistrequester.h  -  description
                             -------------------
    copyright            : (C) 2005 by Csaba Karai
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

#ifndef __KURLLISTREQUESTER_H__
#define __KURLLISTREQUESTER_H__

#include <qwidget.h>
#include "../GUI/krlistwidget.h"
#include <qtoolbutton.h>
#include <QKeyEvent>
#include <klineedit.h>
#include <kurl.h>
#include <kurlcompletion.h>

class KURLListRequester : public QWidget
{
  Q_OBJECT
  
public:
  KURLListRequester( QWidget *parent = 0 );  
  
  KUrl::List   urlList();
  void         setUrlList( KUrl::List );
  
  KLineEdit    *lineEdit()    {return urlLineEdit;}
  KrListWidget *listBox()     {return urlListBox;}
  
  void         setCompletionDir( QString dir ) { completion.setDir( dir ); }

signals:
  void         checkValidity( QString &text, QString &error );
  
protected slots:
  void         slotAdd();
  void         slotBrowse();
  void         slotRightClicked( QListWidgetItem *, const QPoint & );
  
protected:
  virtual void keyPressEvent(QKeyEvent *e);
  void         deleteSelectedItems();
  
  KLineEdit    *urlLineEdit;
  KrListWidget *urlListBox;
  QToolButton  *urlAddBtn;
  QToolButton  *urlBrowseBtn;
  
  KUrlCompletion completion;
};

#endif /* __KURLLISTREQUESTER_H__ */
