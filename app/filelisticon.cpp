/*
    SPDX-FileCopyrightText: Krusader Krew <https://krusader.org>
    SPDX-FileCopyrightText: Nikita Melnichenko <nikita+kde@melnichenko.name>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filelisticon.h"

#include "defaults.h"
#include "krglobal.h"

#include <KSharedConfig>

QSize FileListIcon::size() const
{
    int linearSize = KConfigGroup(krConfig, "Look&Feel").readEntry("Filelist Icon Size", Defaults::filelistIconSize);
    return QSize(linearSize, linearSize);
}

QPixmap FileListIcon::pixmap() const
{
    return QIcon::pixmap(size());
}
