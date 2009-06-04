/*****************************************************************************
 * Copyright (C) 2004 Shie Erlich <erlich@users.sourceforge.net>             *
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

#ifndef CONFIG_H
#define CONFIG_H

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
