/*
    SPDX-FileCopyrightText: 2004 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2010 Jan Lepper <dehtris@yahoo.de>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DIRHISTORYQUEUE_H
#define DIRHISTORYQUEUE_H

// QtCore
#include <QObject>
#include <QStringList>
#include <QUrl>

#include <KConfigGroup>

class KrPanel;

class DirHistoryQueue : public QObject
{
    Q_OBJECT
public:
    explicit DirHistoryQueue(KrPanel *panel);
    ~DirHistoryQueue() override;

    void clear();
    int currentPos()
    {
        return _currentPos;
    }
    qsizetype count()
    {
        return _urlQueue.count();
    }
    QUrl currentUrl();
    void setCurrentUrl(const QUrl &url);
    const QUrl &get(int pos)
    {
        return _urlQueue[pos];
    }
    void add(QUrl url, const QString &currentItem);
    QUrl gotoPos(int pos);
    QUrl goBack();
    QUrl goForward();
    bool canGoBack()
    {
        return _currentPos < count() - 1;
    }
    bool canGoForward()
    {
        return _currentPos > 0;
    }
    QString currentItem(); // current item of the view

    void save(KConfigGroup cfg);
    bool restore(const KConfigGroup &cfg);

public slots:
    void saveCurrentItem();

private:
    KrPanel *_panel;
    int _currentPos;
    QList<QUrl> _urlQueue;
    QStringList _currentItems;
};

#endif
