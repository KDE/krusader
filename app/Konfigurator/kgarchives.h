/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGARCHIVES_H
#define KGARCHIVES_H

#include "konfiguratorpage.h"

class KgArchives : public KonfiguratorPage
{
    Q_OBJECT

public:
    explicit KgArchives(bool first, QWidget *parent = nullptr);

public slots:
    void slotAutoConfigure();

protected:
    void disableNonExistingPackers();
};

#endif /* __KGARCHIVES_H__ */
