/***************************************************************************
                          dirhistorybutton.cpp  -  description
                             -------------------
    begin                : Sun Jan 4 2004
    copyright            : (C) 2004 by Shie Erlich & Rafi Yanai
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dirhistorybutton.h"
#include "dirhistoryqueue.h"

#include "../VFS/vfs.h"
#include <qpopupmenu.h>
#include <qdir.h>
#include <klocale.h>
#include <kiconloader.h>

#include <kdebug.h>

DirHistoryButton::DirHistoryButton(DirHistoryQueue* hQ, QWidget *parent, const char *name) : QToolButton(parent,name)
{
  KIconLoader *iconLoader = new KIconLoader();
  QPixmap icon = iconLoader->loadIcon("history", KIcon::Toolbar, 16);

  setFixedSize(icon.width() + 4, icon.height() + 4);
  setPixmap(icon);
  setTextLabel(i18n("Open the directory history list"), true);
  setPopupDelay(10); // 0.01 seconds press
  setAcceptDrops(false);

  popupMenu = new QPopupMenu(this);
  Q_CHECK_PTR(popupMenu);
  
  setPopup(popupMenu);
  popupMenu->setCheckable(true);

  historyQueue = hQ;

  connect(popupMenu, SIGNAL(aboutToShow()), this, SLOT(slotAboutToShow()));
  connect(popupMenu, SIGNAL(activated(int)), this, SLOT(slotPopupActivated(int)));
}

DirHistoryButton::~DirHistoryButton(){
}

void DirHistoryButton::openPopup()
{
  QPopupMenu* pP = popup();
  if (pP)
  {
    popup()->exec(mapToGlobal(QPoint(0, height())));
  }
}
/** No descriptions */
void DirHistoryButton::slotPopup(){
//  kdDebug() << "History slot" << endl;
}
/** No descriptions */
void DirHistoryButton::slotAboutToShow(){
//  kdDebug() << "about to show" << endl;
  popupMenu->clear();
  QStringList::iterator it;

  int id = 0;
  for (it=historyQueue->pathQueue.begin(); it!=historyQueue->pathQueue.end(); ++it)
  {
    popupMenu->insertItem(*it, id++);
  }
  if (id > 0)
  {
    popupMenu->setItemChecked(0, true);
  }
}  
/** No descriptions */
void DirHistoryButton::slotPopupActivated(int id){
//  kdDebug() << "popup activated" << endl;
  KURL url(historyQueue->pathQueue[id]);
  if (historyQueue->checkPath(historyQueue->pathQueue[id]))
  {
    emit openUrl( vfs::fromPathOrURL( url.prettyURL() ) );
  }
}

#include "dirhistorybutton.moc"
