/*
    SPDX-FileCopyrightText: 2018-2022 Nikita Melnichenko <nikita+kde@melnichenko.name>
    SPDX-FileCopyrightText: 2018-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    static bool exists(const QString &iconName);

    /// Apply overlays to the pixmap with fallbacks to standard emblems
    static void applyOverlays(QPixmap *pixmap, QStringList overlays);

    /// Determine if light window theme is currently enabled
    static bool isLightWindowThemeActive();
};

#endif // ICON_H
