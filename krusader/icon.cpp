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
#include <QPalette>

#include <KConfigCore/KSharedConfig>
#include <KIconThemes/KIconLoader>


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
    // i.e. each of the themes provide a complete set of icons used in the interface
    const QString breeze(Icon::isLightWindowThemeActive() ? "breeze" : "breeze-dark");
    themes << breeze << "oxygen";

    return themes;
}


class IconEngine : public QIconEngine
{
public:
    IconEngine(QString iconName, QIcon fallbackIcon, QStringList overlays = QStringList()) :
        _iconName(iconName), _fallbackIcon(fallbackIcon), _overlays(overlays)
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
    QStringList _overlays;
};

Icon::Icon() : QIcon()
{
}

Icon::Icon(QString name, QStringList overlays) :
    QIcon(new IconEngine(name, QIcon(missingIconPath), overlays))
{
}

Icon::Icon(QString name, QIcon fallbackIcon, QStringList overlays) :
    QIcon(new IconEngine(name, fallbackIcon, overlays))
{
}

struct IconSearchResult
{
    QIcon icon; ///< icon returned by search; null icon if not found
    QString originalThemeName; ///< original theme name if theme is modified by search

    IconSearchResult(QIcon icon, QString originalThemeName) :
        icon(icon), originalThemeName(originalThemeName) {}
};

// Search icon in the configured themes.
// If this call modifies active theme, the original theme name will be specified in the result.
static inline IconSearchResult searchIcon(QString iconName, QStringList themeFallbackList)
{
    if (QDir::isAbsolutePath(iconName)) {
        // a path is used - directly load the icon
        return IconSearchResult(QIcon(iconName), QString());
    } else if (QIcon::hasThemeIcon(iconName)) {
        // current theme has the icon - load seamlessly
        return IconSearchResult(QIcon::fromTheme(iconName), QString());
    } else if (KIconLoader::global()->hasIcon(iconName)) {
        // KF icon loader does a wider search and helps with mime icons
        return IconSearchResult(KDE::icon(iconName), QString());
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

        // not found
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

void Icon::applyOverlays(QPixmap *pixmap, QStringList overlays)
{
    auto iconLoader = KIconLoader::global();

    // Since KIconLoader loadIcon is not virtual method, we can't redefine loadIcon
    // that is called by drawOverlays. The best we can do is to go over the overlays
    // and ensure they exist from the icon loader point of view.
    // If not, we replace the overlay with "emblem-unreadable" which should be available
    // per freedesktop icon name specification:
    // https://specifications.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html
    QStringList fixedOverlays;
    for (auto overlay : overlays) {
        if (overlay.isEmpty() || iconLoader->hasIcon(overlay)) {
            fixedOverlays << overlay;
        } else {
            fixedOverlays << "emblem-unreadable";
        }
    }

    iconLoader->drawOverlays(fixedOverlays, *pixmap, KIconLoader::Desktop);
}

bool Icon::isLightWindowThemeActive()
{
    const QColor textColor = QPalette().brush(QPalette::Text).color();
    return (textColor.red() + textColor.green() + textColor.blue()) / 3 < 128;
}


class IconCacheKey
{
public:
    IconCacheKey(const QString &name, QStringList overlays,
                 const QSize &size, QIcon::Mode mode, QIcon::State state) :
        name(name), overlays(overlays), size(size), mode(mode), state(state)
    {

        auto repr = QString("%1 [%2] %3x%4 %5 %6").arg(name).arg(overlays.join(';'))
                                                  .arg(size.width()).arg(size.height())
                                                  .arg((int)mode).arg((int)state);
        _hash = qHash(repr);
    }

    bool operator ==(const IconCacheKey &x) const
    {
        return name == x.name && overlays == x.overlays
            && size == x.size && mode == x.mode && state == x.state;
    }

    uint hash() const
    {
        return _hash;
    }

    QString name;
    QStringList overlays;
    QSize size;
    QIcon::Mode mode;
    QIcon::State state;

private:
    uint _hash;
};

uint qHash(const IconCacheKey &key)
{
    return key.hash();
}

QPixmap IconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    static QCache<IconCacheKey, QPixmap> cache(cacheSize);
    static QString cachedTheme;

    QString systemTheme = QIcon::themeName();

    // [WORKAROUND] If system theme is Breeze, pick light or dark variant of the theme explicitly
    // This type of selection works implicitly when QIcon::fromTheme is used,
    // however after QIcon::setThemeName it stops working for unknown reason.
    if (systemTheme == "breeze" || systemTheme == "breeze-dark") {
        const QString pickedSystemTheme(Icon::isLightWindowThemeActive() ? "breeze" : "breeze-dark");
        if (systemTheme != pickedSystemTheme) {
            qDebug() << "System icon theme variant changed:" << systemTheme << "->" << pickedSystemTheme;
            systemTheme = pickedSystemTheme;
            QIcon::setThemeName(systemTheme);
        }
    }

    // invalidate cache if system theme is changed
    if (cachedTheme != systemTheme) {
        if (!cachedTheme.isEmpty()) {
            qDebug() << "System icon theme changed:" << cachedTheme << "->" << systemTheme;
        }

        cache.clear();
        cachedTheme = systemTheme;
    }

    // an empty icon name is a special case - we don't apply any fallback
    if (_iconName.isEmpty()) {
        return QPixmap();
    }

    auto key = IconCacheKey(_iconName, _overlays, size, mode, state);

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
        qWarning() << "Unable to find icon" << _iconName << "of size" << size << "in any configured theme";
        *pixmap = _fallbackIcon.pixmap(size, mode, state);
    }

    // apply overlays in a safe manner
    Icon::applyOverlays(pixmap, _overlays);

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
