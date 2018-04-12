/*****************************************************************************
 * Copyright (C) 2018 Nikita Melnichenko <nikita+kde@melnichenko.name>       *
 * Copyright (C) 2018 Krusader Krew [https://krusader.org]                   *
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

#include "icon.h"

#include "krglobal.h"

// QtCore
#include <QCache>
#include <QPair>
#include <QDir>
#include <QDebug>
// QtGui
#include <QPainter>
#include <QPixmap>

#include <KConfigCore/KSharedConfig>


static const int cacheSize = 500;
static const char *missingIconPath = ":/icons/icon-missing.svgz";


static inline QStringList getThemeFallbackList()
{
    QStringList themes;

    // add user fallback theme if set
    if (krConfig) {
        const KConfigGroup group(krConfig, QStringLiteral("Startup"));
        QString userFallbackTheme = group.readEntry("Fallback Icon Theme", QString());
        if (!userFallbackTheme.isEmpty()) {
            themes << userFallbackTheme;
        }
    }

    // Breeze and Oxygen are weak dependencies of Krusader,
    // i.e. any of the themes supply a complete set of icons used in the interface
    themes << "breeze" << "oxygen";

    return themes;
}


class IconEngine : public QIconEngine
{
public:
    IconEngine(QString iconName, QIcon fallbackIcon) : _iconName(iconName), _fallbackIcon(fallbackIcon)
    {
        _themeFallbackList = getThemeFallbackList();
    }

    virtual void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
    virtual QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;

    virtual IconEngine *clone() const override
    {
        return new IconEngine(*this);
    }

private:
    QString _iconName;
    QStringList _themeFallbackList;
    QIcon _fallbackIcon;
};

Icon::Icon() : QIcon()
{
}

Icon::Icon(QString name) : QIcon(new IconEngine(name, QIcon(missingIconPath)))
{
}

struct IconSearchResult
{
    QIcon icon; ///< icon returned by search; null icon if not found
    QString originalThemeName; ///< original theme name if theme is modified by search

    IconSearchResult(QIcon icon, QString originalThemeName) :
        icon(icon), originalThemeName(originalThemeName) {}
};

// Search icon in specified themes.
// If this call modifies current theme, the original theme name will be the second item in the pair.
static inline IconSearchResult searchIcon(QString iconName, QStringList themeFallbackList)
{
    if (QDir::isAbsolutePath(iconName)) {
        // a path is used - directly load the icon
        return IconSearchResult(QIcon(iconName), QString());
    } else if (QIcon::hasThemeIcon(iconName)) {
        // current theme has the icon - load seamlessly
        return IconSearchResult(QIcon::fromTheme(iconName), QString());
    } else {
        // search the icon in fallback themes
        auto currentTheme = QIcon::themeName();
        for (auto fallbackThemeName : themeFallbackList) {
            QIcon::setThemeName(fallbackThemeName);
            if (QIcon::hasThemeIcon(iconName)) {
                return IconSearchResult(QIcon::fromTheme(iconName), currentTheme);
            }
        }
        QIcon::setThemeName(currentTheme);

        return IconSearchResult(QIcon(), QString());
    }
}

bool Icon::exists(QString iconName)
{
    static QCache<QString, bool> cache(cacheSize);
    static QString cachedTheme;

    // invalidate cache if system theme is changed
    if (cachedTheme != QIcon::themeName()) {
        cache.clear();
        cachedTheme = QIcon::themeName();
    }

    // return cached result when possible
    if (cache.contains(iconName)) {
        return *cache.object(iconName);
    }

    auto searchResult = searchIcon(iconName, getThemeFallbackList());
    if (!searchResult.originalThemeName.isNull()) {
        QIcon::setThemeName(searchResult.originalThemeName);
    }

    bool *result = new bool(!searchResult.icon.isNull());

    // update the cache; the cache takes ownership over the result
    cache.insert(iconName, result);

    return *result;
}

class IconCacheKey
{
public:
    IconCacheKey(const QString &name, const QSize &size, QIcon::Mode mode, QIcon::State state) :
        name(name), size(size), mode(mode), state(state)
    {
    }

    bool operator ==(const IconCacheKey &x) const
    {
        return name == x.name && size == x.size && mode == x.mode && state == x.state;
    }

    uint hash() const
    {
        return qHash(QString("%1 %2x%3 %4 %5").arg(name).arg(size.width()).arg(size.height())
                                              .arg((int)mode).arg((int)state));
    }

    QString name;
    QSize size;
    QIcon::Mode mode;
    QIcon::State state;
};

uint qHash(const IconCacheKey &key)
{
    return key.hash();
}

QPixmap IconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    static QCache<IconCacheKey, QPixmap> cache(cacheSize);
    static QString cachedTheme;

    // invalidate cache if system theme is changed
    if (cachedTheme != QIcon::themeName()) {
        if (!cachedTheme.isEmpty()) {
            qDebug() << "System icon theme changed:" << cachedTheme << "->" << QIcon::themeName();
        }

        cache.clear();
        cachedTheme = QIcon::themeName();
    }

    auto key = IconCacheKey(_iconName, size, mode, state);

    // return cached icon when possible
    if (cache.contains(key)) {
        return *cache.object(key);
    }

    // search icon and extract pixmap
    auto pixmap = new QPixmap;
    auto searchResult = searchIcon(_iconName, _themeFallbackList);
    if (!searchResult.icon.isNull()) {
        *pixmap = searchResult.icon.pixmap(size, mode, state);
    }
    if (!searchResult.originalThemeName.isNull()) {
        QIcon::setThemeName(searchResult.originalThemeName);
    }

    // can't find the icon neither in system theme nor in fallback themes - load fallback icon
    if (pixmap->isNull()) {
        qWarning() << "Unable to find icon" << _iconName << "of size" << size << "in any specified theme";
        *pixmap = _fallbackIcon.pixmap(size, mode, state);
    }

    // update the cache; the cache takes ownership over the pixmap
    cache.insert(key, pixmap);

    return *pixmap;
}

void IconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    QSize pixmapSize = rect.size();
    QPixmap px = pixmap(pixmapSize, mode, state);
    painter->drawPixmap(rect, px);
}
