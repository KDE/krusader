/*****************************************************************************
 * Copyright (C) 2017 Krusader Krew                                          *
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

#include "krviewproperties.h"

KrViewProperties::KrViewProperties(bool displayIcons, bool numericPermissions,
                                   KrViewProperties::SortOptions sortOptions,
                                   KrViewProperties::SortMethod sortMethod, bool humanReadableSize,
                                   bool localeAwareCompareIsCaseSensitive,
                                   QStringList atomicExtensions)
    : numericPermissions(numericPermissions), displayIcons(displayIcons), sortColumn(Name),
      sortOptions(sortOptions), sortMethod(sortMethod), filter(KrViewProperties::All),
      filterMask(KRQuery("*")), filterApplysToDirs(false),
      localeAwareCompareIsCaseSensitive(localeAwareCompareIsCaseSensitive),
      humanReadableSize(humanReadableSize), atomicExtensions(atomicExtensions), numberOfColumns(1)
{
}
