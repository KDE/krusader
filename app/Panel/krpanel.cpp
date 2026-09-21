/*
    SPDX-FileCopyrightText: Krusader Krew <https://krusader.org>
    SPDX-FileCopyrightText: Jan Lepper <krusader@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "../FileSystem/sizecalculator.h"
#include "krpanel.h"

#include "../abstractpanelmanager.h"
#include "panelfunc.h"

QUrl KrPanel::virtualPath() const
{
    return func->virtualDirectory();
}

KrPanel *KrPanel::otherPanel() const
{
    return _manager->otherManager()->currentPanel();
}

bool KrPanel::isLeft() const
{
    return _manager->isLeft();
}
