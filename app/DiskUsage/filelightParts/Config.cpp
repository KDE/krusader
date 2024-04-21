/*
    SPDX-FileCopyrightText: 2004 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Config.h"

// QtGui
#include <QFont>

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>

bool Config::varyLabelFontSizes = true;
bool Config::showSmallFiles = false;
uint Config::contrast = 50;
uint Config::antiAliasFactor = 2;
uint Config::minFontPitch = 10;
uint Config::defaultRingDepth = 4;
Filelight::MapScheme Config::scheme;

inline KConfigGroup Filelight::Config::kconfig()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    return KConfigGroup(config, "DiskUsage");
}

void Filelight::Config::read()
{
    KConfigGroup group = kconfig();

    varyLabelFontSizes = group.readEntry("varyLabelFontSizes", true);
    showSmallFiles = group.readEntry("showSmallFiles", false);
    contrast = group.readEntry("contrast", 50);
    antiAliasFactor = group.readEntry("antiAliasFactor", 2);
    minFontPitch = group.readEntry("minFontPitch", QFont().pointSize() - 3);
    scheme = (MapScheme)group.readEntry("scheme", 0);

    defaultRingDepth = 4;
}

void Filelight::Config::write()
{
    KConfigGroup group = kconfig();

    group.writeEntry("varyLabelFontSizes", varyLabelFontSizes);
    group.writeEntry("showSmallFiles", showSmallFiles);
    group.writeEntry("contrast", contrast);
    group.writeEntry("antiAliasFactor", antiAliasFactor);
    group.writeEntry("minFontPitch", minFontPitch);
    group.writeEntry("scheme", (int)scheme);
}
