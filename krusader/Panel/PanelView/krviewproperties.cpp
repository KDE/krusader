/*****************************************************************************
 * Copyright (C) 2017-2019 Krusader Krew [https://krusader.org]              *
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

#include "krviewproperties.h"

#include <utility>

KrViewProperties::KrViewProperties(bool displayIcons, bool numericPermissions,
                                   KrViewProperties::SortOptions sortOptions,
                                   KrViewProperties::SortMethod sortMethod, bool humanReadableSize,
                                   bool localeAwareCompareIsCaseSensitive,
                                   QStringList atomicExtensions)
    : numericPermissions(numericPermissions), displayIcons(displayIcons), sortColumn(Name),
      sortOptions(sortOptions), sortMethod(sortMethod), filter(KrViewProperties::All),
      filterMask(KrQuery("*")), filterApplysToDirs(false),
      localeAwareCompareIsCaseSensitive(localeAwareCompareIsCaseSensitive),
      humanReadableSize(humanReadableSize), atomicExtensions(std::move(atomicExtensions)), numberOfColumns(1)
{
}
