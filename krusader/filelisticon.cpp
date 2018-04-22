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

#include "filelisticon.h"

#include "krglobal.h"
#include "defaults.h"

#include <KConfigCore/KSharedConfig>


QSize FileListIcon::size() const
{
    int linearSize = KConfigGroup(krConfig, "Look&Feel").readEntry("Filelist Icon Size", _FilelistIconSize).toInt();
    return QSize(linearSize, linearSize);
}

QPixmap FileListIcon::pixmap() const
{
    return QIcon::pixmap(size());
}
