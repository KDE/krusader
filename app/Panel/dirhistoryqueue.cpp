/*
    SPDX-FileCopyrightText: 2004 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2010 Jan Lepper <dehtris@yahoo.de>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dirhistoryqueue.h"

#include "../defaults.h"
#include "../krservices.h"
#include "PanelView/krview.h"
#include "krpanel.h"

// QtCore
#include <QDir>

DirHistoryQueue::DirHistoryQueue(KrPanel *panel)
    : _panel(panel)
    , _currentPos(0)
{
}

DirHistoryQueue::~DirHistoryQueue() = default;

void DirHistoryQueue::clear()
{
    _urlQueue.clear();
    _currentItems.clear();
    _currentPos = 0;
}

QUrl DirHistoryQueue::currentUrl()
{
    if (_urlQueue.count())
        return _urlQueue[_currentPos];
    else
        return QUrl();
}

void DirHistoryQueue::setCurrentUrl(const QUrl &url)
{
    if (_urlQueue.count())
        _urlQueue[_currentPos] = url;
}

QString DirHistoryQueue::currentItem()
{
    if (count())
        return _currentItems[_currentPos];
    else
        return QString();
}

void DirHistoryQueue::saveCurrentItem()
{
    // if the filesystem-url hasn't been refreshed yet,
    // avoid saving current item for the wrong url
    if (count() && _panel->virtualPath().matches(_urlQueue[_currentPos], QUrl::StripTrailingSlash))
        _currentItems[_currentPos] = _panel->view->getCurrentItem();
}

void DirHistoryQueue::add(QUrl url, const QString &currentItem)
{
    url.setPath(QDir::cleanPath(url.path()));

    if (_urlQueue.isEmpty()) {
        _urlQueue.push_front(url);
        _currentItems.push_front(currentItem);
        return;
    }

    if (_urlQueue[_currentPos].matches(url, QUrl::StripTrailingSlash)) {
        _currentItems[_currentPos] = currentItem;
        return;
    }

    for (int i = 0; i < _currentPos; i++) {
        _urlQueue.pop_front();
        _currentItems.pop_front();
    }

    _currentPos = 0;

    // do we have room for another ?
    if (_urlQueue.count() > 12) { // FIXME: use user-defined size
        // no room - remove the oldest entry
        _urlQueue.pop_back();
        _currentItems.pop_back();
    }

    saveCurrentItem();
    _urlQueue.push_front(url);
    _currentItems.push_front(currentItem);
}

QUrl DirHistoryQueue::gotoPos(int pos)
{
    if (pos >= 0 && pos < _urlQueue.count()) {
        saveCurrentItem();
        _currentPos = pos;
        return currentUrl();
    }
    return QUrl();
}

QUrl DirHistoryQueue::goBack()
{
    return gotoPos(_currentPos + 1);
}

QUrl DirHistoryQueue::goForward()
{
    return gotoPos(_currentPos - 1);
}

void DirHistoryQueue::save(KConfigGroup cfg)
{
    saveCurrentItem();

    QList<QUrl> urls;
    for (const QUrl &url : std::as_const(_urlQueue)) {
        // make sure no passwords are permanently stored
        QUrl safeUrl(url);
        safeUrl.setPassword(QString());
        urls << safeUrl;
    }

    cfg.writeEntry("Entrys", KrServices::toStringList(urls)); // krazy:exclude=spelling
    cfg.writeEntry("CurrentItems", _currentItems);
    cfg.writeEntry("CurrentIndex", _currentPos);
}

bool DirHistoryQueue::restore(const KConfigGroup &cfg)
{
    clear();
    _urlQueue = KrServices::toUrlList(cfg.readEntry("Entrys", QStringList())); // krazy:exclude=spelling
    _currentItems = cfg.readEntry("CurrentItems", QStringList());
    if (!_urlQueue.count() || _urlQueue.count() != _currentItems.count()) {
        clear();
        return false;
    }
    _currentPos = cfg.readEntry("CurrentIndex", 0);
    if (_currentPos >= _urlQueue.count() || _currentPos < 0)
        _currentPos = 0;

    return true;
}
