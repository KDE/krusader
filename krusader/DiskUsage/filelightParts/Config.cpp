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

#include "Config.h"
#include <kconfig.h>
#include <kglobal.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

bool Config::varyLabelFontSizes = true;
bool Config::showSmallFiles = false;
uint Config::contrast = 50;
uint Config::antiAliasFactor = 2;
uint Config::minFontPitch = 10;
uint Config::defaultRingDepth = 4;
Filelight::MapScheme Config::scheme;

inline KConfigGroup
Filelight::Config::kconfig()
{
    KSharedConfigPtr config = KGlobal::config();
    return KConfigGroup( config, "DiskUsage" );
}

void
Filelight::Config::read()
{
    KConfigGroup group = kconfig();

    varyLabelFontSizes = group.readEntry( "varyLabelFontSizes", true );
    showSmallFiles     = group.readEntry( "showSmallFiles", false );
    contrast           = group.readEntry( "contrast", 50 );
    antiAliasFactor    = group.readEntry( "antiAliasFactor", 2 );
    minFontPitch       = group.readEntry( "minFontPitch", QFont().pointSize() - 3);
    scheme = (MapScheme) group.readEntry( "scheme", 0 );

    defaultRingDepth   = 4;
}

void
Filelight::Config::write()
{
    KConfigGroup group = kconfig();

    group.writeEntry( "varyLabelFontSizes", varyLabelFontSizes );
    group.writeEntry( "showSmallFiles", showSmallFiles);
    group.writeEntry( "contrast", contrast );
    group.writeEntry( "antiAliasFactor", antiAliasFactor );
    group.writeEntry( "minFontPitch", minFontPitch );
    group.writeEntry( "scheme", (int)scheme );
}
