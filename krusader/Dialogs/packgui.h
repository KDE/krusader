/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2000 Rafi Yanai <krusader@users.sourceforge.net>            *
 * Copyright (C) 2004-2019 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#ifndef PACKGUI_H
#define PACKGUI_H

#include "packguibase.h"

class PackGUI : public PackGUIBase
{
    Q_OBJECT
public:
    PackGUI(QString defaultName, QString defaultPath, int noOfFiles, QString filename = "");

public slots:
    void browse() Q_DECL_OVERRIDE;

protected slots:
    void accept() Q_DECL_OVERRIDE;
    void reject() Q_DECL_OVERRIDE;

public:
    static QString filename, destination, type;
    static QMap<QString, QString> extraProps;
};

#endif
