/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGADVANCED_H
#define KGADVANCED_H

#include "konfiguratorpage.h"

class KgAdvanced : public KonfiguratorPage
{
    Q_OBJECT

public:
    explicit KgAdvanced(bool first, QWidget *parent = nullptr);
};

#endif /* __KGADVANCED_H__ */
