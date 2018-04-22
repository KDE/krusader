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

#ifndef ICON_H
#define ICON_H

// QtGui
#include <QIcon>
#include <QIconEngine>

/**
 * @class Icon
 *
 * Universal icon class for Krusader.
 *
 * There is a list of configured themes that Icon searches in.
 * The order of themes is the following: active theme, theme specified by user,
 * fallback themes that provide complete icon set for Krusader.
 * If icon is not found in any configured theme, the fallback icon is used.
 */
class Icon : public QIcon
{
public:
    Icon();
    explicit Icon(QString name, QStringList overlays = QStringList());
    explicit Icon(QString name, QIcon fallbackIcon, QStringList overlays = QStringList());

    /// Check if icon exists in any configured theme
    static bool exists(QString iconName);

    /// Apply overlays to the pixmap with fallbacks to standard emblems
    static void applyOverlays(QPixmap *pixmap, QStringList overlays);

    /// Determine if light window theme is currently enabled
    static bool isLightWindowThemeActive();
};

#endif // ICON_H
