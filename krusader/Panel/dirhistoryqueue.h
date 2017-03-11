/*****************************************************************************
 * Copyright (C) 2004 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2004 Rafi Yanai <yanai@users.sourceforge.net>               *
 * Copyright (C) 2010 Jan Lepper <dehtris@yahoo.de>                          *
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

// QtCore
#include <QObject>
#include <QStringList>
#include <QUrl>

# include <KConfigCore/KConfigGroup>

class KrPanel;

class DirHistoryQueue : public QObject
{
    Q_OBJECT
public:
    explicit DirHistoryQueue(KrPanel *panel);
    ~DirHistoryQueue();

    void clear();
    int currentPos() {
        return _currentPos;
    }
    int count() {
        return _urlQueue.count();
    }
    QUrl currentUrl();
    void setCurrentUrl(const QUrl &url);
    const QUrl &get(int pos) {
        return _urlQueue[pos];
    }
    void add(QUrl url, QString currentItem);
    bool gotoPos(int pos);
    bool goBack();
    bool goForward();
    bool canGoBack() {
        return _currentPos < count() - 1;
    }
    bool canGoForward() {
        return _currentPos > 0;
    }
    QString currentItem(); // current item of the view

    void save(KConfigGroup cfg);
    bool restore(KConfigGroup cfg);

public slots:
    void saveCurrentItem();

private:
    KrPanel* _panel;
    int _currentPos;
    QList<QUrl> _urlQueue;
    QStringList _currentItems;
};

#endif
