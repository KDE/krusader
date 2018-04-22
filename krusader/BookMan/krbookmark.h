/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#ifndef KRBOOKMARK_H
#define KRBOOKMARK_H

// QtCore
#include <QList>
#include <QUrl>
// QtWidgets
#include <QAction>

class KActionCollection;
class ListPanelActions;


class KrBookmark: public QAction
{
    Q_OBJECT
public:
    KrBookmark(QString name, QUrl url, KActionCollection *parent, QString icon = "", QString actionName = QString());
    explicit KrBookmark(QString name, QString iconName = ""); // creates a folder
    ~KrBookmark();
    // text() and setText() to change the name of the bookmark
    // icon() and setIcon() to change icons
    inline const QString& iconName() const {
        return _iconName;
    }
    inline const QUrl &url() const {
        return _url;
    }
    inline void setURL(const QUrl &url) {
        _url = url;
    }
    inline bool isFolder() const {
        return _folder;
    }
    inline bool isSeparator() const {
        return _separator;
    }
    QList<KrBookmark *>& children() {
        return _children;
    }

    static KrBookmark * getExistingBookmark(QString actionName, KActionCollection *collection);

    // ----- special bookmarks
    static KrBookmark * trash(KActionCollection *collection);
    static KrBookmark * virt(KActionCollection *collection);
    static KrBookmark * lan(KActionCollection *collection);
    static QAction * jumpBackAction(KActionCollection *collection, bool isSetter = false, ListPanelActions *sourceActions = 0);
    static KrBookmark * separator();

signals:
    void activated(const QUrl &url);

protected slots:
    void activatedProxy();


private:
    QUrl _url;
    QString _iconName;
    bool _folder;
    bool _separator;
    bool _autoDelete;
    QList<KrBookmark *> _children;
};

#endif // KRBOOKMARK_H
