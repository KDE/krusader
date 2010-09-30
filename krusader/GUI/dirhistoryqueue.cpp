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

#include "dirhistoryqueue.h"

#include <kdebug.h>

DirHistoryQueue::DirHistoryQueue(QObject *parent) : QObject(parent), _currentPos(0)
{
}

DirHistoryQueue::~DirHistoryQueue() {}

const KUrl& DirHistoryQueue::current()
{
    if(_urlQueue.count())
        return _urlQueue[_currentPos];
    else
        return KUrl();
}

void DirHistoryQueue::add(KUrl url)
{
    url.cleanPath();

    if(_urlQueue.isEmpty()) {
        _urlQueue.push_front(url);
        return;
    }

    if(_urlQueue[_currentPos].equals(url))
        return;

    for (int i = 0; i < _currentPos; i++)
        _urlQueue.pop_front();

    _currentPos = 0;

    // do we have room for another ?
    if (_urlQueue.count() > 12) { // FIXME: use user-defined size
        // no room - remove the oldest entry
        _urlQueue.pop_back();
    }

    _urlQueue.push_front(url);
}

bool DirHistoryQueue::gotoPos(int pos)
{
    if(pos >= 0 && pos < _urlQueue.count()) {
         _currentPos = pos;
         return true;
    }
    return false;
}

bool DirHistoryQueue::goBack()
{
    return gotoPos(_currentPos + 1);
}

bool DirHistoryQueue::goForward()
{
    return gotoPos(_currentPos - 1);
}

#if 0
/** No descriptions */
void DirHistoryQueue::slotPathChanged(ListPanel* p)
{
    KUrl url = p->virtualPath();
    // already in the queue ?
    if (_urlQueue.indexOf(url) >= 0) {
        // remove it !
        _urlQueue.removeAll(url);
    }
    // do we have room for another ?
    if (_urlQueue.size() > 12) {
        // no room - remove the oldest entry
        _urlQueue.pop_back();
    }

    _urlQueue.push_front(url);
}

void DirHistoryQueue::addUrl(const KUrl& url)
{
    if (pathQueue.indexOf(path) == -1) {
        if (pathQueue.size() > 12) {
            // remove the oldest entry
            pathQueue.pop_back();
        }
    } else {
        pathQueue.remove(path);
    }

    pathQueue.push_front(path);
}

void DirHistoryQueue::RemovePath(const QString& path)
{
    QStringList::iterator it;
    it = pathQueue.find(path);
    if (it != pathQueue.end()) {
        pathQueue.remove(it);
    }
}

bool DirHistoryQueue::checkPath(const QString& path)
{
    KUrl url(path);

    QString p = url.path();
    //  kDebug() << "url:" << p <<  ", file: " << url.fileName() << ", dir: " << url.directory() <<  endl;
    if (url.protocol() == "file") {
        QDir dir(path);
        if (!dir.exists()) {
            RemovePath(path);
            return false;
        }
    }

    return true;

}
#endif

#include "dirhistoryqueue.moc"
