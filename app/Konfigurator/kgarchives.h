/*
    SPDX-FileCopyrightText: Krusader Krew <https://krusader.org>
    SPDX-FileCopyrightText: Csaba Karai <krusader@users.sourceforge.net>

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
