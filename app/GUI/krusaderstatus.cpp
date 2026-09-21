/*
    SPDX-FileCopyrightText: Krusader Krew <https://krusader.org>
    SPDX-FileCopyrightText: Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: Rafi Yanai <krusader@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krusaderstatus.h"

// QtGui
#include <QFontMetrics>

#include <KLocalizedString>

KrusaderStatus::KrusaderStatus(QWidget *parent)
    : QStatusBar(parent)
{
    showMessage(i18n("Ready."), 5000);
    setMaximumHeight(QFontMetrics(font()).height() + 2);
}

KrusaderStatus::~KrusaderStatus() = default;
