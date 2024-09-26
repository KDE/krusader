/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PACKGUI_H
#define PACKGUI_H

#include "packguibase.h"

class PackGUI : public PackGUIBase
{
    Q_OBJECT
public:
    PackGUI(const QString &defaultName, const QString &defaultPath, qsizetype noOfFiles, const QString &filename = "");

public slots:
    void browse() override;

protected slots:
    void accept() override;
    void reject() override;

public:
    static QString filename, destination, type;
    static QMap<QString, QString> extraProps;
};

#endif
