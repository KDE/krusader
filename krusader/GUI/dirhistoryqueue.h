/*****************************************************************************
 * Copyright (C) 2004 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2004 Rafi Yanai <yanai@users.sourceforge.net>               *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#ifndef DIRHISTORYQUEUE_H
#define DIRHISTORYQUEUE_H

#include <QtCore/QObject>
#include <kurl.h>

class DirHistoryQueue : public QObject
{
    Q_OBJECT
public:
    DirHistoryQueue(QObject *parent);
    ~DirHistoryQueue();

    int currentPos() {
        return _currentPos;
    }
    int count() {
        return _urlQueue.count();
    }
    const KUrl& current();
    const KUrl& get(int pos) {
        return _urlQueue[pos];
    }
    void add(KUrl url);
    bool gotoPos(int pos);
    bool goBack();
    bool goForward();
    bool canGoBack() {
        return _currentPos < count() - 1;
    }
    bool canGoForward() {
        return _currentPos > 0;
    }

public slots: // Public slots
    /** No descriptions */
//     void slotPathChanged(ListPanel* p);
private:
// void addUrl(const KUrl& url);
//     ListPanel* panel;
    int _currentPos;
    KUrl::List _urlQueue;
};

#endif
