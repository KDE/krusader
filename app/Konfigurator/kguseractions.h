/*
    SPDX-FileCopyrightText: Krusader Krew <https://krusader.org>
    SPDX-FileCopyrightText: Jonas Bähr <krusader@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGUSERACTIONS_H
#define KGUSERACTIONS_H

#include "konfiguratorpage.h"

class KgUserActions : public KonfiguratorPage
{
    Q_OBJECT

public:
    explicit KgUserActions(bool first, QWidget *parent = nullptr);

public slots:
    void startActionMan();
};

#endif /* __KGUSERACTIONS_H__ */
