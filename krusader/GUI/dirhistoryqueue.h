/***************************************************************************
                          dirhistoryqueue.h  -  description
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

#ifndef DIRHISTORYQUEUE_H
#define DIRHISTORYQUEUE_H

#include <qobject.h>
#include <kurl.h>

class ListPanel;

/**
  *@author Shie Erlich & Rafi Yanai
  */

class DirHistoryQueue : public QObject  {
  Q_OBJECT
public: 
	DirHistoryQueue(ListPanel* p);
	~DirHistoryQueue();
	KURL::List urlQueue;
//  bool checkPath(const QString& path);
//  void RemovePath(const QString& path);

public slots: // Public slots
  /** No descriptions */
  void slotPathChanged(ListPanel* p);
private:
//	void addUrl(const KURL& url);
	ListPanel* panel;
};

#endif
