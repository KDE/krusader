/*
    SPDX-FileCopyrightText: 2018-2022 Nikita Melnichenko <nikita+kde@melnichenko.name>
    SPDX-FileCopyrightText: 2018-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filelisticon.h"

#include "defaults.h"
#include "krglobal.h"

#include <KSharedConfig>

QSize FileListIcon::size() const
{
    int linearSize = KConfigGroup(krConfig, "Look&Feel").readEntry("Filelist Icon Size", _FilelistIconSize).toInt();
    return QSize(linearSize, linearSize);
}

QPixmap FileListIcon::pixmap() const
{
    return QIcon::pixmap(size());
}
