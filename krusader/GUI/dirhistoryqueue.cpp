/***************************************************************************
                          dirhistoryqueue.cpp  -  description
                             -------------------
    begin                : Thu Jan 1 2004
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

#include "dirhistoryqueue.h"
#include "../Panel/listpanel.h"

#include <kdebug.h>

DirHistoryQueue::DirHistoryQueue(ListPanel* p){
  panel = p;

  connect(panel, SIGNAL(pathChanged(ListPanel*)), this, SLOT(slotPathChanged(ListPanel*)));
}
DirHistoryQueue::~DirHistoryQueue(){
}

/** No descriptions */
void DirHistoryQueue::slotPathChanged(ListPanel* p){
//  int dummy = 1;
  AddPath(p->getPath());
}

void DirHistoryQueue::AddPath(const QString& path)
{
  if (pathQueue.findIndex(path) == -1)
  {
    if (pathQueue.size() > 12)
    {
      pathQueue.pop_front();
    }
//    DumpQueue();
  }
  else
  {
    pathQueue.remove(path);
  }

// prevent non file paths from occuring in the list
//  KURL url(path);
//  if (url.protocol() == "file")
  {
    pathQueue.push_front(path);
  }
}

void DirHistoryQueue::RemovePath(const QString& path)
{
  QStringList::iterator it;
  it = pathQueue.find(path);
  if (it != pathQueue.end())
  {
    pathQueue.remove(it);
  }
}

bool DirHistoryQueue::checkPath(const QString& path)
{
  KURL url(path);

  QString p = url.path();
//  kdDebug() << "url:" << p <<  ", file: " << url.fileName() << ", dir: " << url.directory() <<  endl;
  if (url.protocol() == "file")
  {
    QDir dir(path);
    if (!dir.exists())
    {
      RemovePath(path);
      return false;
    }
  }
    
  return true;
  
}

void DirHistoryQueue::DumpQueue()
{
  QStringList::iterator it;
  kdDebug() << "path: " << endl;
  for (it=pathQueue.begin(); it!=pathQueue.end(); ++it)
  {
    kdDebug() << *it << endl;
  }
}

