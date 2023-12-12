/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRBOOKMARK_H
#define KRBOOKMARK_H

// QtCore
#include <QList>
#include <QUrl>
// QtWidgets
#include <QAction>

class KActionCollection;
class ListPanelActions;

class KrBookmark : public QAction
{
    Q_OBJECT
public:
    KrBookmark(const QString &name, QUrl url, KActionCollection *parent, const QString &iconName = "", const QString &actionName = QString());
    explicit KrBookmark(const QString &name, const QString &iconName = ""); // creates a folder
    ~KrBookmark() override;

    // text() and setText() to change the name of the bookmark
    // icon() and setIcon() to change icons

    void setIconName(const QString &iconName);

    inline const QString &iconName() const
    {
        return _iconName;
    }

    inline const QUrl &url() const
    {
        return _url;
    }
    inline void setURL(const QUrl &url)
    {
        _url = url;
    }
    inline bool isFolder() const
    {
        return _folder;
    }
    inline bool isSeparator() const
    {
        return _separator;
    }
    QList<KrBookmark *> &children()
    {
        return _children;
    }

    static KrBookmark *getExistingBookmark(const QString &actionName, KActionCollection *collection);

    // ----- special bookmarks
    static KrBookmark *trash(KActionCollection *collection);
    static KrBookmark *virt(KActionCollection *collection);
    static KrBookmark *lan(KActionCollection *collection);
    static QAction *jumpBackAction(KActionCollection *collection, bool isSetter = false, ListPanelActions *sourceActions = nullptr);
    static KrBookmark *separator();

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
