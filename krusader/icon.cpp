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

// QtCore
#include <QDir>
#include <QDebug>
// QtGui
#include <QPainter>


class IconEngine : public QIconEngine
{
public:
    IconEngine(QString iconName, QIcon fallbackIcon) : _iconName(iconName), _fallbackIcon(fallbackIcon)
    {
        // TODO: fill from settings
        _themeFallbackList << "breeze" << "oxygen";
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

// TODO: use some neutral icon from resource file as a fallback
Icon::Icon(QString name) : QIcon(new IconEngine(name, QIcon::fromTheme("emblem-unreadable")))
{
}

QPixmap IconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    // TODO: implement icon cache here as setThemeName invalidates Qt icon cache
    // TODO: need to track system icon theme change and reset active theme name to system theme, invalidate Krusader icon cache

    if (QDir::isAbsolutePath(_iconName)) {
        return QIcon(_iconName).pixmap(size, mode, state);
    }

    if (QIcon::hasThemeIcon(_iconName))
        return QIcon::fromTheme(_iconName).pixmap(size, mode, state);

    QPixmap pixmap;
    auto currentTheme = QIcon::themeName();
    for (auto fallbackThemeName : _themeFallbackList) {
        QIcon::setThemeName(fallbackThemeName);
        if (QIcon::hasThemeIcon(_iconName)) {
            pixmap = QIcon::fromTheme(_iconName).pixmap(size, mode, state);
            break;
        }
    }
    QIcon::setThemeName(currentTheme);

    if (pixmap.isNull()) {
        qDebug() << "Unable to find icon" << _iconName << "of size" << size << "in any supported theme";

        pixmap = _fallbackIcon.pixmap(size, mode, state);

        if (pixmap.isNull()) {
            qWarning() << "Fallback icon" << _fallbackIcon << "is unavailable";
        }
    }

    return pixmap;
}

void IconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    QSize pixmapSize = rect.size();
    QPixmap px = pixmap(pixmapSize, mode, state);
    painter->drawPixmap(rect, px);
}
