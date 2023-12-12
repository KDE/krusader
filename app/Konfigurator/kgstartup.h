/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGSTARTUP_H
#define KGSTARTUP_H

#include "konfiguratorpage.h"

class KgStartup : public KonfiguratorPage
{
    Q_OBJECT

public:
    explicit KgStartup(bool first, QWidget *parent = nullptr);

public slots:
    void slotDisable();

protected:
    KonfiguratorRadioButtons *saveRadio;
    KonfiguratorCheckBoxGroup *uiCbGroup;
    KonfiguratorComboBox *profileCombo;
};

#endif /* __KGSTARTUP_H__ */
