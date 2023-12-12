/*
    SPDX-FileCopyrightText: 2004 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

    // keep everything positive, avoid using DON'T, NOT or NO

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
