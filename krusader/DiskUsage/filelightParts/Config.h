/*****************************************************************************
 * Copyright (C) 2004 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2004-2020 Krusader Krew [https://krusader.org]              *
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

#ifndef CONFIG_H
#define CONFIG_H

// QtCore
#include <QStringList>

class KConfigGroup;

namespace Filelight
{
enum MapScheme { Rainbow, HighContrast, KDE, FileDensity, ModTime };

class Config
{
    static KConfigGroup kconfig();

public:
    static void read();
    static void write();

    //keep everything positive, avoid using DON'T, NOT or NO

    static bool varyLabelFontSizes;
    static bool showSmallFiles;
    static uint contrast;
    static uint antiAliasFactor;
    static uint minFontPitch;
    static uint defaultRingDepth;

    static MapScheme scheme;
};
}

using Filelight::Config;

#endif
