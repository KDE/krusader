/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
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

#ifndef KRBOOKMARK_H
#define KRBOOKMARK_H

#include <kaction.h>
#include <QtCore/QList>
#include <kurl.h>

class KActionCollection;

class KrBookmark: public KAction
{
    Q_OBJECT
public:
    KrBookmark(QString name, KUrl url, KActionCollection *parent, QString icon = "", QString actionName = QString());
    KrBookmark(QString name, QString icon = ""); // creates a folder
    ~KrBookmark();
    // text() and setText() to change the name of the bookmark
    // icon() and setIcon() to change icons
    inline const QString& iconName() const {
        return _icon;
    }
    inline const KUrl& url() const {
        return _url;
    }
    inline void setURL(const KUrl& url) {
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

    static KrBookmark* getExistingBookmark(QString actionName, KActionCollection *collection);
    // ----- special bookmarks
    static KrBookmark* trash(KActionCollection *collection);
    static KrBookmark* virt(KActionCollection *collection);
    static KrBookmark* lan(KActionCollection *collection);
    static KrBookmark* separator();

signals:
    void activated(const KUrl& url);

protected slots:
    void activatedProxy();


private:
    KUrl _url;
    QString _icon;
    bool _folder;
    bool _separator;
    bool _autoDelete;
    QList<KrBookmark *> _children;
};

#endif // KRBOOKMARK_H
