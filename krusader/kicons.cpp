/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2000 Rafi Yanai <yanai@users.sourceforge.net>               *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
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

#include "kicons.h"
#include "krglobal.h"
#include "defaults.h"

// QtCore
#include <QString>
// QtGui
#include <QPixmap>
// QtWidgets
#include <QStyle>

#include <KConfigCore/KSharedConfig>
#include <KIconThemes/KIconLoader>

QPixmap FL_LOADICON(QString name)
{
    KConfigGroup group(krConfig, "Look&Feel");
    int size = (group.readEntry("Filelist Icon Size", _FilelistIconSize)).toInt();
    return krLoader->loadIcon(name, KIconLoader::Desktop, size);
}
