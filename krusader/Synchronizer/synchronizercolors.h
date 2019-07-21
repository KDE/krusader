/*****************************************************************************
 * Copyright (C) 2016-2019 Krusader Krew [https://krusader.org]              *
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

#ifndef SYNCHRONIZERCOLORS_H
#define SYNCHRONIZERCOLORS_H

#include <KGuiAddons/KColorUtils>

#define DECLARE_SYNCHRONIZER_BACKGROUND_DEFAULTS  QColor SYNCHRONIZER_BACKGROUND_DEFAULTS[] = { \
    QGuiApplication::palette().color(QPalette::Active, QPalette::Base), \
    QGuiApplication::palette().color(QPalette::Active, QPalette::Base), \
    QGuiApplication::palette().color(QPalette::Active, QPalette::Base), \
    QGuiApplication::palette().color(QPalette::Active, QPalette::Base), \
    KColorUtils::tint(QGuiApplication::palette().color(QPalette::Active, QPalette::Base), Qt::red, 0.5) \
}

#define DECLARE_SYNCHRONIZER_FOREGROUND_DEFAULTS  QColor SYNCHRONIZER_FOREGROUND_DEFAULTS[] = { \
    QGuiApplication::palette().color(QPalette::Active, QPalette::Text), \
    KColorUtils::tint(QGuiApplication::palette().color(QPalette::Active, QPalette::Text), Qt::red, 0.7), \
    KColorUtils::tint(QGuiApplication::palette().color(QPalette::Active, QPalette::Text), QColor(0, 115, 207), 0.7), \
    KColorUtils::tint(QGuiApplication::palette().color(QPalette::Active, QPalette::Text), Qt::green, 0.5), \
    QGuiApplication::palette().color(QPalette::Active, QPalette::Text) \
}

#endif /* __SYNCHRONIZERCOLORS_H__ */
